#include "utils.hpp"


bool sortContours (std::vector<cv::Point>& i , std::vector<cv::Point>& j){

  return (cv::contourArea(i) > cv::contourArea(j));
}

/*############################################################################*/

bool detectCards (std::vector<cv::Mat>& cards, cv::Mat& frame, int numberToDetect){

  cv::Mat mask;
  cv::Mat imageHSV;
  
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(9);
  
  // Convert the image in HSV
  cv::cvtColor(frame, imageHSV, cv::COLOR_BGR2HSV);

  // Detect all the pixels that are not white
  cv::inRange(imageHSV, cv::Scalar(0, 0, 100), cv::Scalar(250, 100, 255), mask);
  cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));
  cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));
  cv::bitwise_not(mask, mask);

  cv::imwrite("../Data/mask.png", mask, compression_params);

  // Getting the edge map on the image
  cv::Canny(mask, mask, 200, 500, 3);
  
  cv::imwrite("../Data/edged.png", mask, compression_params);

  // Find contours in the edge map, then sort them by their size in descending order

  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;

  cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
  
  std::sort(contours.begin(), contours.end(), sortContours);

  std::vector<cv::Point> approx;
  cv::Point tmp;
  for(auto it = contours.begin(); it < contours.end(); ++it) {
    
    cv::approxPolyDP(*it, approx, 0.02 * cv::arcLength(*it, true), true);

    if (approx.size() == 4 && isRectangle(approx)){
      
      tmp = approx[3];
      approx[3] = approx[1];
      approx[1] = tmp;
      cards.push_back(shrinkAnImage(approx, frame, 300, 500));
    }
    
  }

  std::cout << cards.size() << std::endl;
  
  if(cards.size() == numberToDetect)
    return true;
  else
    return false;
  
}

/*############################################################################*/

cv::Mat shrinkAnImage (std::vector<cv::Point>& keyPoints, cv::Mat& frame, size_t width, size_t height){

  cv::Mat image;
  std::vector<cv::Point2f> quad_pts;
  std::vector<cv::Point2f> squre_pts;

  quad_pts.push_back(cv::Point2f(keyPoints[0].x, keyPoints[0].y));
  quad_pts.push_back(cv::Point2f(keyPoints[1].x, keyPoints[1].y));
  quad_pts.push_back(cv::Point2f(keyPoints[2].x, keyPoints[2].y));
  quad_pts.push_back(cv::Point2f(keyPoints[3].x, keyPoints[3].y));
 
  squre_pts.push_back(cv::Point2f(0, 0));
  squre_pts.push_back(cv::Point2f(width, 0));
  squre_pts.push_back(cv::Point2f(width, height));
  squre_pts.push_back(cv::Point2f(0, height));

  cv::Mat transform_matrix = cv::getPerspectiveTransform(quad_pts, squre_pts);
  cv::warpPerspective(frame, image, transform_matrix, cv::Size(width, height));
  return image;
}

/*############################################################################*/

bool isRectangle(std::vector<cv::Point>& keyPoints, size_t delta){
  int x1, x2, x3;
  int y1, y2, y3;

  x1 = std::abs(keyPoints[0].x - keyPoints[1].x) <= delta && std::abs(keyPoints[2].x - keyPoints[3].x) <= delta;
  x2 = std::abs(keyPoints[0].x - keyPoints[2].x) <= delta && std::abs(keyPoints[1].x - keyPoints[3].x) <= delta;
  x3 = std::abs(keyPoints[0].x - keyPoints[3].x) <= delta && std::abs(keyPoints[1].x - keyPoints[2].x) <= delta;

  y1 = std::abs(keyPoints[0].y - keyPoints[1].y) <= delta && std::abs(keyPoints[2].y - keyPoints[3].y) <= delta;
  y2 = std::abs(keyPoints[0].y - keyPoints[2].y) <= delta && std::abs(keyPoints[1].y - keyPoints[3].y) <= delta;
  y3 = std::abs(keyPoints[0].y - keyPoints[3].y) <= delta && std::abs(keyPoints[1].y - keyPoints[2].y) <= delta;

  return ( (x1 || x2 || x3) && (y1 || y2 || y3) );
  
}

/*############################################################################*/

void isolateNumbers(std::vector<cv::Mat>& cards, std::vector<cv::Mat>& numbers){

  cv::Mat imageHSV;
  cv::Mat mask;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Rect> rightBox;
  std::vector<cv::Vec4i> hierarchy;
  std::vector<cv::Point> keyPoints;

  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(9);
  
  for(auto it = cards.begin(); it < cards.end(); ++it){
    contours.clear();
    hierarchy.clear();
    cv::GaussianBlur(*it, imageHSV, cv::Size(5, 5), 0, 0);	
    cv::cvtColor(imageHSV, imageHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imageHSV, cv::Scalar(30, 60, 40), cv::Scalar(90, 255, 255), mask);
    
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10)));
	
    cv::Canny(mask, mask, 200, 500, 3);
    
    cv::findContours( mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
    std::sort (contours.begin(), contours.end(), sortContours);
    cv::Rect boundRect;
    int minx = 3000;
    int maxx = 0;
    int miny = 3000;
    int maxy = 0;
    rightBox.clear();
    for(auto contoursIterator = contours.begin(); contoursIterator < contours.end(); ++contoursIterator){
      boundRect = boundingRect(*contoursIterator);
      if(boundRect.x > 150 && boundRect.y < 250){
	rightBox.push_back(boundRect);
      }     
    }
    
    for(int i = 0; i < rightBox.size(); ++i){
	if(rightBox[i].x <= minx)
	  minx = rightBox[i].x - 5;
	if(rightBox[i].y <= miny)
	  miny = rightBox[i].y - 5;
	if((rightBox[i].x + rightBox[i].width) >= maxx)
	  maxx = rightBox[i].x + rightBox[i].width + 5;
	if((rightBox[i].y + rightBox[i].height) >= maxy)
	  maxy = rightBox[i].y + rightBox[i].height + 5;
      }

    keyPoints.clear();
    
    keyPoints.push_back(cv::Point2f(minx, miny));
    keyPoints.push_back(cv::Point2f(maxx, miny));
    keyPoints.push_back(cv::Point2f(maxx, maxy));
    keyPoints.push_back(cv::Point2f(minx, maxy));

    int width = cv::norm(keyPoints[0] - keyPoints[1]);
    int height = cv::norm(keyPoints[0] - keyPoints[3]);
    
    numbers.push_back(shrinkAnImage(keyPoints, *it, width, height));
  }
  
}

/*############################################################################*/

void straightenCards(std::vector<cv::Mat>& cards){

  cv::Mat imageHSV;
  cv::Mat mask;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::Rect boundRect;
  std::vector<cv::Point> keyPoints;

  for(auto it = cards.begin(); it < cards.end(); ++it){

    cv::GaussianBlur(*it, imageHSV, cv::Size(5, 5), 0, 0);	
    cv::cvtColor(imageHSV, imageHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imageHSV, cv::Scalar(30, 60, 40), cv::Scalar(90, 255, 255), mask);
    
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10)));
	
    cv::Canny(mask, mask, 200, 500, 3);
    
    cv::findContours( mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
    std::sort (contours.begin(), contours.end(), sortContours);


    boundRect = boundingRect(contours[0]);
    keyPoints.clear();
    if(boundRect.x < 150 && boundRect.y < 250){
      keyPoints.push_back(cv::Point2f(0, 500));
      keyPoints.push_back(cv::Point2f(0, 0));
      keyPoints.push_back(cv::Point2f(300, 0));
      keyPoints.push_back(cv::Point2f(300, 500));
      *it = shrinkAnImage(keyPoints, *it, 300, 500);
    }
    else if(boundRect.x < 150 && boundRect.y > 250){
      keyPoints.push_back(cv::Point2f(300, 500));
      keyPoints.push_back(cv::Point2f(0, 500));
      keyPoints.push_back(cv::Point2f(0, 0));
      keyPoints.push_back(cv::Point2f(300, 0));
      *it = shrinkAnImage(keyPoints, *it, 300, 500);
    }
    else if(boundRect.x > 150 && boundRect.y > 250){
      keyPoints.push_back(cv::Point2f(300, 0));
      keyPoints.push_back(cv::Point2f(300, 500));
      keyPoints.push_back(cv::Point2f(0, 500));
      keyPoints.push_back(cv::Point2f(0, 0));
      *it = shrinkAnImage(keyPoints, *it, 300, 500);
    }
    
  }
  
}
