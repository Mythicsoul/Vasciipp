#ifndef ASCII_HPP
#define ASCII_HPP

#include "utils.hpp"
#include <opencv2/opencv.hpp>

extern char asciiLookup[256];
extern std::string customAsciiCharSet;

void setupAscii(bool &invertAscii, bool &customAscii);
char pixelToASCII(int pixelValue);
void processFrame(const cv::Mat &frame, cv::Mat &greyFrame, cv::Mat &resizedFrame, int &frameWidth,
                  int &frameHeight, timingInfo &timingInfo);
void render(const cv::Mat &resizedFrame);

#endif
