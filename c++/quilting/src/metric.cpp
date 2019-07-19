#include <iostream>
#include <utility>
#include <math.h>

#include <opencv2/opencv.hpp>

#define ABS(a) ((a) > 0 ? (a) : -(a))
#define SQR_DIFF(a, b) (ABS(a.val[0] - b.val[0]) + \
						ABS(a.val[1] - b.val[1]) + \
						ABS(a.val[2] - b.val[2]))

using namespace std;

pair<long long, long long> texture_energy(cv::Mat texture,
                                          cv::Mat synth, int size);
pair<int, int> most_similar(cv::Mat texture, cv::Mat  synth,
							int ic, int jc, int size);
int euclidean_norm(cv::Mat img1, int i1, int j1,
                         cv::Mat img2, int i2, int j2,
						 int size);
cv::Scalar closest_pixel(cv::Mat img, int i, int j);
int sqr_diff(CvScalar p1, CvScalar p2);
cv::Mat gradient_image(cv::Mat image);
double *apply_operation(cv::Mat image, double *op, int size);
void histogram(cv::Mat img, double hist[], int channel);
double histogram_ssd(cv::Mat img1, cv::Mat img2, int channel);
void usage();

int main(int argc, char *argv[]){
  cv::Mat texture, synthesized;
  int size;
  pair<long long, long long> energy;
  double r, g, b;

  if(argc < 3){
	usage();
	return 0;
  }

  texture = cv::imread(argv[1]);
  synthesized = cv::imread(argv[2]);

  if(argc == 4)
    size = texture.cols / 2;
  else
    size = texture.cols / 4;

  energy = texture_energy(texture, synthesized, size);

  cout << argv[2] << endl;
  cout << "Neighborhood size: " << size << endl; 
  cout << "Color texture energy: "
	   << energy.first << endl;
  cout << "Gradient texture energy:"
	   << energy.second << endl;

  b = histogram_ssd(texture, synthesized, 0);
  g = histogram_ssd(texture, synthesized, 1);
  r = histogram_ssd(texture, synthesized, 2);

  cout << "Color histogram ssd:"
	   <<  sqrt(r * r + g * g + b * b) << endl;
}

pair<long long, long long> texture_energy(cv::Mat texture,
                                          cv::Mat synth, int size){
  int intensity_norm, gradient_norm;
  pair<int, int> center;
  cv::Mat tex_gradient, synth_gradient;
  pair<long long, long long> energy;

  energy.first = 0;
  energy.second = 0;

  tex_gradient = gradient_image(texture);
  synth_gradient = gradient_image(synth);

  /*cvNamedWindow("Gradient", CV_WINDOW_AUTOSIZE);
  cvShowImage("Gradient", synth_gradient);
  cvNamedWindow("Texture", CV_WINDOW_AUTOSIZE);
  cvShowImage("Texture", synth);

  cvWaitKey(0);*/

  // Find best similarity for each neighborhood in the
  // synthesized image
  for(int i = 0 ; i < synth.cols - size + 1 ; i += size / 4)
    for(int j = 0 ; j < synth.rows - size + 1 ; j += size / 4){
	  //cout << i << " " << j << " " << energy.first
	  //	   << " " << energy.second << endl;

	  // Find most similar neighborhood
	  center = most_similar(texture, synth, i, j, size);
	  // Get norm of diference between the neighborhoods
	  intensity_norm = euclidean_norm(texture, center.first, center.second,
									  synth, i, j, size);
	  gradient_norm = euclidean_norm(tex_gradient, center.first,
									 center.second, synth, i, j, size);

	  energy.first += (intensity_norm * intensity_norm); 
	  energy.second += (gradient_norm * gradient_norm);

	  if(energy.first < 0 || energy.second < 0){
        std::cerr << "error: overflow" << std::endl;
		abort();
	  }
	}

  return energy;
}

pair<int, int> most_similar(cv::Mat texture, cv::Mat synth,
							int ic, int jc, int size){
  pair<int, int> best;
  int min_norm = -1, norm;

  for(int i = 0 ; i < texture.rows - size + 1 ; i++){
    for(int j = 0 ; j < texture.cols - size + 1 ; j++){
	  
	  norm = euclidean_norm(texture, i, j, synth, ic, jc, size);

	  if(min_norm == -1 || min_norm > norm){
		best.first = i;
		best.second = j;
		min_norm = norm;
	  }
	}
  }

  return best;
}

int euclidean_norm(cv::Mat img1, int i1, int j1,
                   cv::Mat img2, int i2, int j2,
				   int size){
  int norm = 0, diff;
  cv::Scalar p1, p2;

  for(int i = 0 ; i < size ; i++)
	for(int j = 0 ; j < size ; j++){
      p1 = img1.at<cv::Vec3b>(i1 + i, j1 + j);
      p2 = img2.at<cv::Vec3b>(i2 + i, j2 + j);

	  diff = sqr_diff(p1, p2);
	  norm += diff;
	}

  return sqrt(norm);
}

// Closest pixel convention
cv::Scalar closest_pixel(cv::Mat img, int i, int j){
  if(i < 0) i = 0;
  if(i >= img.cols) i = img.cols - 1;
  if(j < 0) j = 0;
  if(j >= img.rows) j = img.rows - 1;
  
  return img.at<cv::Vec3b>(i, j);
}

int sqr_diff(CvScalar p1, CvScalar p2){
  int r, g, b;

  r = ABS(p1.val[2] - p2.val[2]);
  g = ABS(p1.val[1] - p2.val[1]);
  b = ABS(p1.val[0] - p2.val[0]);

  return r * r + g * g + b * b; 
}

cv::Mat gradient_image(cv::Mat image){

  cv::Mat gradient;
  double Dx[] = {-1, -2, -1,
				 0, 0, 0,
				 1, 2, 2};
  double Dy[] = {-1, 0, 1,
				 -2, 0, 2,
				 -1, 0, 1};
  double *dx, *dy;
  int width = image.cols, height = image.rows, index, size;
  cv::Scalar pixel;

  size = width * height;

  gradient.create(cv::Size(width, height), image.type());
  dx = apply_operation(image, Dx, 3);
  dy = apply_operation(image, Dy, 3);

  for(int i = 0 ; i < height ; i++)
	for(int j = 0 ; j < width ; j++){
	  index = i * width + j;
	  pixel.val[0] = pixel.val[1] = pixel.val[2] =
		//ABS(dx[index]) + ABS(dy[index]);
		sqrt(dx[index] * dx[index] + dy[index] * dy[index]);

	  cvSet2D(gradient, i, j, pixel);
	}

  delete [] dx;
  delete [] dy;

  return gradient;
}

// Size should be odd
double *apply_operation(cv::Mat image, double *op, int size){
  double value, *result;
  int height, width, intensity, half = size / 2, i_eff, j_eff;
  CvScalar pixel;

  result = new double[image.cols * image.rows];
  height = image.rows;
  width = image.cols;

  for(int i = 0 ; i < height ; i++)
	for(int j = 0 ; j < width ; j++){

	  value = 0;

	  for(int u = 0 ; u < size ; u++)
		for(int v = 0 ; v < size ; v++){

		  i_eff = i + u - half;
		  j_eff = j + v - half;

		  if(i_eff < 0) i_eff = 0; if(i_eff >= height) i_eff = height - 1;
		  if(j_eff < 0) j_eff = 0; if(j_eff >= width) j_eff = width - 1;

          pixel = image.at<cv::Vec3b>(i_eff, j_eff);
		  intensity = (pixel.val[0] + pixel.val[1] + pixel.val[2]) / 3;

		  value += intensity * op[u * size + v];
		}

	  result[i * width + j] = value;
	}

  return result;
}

void histogram(cv::Mat img, double hist[], int channel){
  cv::Scalar pixel;
  
  for(int i = 0 ; i < 256 ; i++)
	hist[i] = 0;

  for(int i = 0 ; i < img.rows ; i++)
    for(int j = 0 ; j < img.cols ; j++){
      pixel = img.at<cv::Vec3b>(i, j);
	  hist[(int)pixel.val[channel]] += 1.0;
	}

  for(int i = 0 ; i < 256 ; i++)
    hist[i] /= (img.cols * img.rows);
}

double histogram_ssd(cv::Mat img1, cv::Mat img2, int channel){
  double hist1[256], hist2[256], ssd;
  
  ssd = 0;

  histogram(img1, hist1, channel);
  histogram(img2, hist2, channel);
  
  for(int i = 0 ; i < 256 ; i++)
	ssd += (hist1[i] * hist1[i] - hist2[i] * hist2[i]);

  return ssd;
}

void usage(){
  cout << "usage: metric input_filename synth_filename" << endl;
}
