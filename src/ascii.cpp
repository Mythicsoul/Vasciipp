#include "ascii.hpp"

char asciiLookup[256];
std::string customAsciiCharSet;

void setupAscii(bool &invertAscii, bool &customAscii) {
  std::string asciiCharSet = " .:-=+*#%@";
  if (customAscii) {
    asciiCharSet = customAsciiCharSet;
  }
  if (invertAscii) {
    std::reverse(asciiCharSet.begin(), asciiCharSet.end());
    asciiCharSet.append("      ");
  }
  int asciiCharSetSize = asciiCharSet.size();
  for (int i = 0; i < 256; ++i) {
    asciiLookup[i] = asciiCharSet[(i * asciiCharSetSize) / 256];
  }
}

char pxToASCII(int pixelValue) { return asciiLookup[pixelValue]; }

void processFrame(const cv::Mat &frame, cv::Mat &greyFrame, cv::Mat &resizedFrame, int &frameWidth,
                  int &frameHeight, timingInfo &timingInfo) {
  auto startGray = std::chrono::high_resolution_clock::now();
  cv::cvtColor(frame, greyFrame, cv::COLOR_BGR2GRAY);
  auto endGray = std::chrono::high_resolution_clock::now();
  timingInfo.grayTime = std::chrono::duration<double, std::milli>(endGray - startGray).count();

  auto startResize = std::chrono::high_resolution_clock::now();
  cv::resize(greyFrame, resizedFrame, cv::Size(frameWidth, frameHeight));
  auto endResize = std::chrono::high_resolution_clock::now();
  timingInfo.resizeTime =
      std::chrono::duration<double, std::milli>(endResize - startResize).count();

  auto startRender = std::chrono::high_resolution_clock::now();
  std::cout << "\33[H";
  render(resizedFrame);
  auto endRender = std::chrono::high_resolution_clock::now();
  timingInfo.renderTime =
      std::chrono::duration<double, std::milli>(endRender - startRender).count();
}

void render(const cv::Mat &resizedFrame) {
  std::string output;
  output.reserve(resizedFrame.rows * (resizedFrame.cols + 1));
  for (int x = 0; x < resizedFrame.rows; ++x) {
    for (int y = 0; y < resizedFrame.cols; ++y) {
      output += asciiLookup[resizedFrame.at<uchar>(x, y)];
    }
    output += '\n';
  }
  std::cout << output << std::flush;
  // overwriting last line without appending a new one
  std::cout << output.substr(output.size() - (resizedFrame.cols + 1), resizedFrame.cols)
            << std::flush;
}
