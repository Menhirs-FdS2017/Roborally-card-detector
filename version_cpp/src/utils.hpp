#ifndef _UTILS_H
#define _UTILS_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>

/**
 * Sort the contours in revers order
 */
bool sortContours (std::vector<cv::Point>& i , std::vector<cv::Point>& j);

/**
 * Detect all the card in the image and fill @cards with thoses cards
 */
bool detectCards (std::vector<cv::Mat>& cards, cv::Mat& frame, int numberToDetect);

/*
 * Return an image with only the parts that are wanted in @frame 
 */
cv::Mat shrinkAnImage (std::vector<cv::Point>& keyPoints, cv::Mat& frame, size_t width, size_t height);

bool isRectangle(std::vector<cv::Point>& keyPoints, size_t delta = 250);

void isolateNumbers(std::vector<cv::Mat>& cards, std::vector<cv::Mat>& numbers);

void straightenCards(std::vector<cv::Mat>& cards);


#endif
