#include "utils.hpp"

int picIdDeg = 0;

int compression_params_array[] = {CV_IMWRITE_PNG_COMPRESSION,9};
std::vector<int> compression_params(compression_params_array, compression_params_array + 2);
//compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
//compression_params.push_back(9);

bool sortContours (std::vector<cv::Point>& i , std::vector<cv::Point>& j){

  return (cv::contourArea(i) > cv::contourArea(j));
}

bool sortBoxLeftRight(cv::Rect& i, cv::Rect& j){

  return (i.x < j.x);
}

bool sortApprox(std::vector<cv::Point>& i, std::vector<cv::Point>& j){
  cv::Rect recti = boundingRect(i);
  cv::Rect rectj = boundingRect(j);
  
  
  return (recti.x < rectj.x);
    
}

/*############################################################################*/

bool detectCards (std::vector<cv::Mat>& cards, cv::Mat& frame, int numberToDetect){

  cv::Mat mask;
  cv::Mat imageHSV;
  std::vector<std::vector<cv::Point> > approxTab;
  
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
      approxTab.push_back(approx);
      
    }
    
  }
  
  std::sort(approxTab.begin(), approxTab.end(), sortApprox);

  for(auto it = approxTab.begin(); it < approxTab.end(); ++it){

    cards.push_back(shrinkAnImage(*it, frame, 300, 500));
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
  
  for(auto it = cards.begin(); it < cards.end(); ++it){
    contours.clear();
    hierarchy.clear();
    cv::GaussianBlur(*it, imageHSV, cv::Size(5, 5), 0, 0);	
    cv::cvtColor(imageHSV, imageHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imageHSV, cv::Scalar(30, 50, 40), cv::Scalar(90, 255, 255), mask);
    
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
	minx = rightBox[i].x;
      if(rightBox[i].y <= miny)
	miny = rightBox[i].y;
      if((rightBox[i].x + rightBox[i].width) >= maxx)
	maxx = rightBox[i].x + rightBox[i].width;
      if((rightBox[i].y + rightBox[i].height) >= maxy)
	maxy = rightBox[i].y + rightBox[i].height;
    }

    keyPoints.clear();
    
    keyPoints.push_back(cv::Point2f(minx, miny));
    keyPoints.push_back(cv::Point2f(maxx, miny));
    keyPoints.push_back(cv::Point2f(maxx, maxy));
    keyPoints.push_back(cv::Point2f(minx, maxy));
    
    numbers.push_back(shrinkAnImage(keyPoints, *it, 100, 55));
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

void detectNumbers(std::vector<cv::Mat>& numbers, std::vector<int>& decriptedNumbers){
  cv::Mat imageGray;
  cv::Mat threshold;
  cv::Mat imageHSV;
  cv::Mat mask;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  std::vector<cv::Rect> boundingVect;
  
  int h = 0;
  for(auto it = numbers.begin(); it < numbers.end(); ++it){
    boundingVect.clear();
    cv::cvtColor(*it, imageGray, cv::COLOR_RGB2GRAY);
    cv::adaptiveThreshold(imageGray, threshold, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,  cv::THRESH_BINARY, 9, 0);
    cv::morphologyEx(threshold, threshold, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));
    cv::morphologyEx(threshold, threshold, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));

    std::string str = "../Data/threshold_" + std::to_string(h) + ".png";
    cv::imwrite(str, threshold, compression_params);
    
    cv::findContours( threshold, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
    std::sort (contours.begin(), contours.end(), sortContours);

    for(int i = 0; i < contours.size(); ++i){
      boundingVect.push_back(boundingRect(contours[i]));
      //cv::rectangle(*it, boundingVect[i], cv::Scalar(255, 0, 255));
    }
    int minx, maxx, miny, maxy, size = 0;
    for(int i = 0; i < boundingVect.size(); ++i){
      for(int j = i + 1; j < boundingVect.size(); ++j){
        int smallerBoxIndex = boundingVect[i].width < boundingVect[j].width ? j:i;
	int largerBoxIndex = smallerBoxIndex == i ? j : i;

	cv::Rect smallerBox = boundingVect[smallerBoxIndex];
	cv::Rect largerBox = boundingVect[largerBoxIndex];
	
	if((std::abs(boundingVect[i].x - boundingVect[j].x)) < 5){
	  // Fusion
	  if(boundingVect[i].x < boundingVect[j].x){
	    minx = boundingVect[i].x;
	  }
	  else{
	    minx = boundingVect[j].x;
	  }	    
	  if(boundingVect[i].y < boundingVect[j].y){
	    miny = boundingVect[i].y;
	  }
	  else{
	    miny = boundingVect[j].y;
	  }
	  if(boundingVect[i].x + boundingVect[i].width < boundingVect[j].x + boundingVect[j].width){
	    maxx = boundingVect[j].x + boundingVect[j].width;
	  }
	  else{
	    maxx = boundingVect[i].x + boundingVect[i].width;
	  }
	  if(boundingVect[i].y + boundingVect[i].height < boundingVect[j].y + boundingVect[j].height){
	    maxy = boundingVect[j].y + boundingVect[j].height;
	  }
	  else{
	    maxy = boundingVect[i].y + boundingVect[i].height;
	  }
	  

	  int width = cv::norm(cv::Point(minx, miny) - cv::Point(maxx, miny));
	  int height = cv::norm(cv::Point(minx, miny) - cv::Point(minx, maxy));
	  boundingVect.erase(boundingVect.begin() + i);
	  boundingVect.erase(boundingVect.begin() + (j - 1));
	  i = -1;
	  boundingVect.push_back(cv::Rect(minx, miny, width, height));
	  break;
	}
      }
    }

    std::sort (boundingVect.begin(), boundingVect.end(), sortBoxLeftRight);

    std::vector<cv::Mat> digits;
    for(int i = 0; i < boundingVect.size(); ++i){
      std::vector<cv::Point> keyPoints;
      keyPoints.push_back(cv::Point(boundingVect[i].x, boundingVect[i].y));
      keyPoints.push_back(cv::Point(boundingVect[i].x + boundingVect[i].width, boundingVect[i].y));
      keyPoints.push_back(cv::Point(boundingVect[i].x + boundingVect[i].width, boundingVect[i].y + boundingVect[i].height));
      keyPoints.push_back(cv::Point(boundingVect[i].x, boundingVect[i].y + boundingVect[i].height));
      
      digits.push_back(shrinkAnImage(keyPoints, *it, boundingVect[i].width, boundingVect[i].height));
    }
    
    int number = 0;
    for(int i = 0; i < digits.size(); ++i){
      int value = getDigit(digits[i]);
      std::cout << "(" << h << "," << i << ") : " << value << std::endl;
      std::string str = "../Data/digit_" + std::to_string(h) + std::to_string(i) + ".png";;
      cv::imwrite(str, digits[i], compression_params);
      //some magic stuff
      if(i == 0 && digits.size() == 3)
	number = number + value * 100;
      else if((i == 0 && digits.size() == 2) || (i == 1 && digits.size() == 3))
	number = number + value * 10;
      else
	number = number + value;
    }

    decriptedNumbers.push_back(number);
    ++h;
  }
}

bool hasSegment(cv::Mat& digit, int segmentIndex)
{
  int width = digit.size().width;
  int height = digit.size().height;
  
  int x,y;
  switch(segmentIndex)
    {
    case 0:
      x=width/2;
      y=3;
      break;
    case 1:
      x=width-4;
      y=height/4;
      break;
    case 2:
      x=width-4;
      y=3*height/4;
      break;
    case 3:
      x=width/2;
      y=height-4;
      break;
    case 4:
      x=3;
      y=3*height/4;
      break;
    case 5:
      x=3;
      y=height/4;
      break;
    case 6:
      x=width/2;
      y=height/2;
      break;
    default:
      std::cerr << "invalid segment index (" << segmentIndex << ") given." << std::endl;
      x=-1000;
      y=-1000;
    }
  
  bool hasGreen = false;
  for(int i=x-1; i<=x+1; ++i)
    for(int j=y-1; j<=y+1; ++j)
      {
	if(digit.at<unsigned char>(cv::Point(i,j)) != 0)
	  {
	    hasGreen = true;
	    //return true;
	  }
  }

  cv::Scalar color = hasGreen ? cv::Scalar(255,0,0) : cv::Scalar(0,0,255);
  cv::rectangle(digit,cv::Point(x-1,y-1), cv::Point(x+1,y+1), color, -1);
  std::string str = "../Data/digit_debug_" + std::to_string(picIdDeg++) + ".png";
  cv::imwrite(str, digit, compression_params);
  //return false;
  return hasGreen;
}

int getDigit(cv::Mat& digit){

  //Détection des 1
  if((digit.size().height / 3) >= digit.size().width){
    return 1;
  }
  unsigned char binarySegments = 0;
  cv::Mat rDigit, rDigitGray,  rDigitThreshold;
  cv::resize(digit,rDigit, cv::Size(50,100));
  cv::cvtColor(rDigit, rDigitGray, CV_BGR2GRAY);
  //cv::adaptiveThreshold(rDigitGray, rDigitThreshold, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,  cv::THRESH_BINARY, 19, 0);
  cv::threshold(rDigitGray,rDigitThreshold, 60, 255, 0);
 
  std::string str = "../Data/digit_threshold.png";
  cv::imwrite(str, rDigitThreshold, compression_params);
  /*int x = digit.size().width/2 , y = digit.size().height/2;

  bool hasGreenInCenter = false;
  
  for(int i=x-1; i<=x+1; ++i)
    for(int j=y-1; j<=y+1; ++j)
      {
	if(rDigitThreshold.at<unsigned char>(cv::Point(i,j)) != 0)
	  hasGreenInCenter = true;
      }
  //*/
  
  if(!hasSegment(rDigitThreshold,6)) // 0 ou 7
    {
      if(hasSegment(rDigitThreshold,4))
	return 0;
      else
	return 7;
    }
  else
    {
      if(hasSegment(rDigitThreshold,4))
	if(hasSegment(rDigitThreshold,1))
	  if(hasSegment(rDigitThreshold,5))
	    return 8;
	  else
	    return 2;
	else
	  return 6;
      else
	if(hasSegment(rDigitThreshold,0))
	  if(hasSegment(rDigitThreshold,5))
	    if(hasSegment(rDigitThreshold,1))
	      return 9;
	    else
	      return 5;
	  else
	    return 3;
	else
	  return 4;
    }

  /*
  //Detections des segments allumés
  switch(binarySegments)
    { // trust me, this will work every time
    case 0b11111010:
      return 0;
    case 0b10111110:
      return 2;
    case 0b00111110:
      return 3;
    case 0b01110100:
      return 4;
    case 0b01101110:
      return 5;
    case 0b11101110:
      return 6;
    case 0b00110010:
      return 7;
    case 0b11111110:
      return 8;
    case 0b01111110:
      return 9;
    default:
      return -100000;
      } //*/
  
  cv::rectangle(digit, cv::Point(0, 0), cv::Point(digit.size().width - 1, digit.size().height / 3), cv::Scalar(255, 0, 255));
  cv::rectangle(digit, cv::Point(0, digit.size().height / 3), cv::Point(digit.size().width - 1, (digit.size().height / 3) * 2), cv::Scalar(255, 255, 0));
  cv::rectangle(digit, cv::Point(0, (digit.size().height / 3) * 2), cv::Point(digit.size().width - 1, digit.size().height - 1), cv::Scalar(0, 255, 255));
  
  return -1000;
}
