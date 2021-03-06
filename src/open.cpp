#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <unistd.h>


int main(int argc, char ** argv){

    cv::Mat image;
    image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    cv::imshow( "Display window", image );                   // Show our image inside it.

    cv::waitKey(0);                                          // Wait for a keystroke in the window
    return 0;
}
