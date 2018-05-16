#include "image.h"

//BMAImage make_histogram(const BMAImage& src, double& max_val, const std::string& str)
//{
//	BMAImage hist;
//	int channels[] = { 0 };
//	int histSize[] = { 256 };
//	float hrange[] = { 0, 255 };
//	const float* ranges[] = { hrange };
//	calcHist(&src, 1, channels, Mat(), hist, 1, histSize, ranges);
//
//	minMaxLoc(hist, 0, &max_val, 0, 0);
//	//int scale_width = 1;
//	//int scale_height = 1;
//	//Mat hist_img(histSize[0] * scale_height, histSize[0] * scale_width, CV_8UC1, Scalar(255));
//	//int hpt = static_cast<int>(0.9 * histSize[0] * scale_height);
//	//for (int i = 0; i < histSize[0]; i++)
//	//{
//	//	float bin_val = hist.at<float>(i);
//	//	int intensity = static_cast<int>(bin_val * hpt / max_val);
//	//	line(hist_img, Point(i * scale_width, histSize[0] * scale_height), Point(i * scale_width, histSize[0] * scale_height - intensity), Scalar(0));
//	//}
//	//imshow(str, hist_img);
//	//imwrite("hist_img.bmp", hist_img);
//	//imwrite("hist.bmp", hist);
//	return hist;
//}

void cal_hist(const Image& img, unsigned* hist)
{
	unsigned r, c;
	uchar* p;
	for (r = 0; r < img.height; r++)
	{
		p = img.ptr(r);
		for (c = 0; c < img.width; c++)
		{
			hist[*p]++;
			p++;
		}
	}
}


//void find_lightest(const Image& img, double friction, uchar* pixel_value)
//{
//	unsigned hist[256] = { 0 };
//	cal_hist(img, hist);
//	int thre = int(friction*img.width*img.height);
//	int k=0;
//	for (int i = 255; i >= 0; i--)
//	{
//		k += hist[i];
//		if (k > thre)
//		{
//			*pixel_value = i;
//			return;
//		}
//		
//	}
//}
//void find_core_boundary(const Image& img, double core_friction, bool left, bool right, uchar* left_boundary_value, uchar* right_boundary_value)
//{
//	int max=0;
//	uchar max_pixel;
//	//BMAImage hist = make_histogram(roi, max_val, "直方图");
//	unsigned hist[256] = { 0 };
//	cal_hist(img, hist);
//	for (int i = 0; i < 256; i++)
//	{
//		if (hist[i]>max)
//		{
//			max = hist[i];
//			max_pixel = i;
//		}
//	}
//
//	if (left)
//	{
//		for (int i = max_pixel; i >= 0; i--)
//		{
//			if (hist[i] > core_friction * max)
//			{
//				*left_boundary_value = i;
//			}
//			else break;
//		}
//	}
//	if (right)
//	{
//		for (int i = max_pixel; i < 256; i++)
//		{
//			if (hist[i] > core_friction * max)
//			{
//				*right_boundary_value = i;
//			}
//			else break;
//		}
//	}
//}



