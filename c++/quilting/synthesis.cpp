#include <iostream>

#include <opencv2/opencv.hpp>

#include <quilting.h>

using namespace std;

bool parse_arguments(int argc, char *argv[]);
void usage();

char *filename = 0;
char *output = 0;
int synth_size = 256;
int block_size = 20;
int overlap = 6;
int smooth = 1;

int main(int argc, char *argv[]){
  cv::Mat input, random_synth, overlap_synth, cut_synth;
  char *titles[] = {(char *)"Input Texture",
					(char *)"Synth. (Random)",
					(char *)"Synth. (Overlap)",
					(char *)"Synth. (Cut)"};

  if(!parse_arguments(argc, argv)){
	usage();
	return 1;
  }
  
  input = cv::imread(argv[1]);
  
  if(input.empty()){
    std::cout << "error: file not found: " << filename << std::endl;
	return 1;
  }

  // Synthesize images
  std::cout << "synthesizing random tiling" << std::endl;
  random_synth = random_tiling(input, synth_size, synth_size,
  							   block_size, block_size);
  cout << "synthesizing with overlap constraints" << endl;
  overlap_synth = overlap_constrained(input, synth_size, synth_size,
                                      block_size, block_size / overlap);
  std::cout << "synthesizing full quilting " << smooth << std::endl;
  cut_synth = boundary_cut(input, synth_size, synth_size,
						   block_size, block_size / overlap, smooth);

  // Create all windows
  cv::namedWindow(titles[0], CV_WINDOW_AUTOSIZE);
  cv::namedWindow(titles[1], CV_WINDOW_AUTOSIZE);
  cv::namedWindow(titles[2], CV_WINDOW_AUTOSIZE);
  cv::namedWindow(titles[3], CV_WINDOW_AUTOSIZE);

  // Show all images
  cv::imshow(titles[0], input);
  cv::imshow(titles[1], random_synth);
  cv::imshow(titles[2], overlap_synth);
  cv::imshow(titles[3], cut_synth);

  cv::imwrite(output, cut_synth);

  // Wait for a key
  cv::waitKey(0);
  
  // Destroy all windows
  cv::destroyWindow(titles[0]);
  cv::destroyWindow(titles[1]);
  cv::destroyWindow(titles[2]);
  cv::destroyWindow(titles[3]);
}

bool parse_arguments(int argc, char *argv[]){

  if(argc < 3)
	return false;

  filename = argv[1];
  output = argv[2];

  if(argc > 3)
	synth_size = atoi(argv[3]);

  if(argc > 4)
	block_size = atoi(argv[4]);

  if(argc > 5)
	overlap = atoi(argv[5]);

  if(argc > 6)
	smooth = atoi(argv[6]);

  return true;
}

void usage(){
  cout << "usage: synthesis filename output [synth_size] "
	   << "[block_size] [overlap] [smooth]" << endl;
}
