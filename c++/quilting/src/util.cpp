#include <algorithm>
#include <vector>
#include "util.h"

using namespace std;

cv::Mat luminance_image(cv::Mat image){
  int w = image->width, h = image->height, intensity;
  cv::Mat luminance = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
  CvScalar pixel;

  for(int i = 0 ; i < h ; i++)
	for(int j = 0 ; j < w ; j++){
	  pixel = cvGet2D(image, i, j);

	  intensity = (pixel.val[0] + pixel.val[1] + pixel.val[2]) / 3;
	  pixel.val[0] = pixel.val[1] = pixel.val[2] = intensity;

	  cvSet2D(luminance, i, j, pixel);
	}

  return luminance;
}

cv::Mat blurred_luminance_image(cv::Mat image, int window_size){
  cv::Mat blurred_luminance;
  
  blurred_luminance = luminance_image(image);
  average_filter(blurred_luminance, window_size);

  return blurred_luminance;
}

void average_filter(cv::Mat image, int window_size){
  int w = image->width, h = image->height;
  CvScalar pixel;
  cv::Mat copy = cvCloneImage(image);

  // Go through all pixels
  for(int i = 0 ; i < h ; i++){
	for(int j = 0 ; j < w ; j++){
	  pixel = window_average(copy, i, j, window_size);
	  cvSet2D(image, i, j, pixel);
	}
  }
  
  cvReleaseImage(&copy);
}

void median_filter(cv::Mat image, int window_size){
  int w = image->width, h = image->height;
  CvScalar pixel;
  cv::Mat copy = cvCloneImage(image);

  // Go through all pixels
  for(int i = 0 ; i < h ; i++){
	for(int j = 0 ; j < w ; j++){
	  pixel = window_median(copy, i, j, window_size);
	  cvSet2D(image, i, j, pixel);
	}
  }

  cvReleaseImage(&copy);
}

CvScalar window_average(cv::Mat image, int i, int j, int window_size){
  int w = image->width, h = image->height, ik, jk, r, g, b,
	half_size = window_size / 2;
  CvScalar pixel;

  r = g = b = 0;
  
  // At boundaries the nearest pixel is taken
  for(int m = -half_size ; m <= half_size ; m++)
	for(int n = -half_size ; n <= half_size ; n++){
	  ik = i + m;
	  jk = j + n;
		  
	  if(ik < 0) ik = 0;
	  if(ik >= h) ik = h - 1;
	  if(jk < 0) jk = 0;
	  if(jk >= w) jk = w - 1;
	  
	  pixel = cvGet2D(image, ik, jk);
	  
	  b += pixel.val[0];
	  g += pixel.val[1];
	  r += pixel.val[2];
	}
  
  pixel.val[0] = b / (window_size * window_size);
  pixel.val[1] = g / (window_size * window_size);
  pixel.val[2] = r / (window_size * window_size);

  return pixel;
}

CvScalar window_median(cv::Mat image, int i, int j, int window_size){
  int w = image->width, h = image->height, ik, jk,
	half_size = window_size / 2;
  CvScalar pixel;
  vector<int> r, g, b;

  // At boundaries the nearest pixel is taken
  for(int m = -half_size ; m <= half_size ; m++)
	for(int n = -half_size ; n <= half_size ; n++){
	  ik = i + m;
	  jk = j + n;
	  
	  if(ik < 0) ik = 0;
	  if(ik >= h) ik = h - 1;
	  if(jk < 0) jk = 0;
	  if(jk >= w) jk = w - 1;
	  
	  pixel = cvGet2D(image, ik, jk);
	  
	  b.push_back(pixel.val[0]);
	  g.push_back(pixel.val[1]);
	  r.push_back(pixel.val[2]);
	}
  
  sort(b.begin(), b.end());
  sort(g.begin(), g.end());
  sort(r.begin(), r.end());
  
  pixel.val[0] = b[b.size() / 2];
  pixel.val[1] = g[g.size() / 2];
  pixel.val[2] = r[r.size() / 2];
  
  return pixel;
}
