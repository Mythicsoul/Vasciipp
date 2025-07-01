#include <getopt.h>

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <unistd.h>

#include "ascii.hpp"
#include "utils.hpp"

bool showVerbose = false;
bool fillTerminal = false;
bool loopFile = false;
bool invertAscii = false;
bool customAscii = false;

std::queue<cv::Mat> frameQueue;
std::mutex queueMutex;
std::condition_variable frameReadyCV;

std::atomic<int> frameCount(0);
std::atomic<bool> running(true);

TerminalSettings *ts = nullptr;

void parseFlags(int argc, char **argv) {
  const char *const short_opts = "hvfliF:B:A:c";
  struct option long_opts[] = {
      {"help", no_argument, nullptr, 'h'},           {"verbose", no_argument, nullptr, 'v'},
      {"fill", no_argument, nullptr, 'f'},           {"loop", no_argument, nullptr, 'l'},
      {"invert", no_argument, nullptr, 'i'},         {"fg-color", required_argument, nullptr, 'F'},
      {"bg-color", required_argument, nullptr, 'B'}, {"ascii", required_argument, nullptr, 'A'},
      {"colors", no_argument, nullptr, 'c'},         {nullptr, 0, nullptr, 0}};

  int opt, optIndex = 0;
  while ((opt = getopt_long(argc, argv, short_opts, long_opts, &optIndex)) != -1) {
    switch (opt) {
    case 'h': // totally intended feature: scream for help -hhhhhhhhhhhhh
      std::cout << "Usage: " << argv[0] << " <file> <option(s)>\n"
                << "Quit by pressing 'q'\n\n"
                << "Options:\n"
                << "  -h, --help      displays this message\n"
                << "  -v, --verbose   displays additional information\n"
                << "  -f, --fill      ignores aspect ratio and fills all terminal cells\n"
                << "  -l, --loop      continously loops until interrupted\n"
                << "  -c, --colors    prints color info\n"
                << "  -F, --fg-color  sets foreground color\n"
                << "  -B, --bg-color  sets background color\n"
                << "  -A, --ascii     sets custom ascii character set (default: \" .:-=+*#%@'\")\n"
                << "  -i, --invert    inverts the ascii character set and adds\n"
                << "                  some empty spaces for visual clarity\n"
                << std::endl;
      exit(0);
    case 'v':
      showVerbose = true;
      break;
    case 'f':
      fillTerminal = true;
      break;
    case 'l':
      loopFile = true;
      break;
    case 'i':
      invertAscii = true;
      break;
    case 'F':
      setForegroundColor(optarg);
      break;
    case 'B':
      setBackgroundColor(optarg);
      break;
    case 'A':
      customAscii = true;
      customAsciiCharSet = optarg;
      break;
    case 'c':
      std::cout << "Available colors:\n\n"
                << "black, red, green, yellow,\n"
                << "blue, magenta, cyan, white\n"
                << "\nBrighter variants are prefixed with \"bright_\"\n"
                << "\nUsage examples: \n"
                << argv[0] << " <file> -F bright_cyan --bg-color=red\n"
                << argv[0] << " <file> --fg-color \"yellow\" -B black\n"
                << std::endl;
      exit(0);
    default:
      exit(1);
    }
  }
}

void captureFrames(cv::VideoCapture &capture, verboseInfo &stats, timingInfo &timingInfo,
                   std::atomic<bool> &running, double &frameDelay) {
  cv::Mat frame;
  using duration_type = std::chrono::duration<double, std::milli>;
  auto expectedFrameTime = std::chrono::steady_clock::now();

  while (running) {

    { // lock
      std::unique_lock<std::mutex> queueLock(queueMutex);
      frameReadyCV.wait(queueLock, [&]() { return frameQueue.size() < 1 || !running; });
      if (!running)
        break;

      if (frameQueue.size() < 1) {
        auto startCapture = std::chrono::steady_clock::now();
        capture >> frame;
        auto endCapture = std::chrono::steady_clock::now();
        auto captureTime =
            std::chrono::duration<double, std::milli>(endCapture - startCapture).count();
        timingInfo.captureTime = captureTime;

        if (frame.empty()) {
          if (loopFile) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            stats.totalFrameCount = 0;
            continue;
          }
          running = false;
          frameReadyCV.notify_all();
          break;
        }

        // add frame to q & notify main thread
        frameQueue.push(frame);
        frameReadyCV.notify_one();
      }
    } // unlock

    auto now = std::chrono::steady_clock::now();
    auto processingTime =
        std::chrono::duration<double, std::milli>(now - expectedFrameTime).count();

    if (processingTime < frameDelay) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(static_cast<int>(frameDelay - processingTime)));
    }

    expectedFrameTime +=
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration_type(frameDelay));
  }
}

int main(int argc, char **argv) {
  // for(int i = 0; i < argc; ++i) {
  //     printf("Argument %d: %s\n", i, argv[i]);
  // }
  // exit(0);
  std::signal(SIGINT, signalHandler);

  parseFlags(argc, argv);

  if (optind >= argc) {
    std::cerr << "\nUsage: " << argv[0] << " <file> <option(s)>\n"
              << "You must provide a file.\n"
              << "Type " << argv[0] << " --help to see a list of all options." << std::endl;
    return 1;
  }

  std::string file = argv[optind];
  if (!std::filesystem::exists(file)) {
    std::cerr << "\nError: Invalid file '" << file << "'\n" << std::endl;
    return 1;
  }

  cv::VideoCapture capture(file);
  if (!capture.isOpened()) {
    std::cerr << "Error: Cannot open '" << file << "'" << std::endl;
    return 1;
  }

  ts = new TerminalSettings;
  char ch;
  setupAscii(invertAscii, customAscii);
  verboseInfo stats{
      0,
      0,
      static_cast<int>(capture.get(cv::CAP_PROP_FPS)),
      static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT)),
  };

  cv::Mat frame;
  cv::Mat greyFrame;
  cv::Mat resizedFrame;

  struct captureDimensions frameDimensions;
  calcFrameDimensions(capture, frameDimensions, fillTerminal, showVerbose);

  std::thread fpsThread(fpsMonitor, std::ref(frameCount), std::ref(fpsCounter), std::ref(running));

  timingInfo timingInfo;
  double frameDelay = 1000.0 / capture.get(cv::CAP_PROP_FPS);

  std::thread captureThread(captureFrames, std::ref(capture), std::ref(stats), std::ref(timingInfo),
                            std::ref(running), std::ref(frameDelay));

  // system("clear");
  std::cout << "\33[H" << std::flush;

  // lazy image handling
  if (stats.totalFrames <= 1) {
    cv::Mat image = cv::imread(file);

    processFrame(image, greyFrame, resizedFrame, frameDimensions.frameWidth,
                 frameDimensions.frameHeight, timingInfo);

    if (read(STDIN_FILENO, &ch, 1)) {
      running = false;
      frameReadyCV.notify_all();
      captureThread.join();
      fpsThread.join();
      ts->restore();
      delete ts;
      return 0;
    }
  }

  while (true) {
    auto frameStart = std::chrono::high_resolution_clock::now();

    {
      std::unique_lock<std::mutex> queueLock(queueMutex);
      frameReadyCV.wait(queueLock, [&] { return !frameQueue.empty() || !running; });
      if (!running || frameQueue.empty())
        break;
      // get frame, empty q, notify
      frame = frameQueue.front();
      frameQueue.pop();
      frameReadyCV.notify_one();
    }

    processFrame(frame, greyFrame, resizedFrame, frameDimensions.frameWidth,
                 frameDimensions.frameHeight, timingInfo);

    frameCount++;            // atomic<int> for fpsMonitor
    stats.totalFrameCount++; // for total frames

    if (showVerbose) {
      displayVerbose(stats, frameDimensions, frame, timingInfo);
    }

    if (keyPressed()) { // smacking keyboard increases fps KEKW (fixed)
      ssize_t result = read(STDIN_FILENO, &ch, 1);
      if (result > 0 && (ch == 'q' || ch == 'Q')) {
        break;
      } else {
        continue;
      }
    }

    // spamming sometimes causes a crash
    calcFrameDimensions(capture, frameDimensions, fillTerminal, showVerbose);
    if (frameDimensions.frameHeight != frameDimensions.prevFrameHeight ||
        frameDimensions.frameWidth != frameDimensions.frameHeight) {
      // not a typo, condition is: not a square
      frameDimensions.prevFrameHeight = frameDimensions.frameHeight;
      frameDimensions.prevFrameWidth = frameDimensions.frameWidth;
      std::cout << "\33[2J";
    }

    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto processingTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
    double dynamicSleepTime = std::max(0.0, frameDelay - processingTime);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dynamicSleepTime)));
  }

  running = false;
  frameReadyCV.notify_all();
  captureThread.join();
  fpsThread.join();
  ts->restore();
  delete ts;
  std::cout << "\033[2J\033[1;1H";
  return 0;
}
