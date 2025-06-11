#include "utils.hpp"

#include <chrono>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <iomanip>
#include <numeric>
#include <thread>

// using namespace std;

std::atomic<int> fpsCounter(0);
const std::string colorRESET = "\033[0m";
std::string selectedColor = colorRESET;

const std::map<std::string, std::string> fgColorMap = {
    {"black", "\033[30m"},        {"red", "\033[31m"},
    {"green", "\033[32m"},        {"yellow", "\033[33m"},
    {"blue", "\033[34m"},         {"magenta", "\033[35m"},
    {"cyan", "\033[36m"},         {"white", "\033[37m"},
    {"bright_black", "\033[90m"}, {"bright_red", "\033[91m"},
    {"bright_green", "\033[92m"}, {"bright_yellow", "\033[93m"},
    {"bright_blue", "\033[94m"},  {"bright_magenta", "\033[95m"},
    {"bright_cyan", "\033[96m"},  {"bright_white", "\033[97m"},
};

const std::map<std::string, std::string> bgColorMap = {
    {"black", "\033[40m"},         {"red", "\033[41m"},
    {"green", "\033[42m"},         {"yellow", "\033[43m"},
    {"blue", "\033[44m"},          {"magenta", "\033[45m"},
    {"cyan", "\033[46m"},          {"white", "\033[47m"},
    {"bright_black", "\033[100m"}, {"bright_red", "\033[101m"},
    {"bright_green", "\033[102m"}, {"bright_yellow", "\033[103m"},
    {"bright_blue", "\033[104m"},  {"bright_magenta", "\033[105m"},
    {"bright_cyan", "\033[106m"},  {"bright_white", "\033[107m"},
};

void setForegroundColor(const std::string &fgColor) {
  if (fgColorMap.find(fgColor) != fgColorMap.end()) {
    std::cout << fgColorMap.at(fgColor);
  } else {
    std::cout << colorRESET; // fallback
  }
}

void setBackgroundColor(const std::string &bgColor) {
  if (bgColorMap.find(bgColor) != bgColorMap.end()) {
    std::cout << bgColorMap.at(bgColor);
  } else {
    std::cout << colorRESET;
  }
}

TerminalSettings::TerminalSettings() {
  tcgetattr(STDIN_FILENO, &defaultAttr);
  vasciiAttr = defaultAttr;
  vasciiAttr.c_lflag &= ~(ICANON | ECHO); // disable *cooked* mode
  tcsetattr(STDIN_FILENO, TCSANOW, &vasciiAttr);
  std::cout << "\033[?25l" << std::flush;
}

TerminalSettings::~TerminalSettings() { tcsetattr(STDIN_FILENO, TCSANOW, &defaultAttr); }

void TerminalSettings::restore() {
  std::cout << "\033[?25h" << colorRESET << std::flush;
  tcsetattr(STDIN_FILENO, TCSANOW, &vasciiAttr); // restore terminal settings
}

void signalHandler(int signal) {
  (void)signal;
  std::cout << "\nCaught SIGINT, attempting graceful exit..." << std::endl;
  ts->restore();
  exit(0);
}

bool keyPressed() {
  fd_set readfds;
  struct timeval tv = {0L, 0L};
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0;
}

void calcFrameDimensions(cv::VideoCapture &capture, struct captureDimensions &frameDimensions,
                         bool &fillTerminal, bool &showVerbose) {
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  const auto terminalWidth = ws.ws_col;
  const auto terminalHeight = ws.ws_row;
  double termAR = static_cast<double>(terminalWidth) / (static_cast<double>(terminalHeight) * 2);
  double videoAR = static_cast<double>(capture.get(cv::CAP_PROP_FRAME_WIDTH)) /
                   (capture.get(cv::CAP_PROP_FRAME_HEIGHT));

  if (fillTerminal) {
    frameDimensions.frameHeight = terminalHeight;
    frameDimensions.frameWidth = terminalWidth;
    if (showVerbose) {
      frameDimensions.frameHeight = terminalHeight - 3;
    }
  }
  if (!fillTerminal) { // have to wrap otherwise dimensions get overwritten w/ -f
    if (videoAR >= 1 && videoAR < termAR) {
      frameDimensions.frameHeight = terminalHeight;
      frameDimensions.frameWidth = (terminalHeight * 2) * videoAR;
      if (showVerbose) {
        frameDimensions.frameHeight = terminalHeight - 3;
        frameDimensions.frameWidth = (frameDimensions.frameHeight * 2) * videoAR;
      }
    } else if (videoAR >= 1 && videoAR > termAR) {
      frameDimensions.frameWidth = terminalWidth;
      frameDimensions.frameHeight = (static_cast<double>(terminalWidth) / 2) / videoAR;
    } else if (videoAR >= 1 && videoAR == termAR) {
      frameDimensions.frameHeight = terminalHeight;
      frameDimensions.frameWidth = terminalWidth;
      if (showVerbose) {
        frameDimensions.frameHeight = terminalHeight - 3;
        frameDimensions.frameWidth = (frameDimensions.frameHeight * 2) * videoAR;
      }
    } else if (videoAR < 1 && videoAR == termAR) {
      frameDimensions.frameWidth = terminalWidth;
      frameDimensions.frameHeight = terminalHeight;
      if (showVerbose) {
        frameDimensions.frameHeight = terminalHeight - 3;
        frameDimensions.frameWidth = (frameDimensions.frameHeight * 2) * videoAR;
      }
    } else if (videoAR < 1 && videoAR < termAR) {
      frameDimensions.frameHeight = terminalHeight;
      frameDimensions.frameWidth = (terminalHeight * 2) * videoAR;
      if (showVerbose) {
        frameDimensions.frameHeight = terminalHeight - 3;
        frameDimensions.frameWidth = (frameDimensions.frameHeight * 2) * videoAR;
      }
    } else if (videoAR < 1 && videoAR > termAR) {
      frameDimensions.frameWidth = terminalWidth;
      frameDimensions.frameHeight = (static_cast<double>(terminalWidth) / 2) / videoAR;
    }
  }
  frameDimensions.prevFrameHeight = frameDimensions.frameHeight;
  frameDimensions.prevFrameWidth = frameDimensions.frameWidth;
}

void displayVerbose(const verboseInfo &stats, struct captureDimensions &frameDimensions,
                    const cv::Mat &frame, const timingInfo &timingInfo) {
  std::ostringstream ss;
  ss << "\nResolution: " << frame.cols << "x" << frame.rows << "@" << stats.videoFrameRate << " ("
     << frame.cols / std::gcd(frame.cols, frame.rows) << ":"
     << frame.rows / std::gcd(frame.cols, frame.rows) << ")";

  ss << "\n"
     << frameDimensions.frameWidth << "x" << frameDimensions.frameHeight << " Cells"
     << " | FPS: " << fpsCounter.load() << " | Frames: " << stats.totalFrameCount << "/"
     << stats.totalFrames;

  ss << std::fixed << std::setprecision(1)
     << "\nCapture: " << timingInfo.captureTime << " | Grey: " << timingInfo.grayTime
     << " | Resize: " << timingInfo.resizeTime << " | Render: " << timingInfo.renderTime
     << " (in ms)        ";
  std::cout << ss.str() << std::flush;
}

void fpsMonitor(std::atomic<int> &fps, std::atomic<int> &fpsCounter, std::atomic<bool> &running) {
  while (running) { // updates fps counter
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    fpsCounter.store(fps.exchange(0));
  }
}
