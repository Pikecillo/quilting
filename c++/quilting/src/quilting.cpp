#include <iostream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <math.h>

#if 0
//#include <highgui.h>

#include <opencv2/opencv.hpp>

#include "quilting.h"
#include "util.h"

using namespace std;

/*
  The most naive approach: tile them by picking random patches
  input is the sample texture, output is the synthesized texture
*/
cv::Mat *random_tiling(cv::Mat *input, int w, int h,
							int w_block, int h_block){
  
  cv::Mat output = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
  pair<int, int> coord;
  
  // Seed
  srand(0);
  
  // Fill the new image with randomly chosen patches
  // from input texture
  for(int i_out = 0 ; i_out < output->height ; i_out += h_block)
    for(int j_out = 0 ; j_out < output->width ; j_out += w_block){

      // Pick random upper left corners for blocks
	  coord = random_patch(input, h_block, w_block);
	  // Paste it to output
      copy_block(input, output, coord.first, coord.second, i_out, j_out,
				 w_block, h_block);
    }
  
  return output;
}

/*
  Picks a random patch
 */
pair<int, int> random_patch(cv::Mat input, int h_block, int w_block){
  pair<int, int> coord;
  
  coord.first = rand() % (input->height - h_block);
  coord.second = rand() % (input->width - w_block);

  return coord;
}

/*
  Copies a block from input to output
*/
void copy_block(cv::Mat input, cv::Mat output,
				int i_in, int j_in, int i_out, int j_out,
				int w_block, int h_block){
  CvScalar pixel;
  
  for(int i = 0 ; i < h_block ; i++)
    for(int j = 0 ; j < w_block ; j++){
	  pixel = cvGet2D(input, i + i_in, j + j_in);
	  
	  if(i + i_out < output->height &&
		 j + j_out < output->width)
		cvSet2D(output, i + i_out, j + j_out, pixel);
	}
}

/*
  A less naive approach: choose patch that best match
  the current output image
*/
cv::Mat overlap_constrained(cv::Mat input, int w, int h,
							  int block_size, int overlap){
  
  cv::Mat output = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
  pair<int, int> best;
  int step = block_size - overlap;

  // Seed
  srand(0);

  // Fill the image with best matching pathces
  // left to right and top to bottom
  for(int i_out = 0 ; i_out < output->height ; i_out += step)
    for(int j_out = 0 ; j_out < output->width ; j_out += step){

	  // First path is random
	  if(i_out == 0 && j_out == 0)
		best = random_patch(input, block_size, block_size);
	  else // Pick best matching patch
		best = pick_best_match(input, output, i_out, j_out,
							   overlap, block_size);
	  
	  // Paste patch on output image
      copy_block(input, output, best.first, best.second,
				 i_out, j_out, block_size, block_size);
    }
  
  return output;
}

/*
  Picks the best matching patch for a current stage of the quilting
*/
pair<int, int> pick_best_match(cv::Mat input, cv::Mat output,
							   int i_out, int j_out,
							   int overlap, int block_size){
  pair<int, int> coord;
  double best = 1E10, error;
  
  // Go through all possible patches to find best match
  for(int i_in = 0 ; i_in < input->height - block_size ; i_in++)
    for(int j_in = 0 ; j_in < input->width - block_size ; j_in++){
      error = overlap_error(input, output, i_in, j_in,
							i_out, j_out, overlap, block_size);
	  
      if(error <= best ){
		best = error;
		coord.first = i_in;
		coord.second = j_in;
      }
    }
  
  return coord;
}

/*
  Calculates the overlap error
*/
double overlap_error(cv::Mat input, cv::Mat output,
					 int i_in, int j_in, int i_out, int j_out,
					 int overlap, int block_size){
  int orientation = 0;
  double error = 0.0;
  CvScalar input_pixel, output_pixel;
  
  // Calculate error for both the upper and left overlap
  // If orientation == 0 left is calculated, otherwise is the upper one
  while(orientation <= 1){
	
    // If the patch is not to be positioned on a border
    if((orientation? i_out : j_out) != 0){
      for(int i = 0 ; i < (orientation? overlap : block_size) ; i++){
		for(int j = 0 ; j < (orientation? block_size : overlap) ; j++){
		  
		  // Make sure indexes are on range
		  if(i + i_in < input->height &&
			 j + j_in < input->width &&
			 i + i_out < output->height &&
			 j + j_out < output->width){
			
			input_pixel = cvGet2D(input, i + i_in, j + j_in);
			output_pixel = cvGet2D(output, i + i_out, j + j_out);
			
			// Get square of difference of RGB values of pixels
			for(int k = 0 ; k < 3 ; k++)
			  error += SQUARE(input_pixel.val[k] - output_pixel.val[k]);
		  }
		}
      }
    }
	
    orientation++;
  }
  
  return sqrt(error);
}

/*
  Quilting approach: do the same as overlap constrained but cut in overlap
  regions through the minimal error paths
 */
cv::Mat boundary_cut(cv::Mat input, int w, int h,
					   int block_size, int overlap, bool smooth){
  
  cv::Mat output = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
  pair<int, int> best;
  int *vertical_cut, *horizontal_cut, step = block_size - overlap;
  double **region, **temp_region;
  
  // Allocate memory for arrays
  allocate(overlap, block_size, vertical_cut, horizontal_cut,
		   region, temp_region);
  
  // Paste a patch with upper left corner at i, j
  for(int i_out = 0 ; i_out < output->height ; i_out += step)
    for(int j_out = 0 ; j_out < output->width ; j_out += step){
	  
      // First path is random
	  if(i_out == 0 && j_out == 0)
		best = random_patch(input, block_size, block_size);
	  else // Pick best matching patch
		best = pick_best_match(input, output, i_out, j_out,
							   overlap, block_size);

	  cout << "Finding patch for " << i_out << " " << j_out
		   << ", found " << best.first << " " << best.second <<endl;

	  // Find best cuts
      find_min_cuts(input, output, best.first, best.second, i_out, j_out,
					overlap, block_size, vertical_cut, horizontal_cut,
					region, temp_region);
	  // Paste according to cut
	  copy_round_cut(input, output, best, i_out, j_out,
					 vertical_cut, horizontal_cut, block_size);
	  
	  // Smooth those cuts
	  if(smooth)
		smooth_cuts(output, i_out, j_out, vertical_cut,
					horizontal_cut, block_size);
    }
  
  // Deallocate memory
  deallocate(overlap, block_size, vertical_cut, horizontal_cut,
			 region, temp_region);
  
  return output;
}

/*
  Allocates memory for arrays required for quilting
*/
void allocate(int overlap, int block_size,
			  int *&vertical_cut, int *&horizontal_cut,
			  double **&region, double **&temp_region){
  
  // Allocate memory for arrays
  vertical_cut = new int[block_size];
  horizontal_cut = new int[block_size];
  region = new double*[block_size];
  temp_region = new double*[block_size];
  
  for(int i = 0 ; i < block_size ; i++){
	region[i] = new double[overlap];
    temp_region[i] = new double[overlap];
  }
}

/*
  Deallocates memory for arrays required for quilting
*/
void deallocate(int overlap, int block_size,
				int *&vertical_cut, int *&horizontal_cut,
				double **&region, double **&temp_region){
  
  for(int i = 0 ; i < block_size ; i++){
    delete [] region[i];
	delete [] temp_region[i];
  }
  
  delete [] vertical_cut;
  delete [] horizontal_cut;
  delete [] region;
  delete [] temp_region;
}

/*
  Find minimum vertical and horizontal cuts for the
  corresponding overlap regions
 */
void find_min_cuts(cv::Mat input, cv::Mat output, int i_in, int j_in,
				   int i_out, int j_out, int overlap, int block_size,
				   int *vertical_cut, int *horizontal_cut,
				   double **region, double **temp){

  // Initialize cuts to zero
  for(int i = 0 ; i < block_size ; i++)
    vertical_cut[i] = horizontal_cut[i] = 0;

  // If the patch is to be located in left border
  if(j_out != 0){
	
	// Compute vertical error region
	error_region(input, output, i_in, j_in, i_out, j_out,
				 block_size, overlap, region, true);
	// Compute vertical minimum error path
	minimum_cut(region, vertical_cut,
				block_size, overlap, temp, true);
  }

  if(i_out != 0){
	
	// Compute horizontal error region
	error_region(input, output, i_in, j_in, i_out, j_out,
				 block_size, overlap, region, false);
	
	// Compute horizontal minimum error path
	minimum_cut(region, horizontal_cut,
				block_size, overlap, temp, false);
  }
}

/*
  Computes error region in overlap
 */
void error_region(cv::Mat input, cv::Mat output,
				  int i_in, int j_in, int i_out, int j_out,
				  int block_size, int overlap,
				  double **region, bool vertical){
  CvScalar input_pixel, output_pixel;
  int real_i_out, real_j_out, real_i_in, real_j_in;

  // For each pixel in the overlap region
  for(int i = 0 ; i < block_size ; i++)
	for(int j = 0 ; j < overlap ; j++){
	  
	  // Indexes in image and texture space
	  real_i_in = vertical ? i + i_in : j + j_in;
	  real_j_in = vertical ? j + j_in : i + i_in;
	  real_i_out = vertical ? i + i_out : j + j_out;
	  real_j_out = vertical ? j + j_out : i + i_out;

	  // Make sure indexes are on range
	  if(real_i_in < input->height && real_j_in < input->width &&
		 real_i_out < output->height && real_j_out < output->width){
		
		input_pixel = cvGet2D(input, real_i_in, real_j_in);
		output_pixel = cvGet2D(output, real_i_out, real_j_out);
		
		region[i][j] = 0.0;
		
		// Get square of difference of pixels
		for(int k = 0 ; k < 3 ; k++)
		  region[i][j] +=
			SQUARE(input_pixel.val[k] - output_pixel.val[k]);
	  }
	  else{
		region[i][j] = 1E10;
	  }
	}
}

/*
  Find the minimum error path through an error region
 */
void minimum_cut(double **overlap_region, int *cut_index,
				 int block_size, int overlap, double **temp,
				 bool vertical){  
  int min, e1, e2, e3, c1, c2;

  for(int j = 0 ; j < overlap ; j++)
	temp[0][j] = overlap_region[0][j];
  
  for(int i = 1 ; i < block_size ; i++)
	for(int j = 0 ; j < overlap ; j++){
	  
	  e1 = (j == 0 ? 1E10 : overlap_region[i - 1][j - 1]);
	  e2 = overlap_region[i - 1][j];
	  e3 = (j == overlap - 1 ? 1E10 : overlap_region[i - 1][j + 1]);
	  
	  temp[i][j] = overlap_region[i][j] + MIN3(e1, e2, e3);
	}
  
  // Reconstruct cut
  // Find start
  min = 0;
  for(int j = 1 ; j < overlap ; j++){
	if(temp[block_size - 1][j] < temp[block_size - 1][min])
	  min = j;
  }
  
  cut_index[block_size - 1] = min;
  
  // Keep going
  for(int i = block_size - 2 ; i >= 0 ; i--){
	
	// Start from central
	min = cut_index[i + 1];
	c1 = (min != 0 ? min - 1 : 0);
	c2 = (min != overlap - 1 ? min + 1 : 0);
	
	// If better go up
	if(c1 && temp[i][c1] < temp[i][min])
	  min = c1;
	if(c2 && temp[i][c2] < temp[i][min])

	  min = c2;
	
	cut_index[i] = min;
  }
}

/*
  Paste patch around cut, and smooth
 */
void copy_round_cut(cv::Mat input, cv::Mat output, pair<int, int> best,
					int i_out, int j_out, int *vertical_cut,
					int *horizontal_cut, int block_size){
  CvScalar pixel;
 
  // Copy pixels from approapriate patch
  for(int i = 0 ; i < block_size ; i++){
	for(int j = 0 ; j < block_size ; j++){
	  
	  // Copy pixel from patch to output image
	  // if it is on the appropriate side of the cut
	  // This comparison right here is like that
	  // because the points in the cut
	  // are expressed in local coordinates for the patch
	  if(j >= vertical_cut[i] && i >= horizontal_cut[j]){
		// Get the pixel from the patch	
		pixel = cvGet2D(input, i + best.first, j + best.second);
		
		/*if (j == vertical_cut[i] || i == horizontal_cut[j]){
		  pixel.val[0] = pixel.val[1] = 0;
		  pixel.val[2] = 256;
		  }*/

		// Keep pixels in range
		if(i_out + i < output->height &&
		   j_out + j < output->width)
		  cvSet2D(output, i_out + i, j_out + j, pixel);
	  }
	}
  }
}

void smooth_cuts(cv::Mat output, int i_out, int j_out,
				 int *vertical_cut, int *horizontal_cut, int block_size){
  CvScalar pixel;
  int i_temp, j_temp;

  // Smooth cuts
  for(int i = 1 ; i < block_size - 1 ; i++){
	for(int k = 0 ; k <= 1 ; k++){
	  
	  // Smooth vertical cut
	  i_temp = i_out + i;
	  j_temp = j_out + vertical_cut[i] + k;
	  
	  if(i_temp < output->height && j_temp < output->width &&
		 i_temp > 0 && j_temp > 0){
		pixel = window_average(output, i_temp, j_temp, 3);
		cvSet2D(output, i_temp, j_temp, pixel);
	  }

	  // Smooth vertical cut
	  i_temp = i_out + horizontal_cut[i] + k;
	  j_temp = j_out + i;
	  
	  if(i_temp < output->height && j_temp < output->width &&
		 i_temp > 0 && j_temp > 0){
		pixel = window_average(output, i_temp, j_temp, 3);
		cvSet2D(output, i_temp, j_temp, pixel);
	  }
	}
  }
}

/*
  Transfer a texture to an image, based on a correspondence map
 */
cv::Mat texture_transfer(cv::Mat image, cv::Mat texture,
						   int block_size, int overlap, double alpha){
  int width = image->width, height = image->height,
	step = block_size - overlap, *vertical_cut, *horizontal_cut;
  cv::Mat image_luminance, *texture_luminance;
  cv::Mat output = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  pair<int, int> best;
  double **region, **temp;

  // Compute luminance images
  image_luminance = blurred_luminance_image(image, 7);
  texture_luminance = blurred_luminance_image(texture, 7);

  // Allocate arrays
  allocate(overlap, block_size, vertical_cut, horizontal_cut, region, temp);

  cvNamedWindow("Grow", CV_WINDOW_AUTOSIZE);
  
  // Find all patches
  for(int i_out = 0 ; i_out < height ; i_out += step)
	for(int j_out = 0 ; j_out < width ; j_out += step){

	  // Find best patch
	  best = best_correspondence_match(output, image, texture,
									   image_luminance, texture_luminance,
									   i_out, j_out, block_size, overlap,
									   alpha);
	  
	  cout << "Finding patch for " << i_out << " " << j_out
		   << ", found " << best.first << " " << best.second <<endl;

	  // Find best cut
	  find_min_cuts(texture, output, best.first, best.second, i_out, j_out,
					overlap, block_size, vertical_cut, horizontal_cut,
					region, temp);
	  
	  // Paste patch on output image
      copy_round_cut(texture, output, best, i_out, j_out,
					 vertical_cut, horizontal_cut, block_size);

	  // Smooth those cuts
	  smooth_cuts(output, i_out, j_out, vertical_cut,
				  horizontal_cut, block_size);

	  cvShowImage("Grow", output);

	  // Wait for a key
	  cvWaitKey(20);
	}

  cvDestroyWindow("Grow");

  // Deallocate arrays
  deallocate(overlap, block_size, vertical_cut, horizontal_cut,
			 region, temp);

  return output;
}

// A nice value for alpha is 0.1
pair <int, int> best_correspondence_match(cv::Mat output,
                                          cv::Mat image, cv::Mat texture,
                                          cv::Mat image_luminance,
                                          cv::Mat texture_luminance,
										  int i_out, int j_out,
										  int block_size, int overlap,
										  double alpha){
  double image_error, luminance_error, best = 1E10, error;
  pair<int, int> coord;
  
  // Go through all possible patches to find best match
  for(int i_tex = 0 ; i_tex < (texture->height - block_size) ; i_tex++)
    for(int j_tex = 0 ; j_tex < (texture->width - block_size) ; j_tex++){
	  // Find error in image and their luminances
      image_error = overlap_error(output, texture, i_out, j_out,
	  						  i_tex, j_tex, overlap, block_size);
	  luminance_error = full_region_error(image_luminance, texture_luminance,
										  i_out, j_out, i_tex, j_tex,
										  block_size);

	  // Weight errors
	  error = alpha * image_error + (1 - alpha) * luminance_error;

      if(error <= best){
		best = error;
		coord.first = i_tex;
		coord.second = j_tex;
      }
    }
 
  return coord;
}

/*
  This error is meant to be used with luminance, it takes into account
  the whole region
 */
double full_region_error(cv::Mat input, cv::Mat output,
						 int i_in, int j_in, int i_out, int j_out,
						 int block_size){
  double error = 0.0;
  CvScalar input_pixel, output_pixel;
  
  for(int i = 0 ; i < block_size ; i++){
	for(int j = 0 ; j < block_size ; j++){
	  
	  // Make sure indexes are on range
	  if(i + i_in < input->height &&
		 j + j_in < input->width &&
		 i + i_out < output->height &&
		 j + j_out < output->width){
		
		input_pixel = cvGet2D(input, i + i_in, j + j_in);
		output_pixel = cvGet2D(output, i + i_out, j + j_out);
		
		// Get square of difference of RGB values of pixels
		for(int k = 0 ; k < 3 ; k++)
		  error += SQUARE(input_pixel.val[k] - output_pixel.val[k]);
	  }
	}
  }  
  
  return sqrt(error);
}
#endif
