#pragma once

#include <cv.h>
#include <utility>

using namespace std;

#define SQUARE(a) ((a) * (a))
#define MIN2(a, b) (((a) < (b)) ? (a) : (b))
#define MIN3(a, b, c) (MIN2(a, MIN2(b, c)))

cv::Mat  random_tiling(cv::Mat input, int w, int h,
						int w_block, int h_block);
pair<int, int> random_patch(cv::Mat input, int h_block, int w_block);
void copy_block(cv::Mat  input, cv::Mat output,
				int i_in, int j_in, int i_out, int j_out,
				int w_block, int h_block);

cv::Mat  overlap_constrained(cv::Mat input, int w, int h,
			      int block_size, int overlap);
pair<int, int> pick_best_match(cv::Mat input, cv::Mat output,
							   int i_out, int j_out,
							   int overlap, int block_size);
double overlap_error(cv::Mat input, cv::Mat output,
		     int i_in, int j_in, int i_out, int j_out,
		     int overlap, int block_size);

cv::Mat boundary_cut(cv::Mat input, int w, int h,
					   int block_size, int overlap, bool smooth);
void allocate(int overlap, int block_size,
			  int *&vertical_cut, int *&horizontal_cut,
			  double **&region, double **&temp_region);
void deallocate(int overlap, int block_size,
				int *&vertical_cut, int *&horizontal_cut,
				double **&region, double **&temp_region);
void find_min_cuts(cv::Mat input, cv::Mat output, int i_in, int j_in,
				   int i_out, int j_out, int overlap, int block_size,
				   int *vertical_cut, int *horizontal_cut,
				   double **region, double **temp);
void error_region(cv::Mat input, cv::Mat output,
				  int i_in, int j_in, int i_out, int j_out,
				  int block_size, int overlap,
				  double **region, bool vertical);
void minimum_cut(double **overlap_region, int *cut_index,
				 int block_size, int overlap, double **temp,
				 bool vertical);
void copy_round_cut(cv::Mat input, cv::Mat output, pair<int, int> best,
					int i_out, int j_out, int *vertical_cut,
					int *horizontal_cut, int block_size);
void smooth_cuts(cv::Mat output, int i_out, int j_out,
				 int *vertical_cut, int *horizontal_cut, int block_size);

cv::Mat texture_transfer(cv::Mat image, cv::Mat texture,
						   int block_size, int overlap, double weight);
pair <int, int> best_correspondence_match(cv::Mat output,
                                          cv::Mat image, cv::Mat texture,
                                          cv::Mat image_luminance,
                                          cv::Mat texture_luminance,
										  int i_image, int j_image,
										  int block_size, int overlap,
										  double weight);
double full_region_error(cv::Mat input, cv::Mat output,
						 int i_in, int j_in, int i_out, int j_out,
						 int block_size);
