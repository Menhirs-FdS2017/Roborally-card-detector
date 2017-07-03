#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <unistd.h>

#include "utils.hpp"

#define NUMBER_CARD_TO_DETECT 5

int main(int argc, char ** argv){

  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(9);
  
  cv::VideoCapture cap(0);
  
  cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

  double fps = cap.get(CV_CAP_PROP_FPS);
  
  int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
  int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);

  std::cout << "height: " << height << "\twidth: " << width << std::endl;
  
  int wait = 1000 / fps;

  if ( !cap.isOpened())
    {
      std::cout << "Cannot open the video file" << std::endl;
      return EXIT_FAILURE;
    }

  std::vector<cv::Mat> cards;
  std::vector<cv::Mat> numbers;
  cv::Mat frame;
    
  do
    {
      bool bSuccess = cap.read(frame);

      if (!bSuccess)
        {
	  std::cout << "Cannot read the frame from video file" << std::endl;
	   return EXIT_FAILURE;
        }

      cv::imwrite("../Data/frame.png", frame, compression_params);
      cards.clear();
      
      
    }while(!detectCards(cards, frame, NUMBER_CARD_TO_DETECT));
  
  straightenCards(cards);
  isolateNumbers(cards, numbers);

  for(int i = 0; i < numbers.size(); ++i){
    std::string str = "../Data/number_" + std::to_string(i) + ".png";
    cv::imwrite(str, numbers[i], compression_params);
  }
  for(int i = 0; i < cards.size(); ++i){
    std::string str = "../Data/card_" + std::to_string(i) + ".png";
    cv::imwrite(str, cards[i], compression_params);
  }    
  return EXIT_SUCCESS;
  
}
