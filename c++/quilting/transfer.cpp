#include <iostream>

#include <opencv2/opencv.hpp>

#include "quilting.h"

bool parse_arguments(int argc, char *argv[]);
void usage();

char *tex_filename = 0;
char *img_filename = 0;
int block_size = 20;
int overlap = 6;
double weight = 0.1;

int main(int argc, char *argv[]){
	cv::Mat input, image, transfer;
  char *titles[] = {(char *)"Input Texture",
					(char *)"Input Image",
					(char *)"Texture Transfer"};

  if(!parse_arguments(argc, argv)){
	usage();
	return 1;
  }
  
  input = cv::imread(argv[1]);
  image = cv::imread(argv[2]);
  
  if(input.empty()){
	cout << "error: file not found: " << tex_filename << endl;
	return 1;
  }
  if(image.empty()){
	cout << "error: file not found: " << img_filename << endl;
	return 1;
  }

  // Texture transfer
  /*std::cout << "Transferring texture" << std::endl;
  transfer = texture_transfer(image, input, block_size, block_size / overlap,
							  weight);*/

  // Create all windows
  cv::namedWindow(titles[0]);
  cv::namedWindow(titles[1]);
  cv::namedWindow(titles[2]);

  // Show all images
  cv::imshow(titles[0], input);
  cv::imshow(titles[1], image);
  cv::imshow(titles[2], transfer);

  // Wait for a key
  cv::waitKey(0);
  
  // Destroy all windows
  cv::destroyWindow(titles[0]);
  cv::destroyWindow(titles[1]);
  cv::destroyWindow(titles[2]);
}

bool parse_arguments(int argc, char *argv[]){

  if(argc < 3)
	return false;

  tex_filename = argv[1];
  img_filename = argv[2];

  if(argc > 3)
	weight = atof(argv[3]);

  if(argc > 4)
	block_size = atoi(argv[4]);

  if(argc > 5)
	overlap = atoi(argv[5]);

  return true;
}

void usage(){
	std::cout << "usage: transfer tex_filename img_filename [weigth] "
	   << "[block_size] [overlap]" << std::endl;
}
