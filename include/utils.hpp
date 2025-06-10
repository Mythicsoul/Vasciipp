#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <opencv2/opencv.hpp>

extern std::atomic<int> fpsCounter;
extern const std::string colorRESET;
extern std::string selectedColor;

extern const std::map<std::string, std::string> fgColorMap;
extern const std::map<std::string, std::string> bgColorMap;

void setForegroundColor(const std::string &fgColor);
void setBackgroundColor(const std::string &bgColor);

class TerminalSettings {
public:
  TerminalSettings();
  ~TerminalSettings();
  void restore();

private:
  struct termios defaultAttr;
  struct termios vasciiAttr;
};

extern TerminalSettings *ts;

void signalHandler(int signal);

bool keyPressed();

struct verboseInfo {
  std::atomic<int> fpsCounter;
  int totalFrameCount;
  const int videoFrameRate;
  const int totalFrames;
};

struct captureDimensions {
  int frameWidth;
  int frameHeight;
  int prevFrameWidth;
  int prevFrameHeight;
};

struct timingInfo {
  double captureTime;
  double grayTime;
  double resizeTime;
  double renderTime;
};

struct terminalSize getTerminalSize();

void calcFrameDimensions(cv::VideoCapture &capture, struct captureDimensions &frameDimensions,
                         bool &fillTerminal, bool &showVerbose);
void displayVerbose(const verboseInfo &stats, struct captureDimensions &frameDimensions,
                    const cv::Mat &frame, const timingInfo &timingInfo);
void fpsMonitor(std::atomic<int> &frameCount, std::atomic<int> &fpsCounter,
                std::atomic<bool> &running);

#endif
