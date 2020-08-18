// imagemosaicunequal.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

int cut_hor; //number of blocks horizantally
int cut_ver; //number of blocks vertically

string ss; //name of each image
Mat src; //image (master)
Scalar mean_src; //rgb of average color of whole image
double meanB, meanG, meanR; //r,g,b of above
double v_d, s_d, h_d; //hsv of the rgb in double
int h, s, v; //hsv in integer of above
string color_of_image; //color.yaml
vector<Mat> blocks; //64 blocks of each image
vector<Mat> master_blocks;
int n = 0; //counter
Mat tmp; //tmp - refer to each block when processing
Vec3d lab_block; //Lab of each block 
int L_color_blocks[64]; //average L in int of each block
int a_color_blocks[64]; //average a in int of each block
int b_color_blocks[64]; //average b in int of each block
int L_compare[64]; //
int a_compare[64]; //
int b_compare[64]; //
int num_of_each_color[10] = { 0 }; //num of image in each color
int black[100], grey[100], white[100], red[100], orange[100], yellow[100];
int green[100], cyan[100], blue[100], purple[100]; //index of image in each color
double dis_finger = 0.0; //distance of two fingerprint
double min_dis = 999.0; //init value
int min_dis_pic[10000] = { 0 }; //store the image index chosen of each block
int k; //index for fingerprint operation
int num_library; //the total number of liabrary images
char overlay, overlay_percent, resolution; //image overlay and resolution setting
string s1; //used to store in yaml
string master, output; //name of master image and output image
vector<Mat>::iterator it, it_m; //iterator
int block_resize; //resize factor, how many times enlarge than original one;

//extract object variables
Mat image_cut;
Mat mask;
Mat bgdModel, fgdModel;
int line0, line1, line2, line3;
uchar rectState, mouse_flag;
Rect rect_cut;
enum { NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
enum { LS_LEFT = 0, LS_RIGHT = 1, LS_NONE = 2 };
const Scalar GREEN = Scalar(0, 255, 0);
const Scalar RED = Scalar(0, 0, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;

vector<Mat> mask_blocks; //blocks of mask to make comparation with master image block
Scalar object_portion; //the portion of object pixels
int how_cut[4][4]; //16 integers to store the distribution of the smaller blocks inside larger blocks
vector<Mat> large_block; //the largest block which is combined with smaller blocks
vector<Mat> cut_level1, cut_level2;
vector<Mat> cut_lvl1, cut_lvl2, cut_lvl3;
vector<Mat> result_block;
int copy_num = 0;
int block_block_index[64] = {0};
Mat copy_tmp, copy_tmp_dst;
int size1_h, size1_w;
				 
/*
OpenCV RGB to HSV color range
Orange  0-22
Yellow 22- 38
Green 38-75
Blue 75-130
Violet 130-160
Red 160-179
*/

string num2str(int i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}

double max3(double a, double b, double c)
{
	return  max(max(a, b), c);
}

int max6_index(double a[6])
{
	double tmp_max = max(max3(a[0], a[1], a[2]), max3(a[3], a[4], a[5]));
	if (tmp_max == a[0]) {
		return 0;
	}
	else if (tmp_max == a[1]) {
		return 1;
	}
	else if (tmp_max == a[2]) {
		return 2;
	}
	else if (tmp_max == a[3]) {
		return 3;
	}
	else if (tmp_max == a[4]) {
		return 4;
	}
	else if (tmp_max == a[5]) {
		return 5;
	}

	return 0;
}

double min3(double a, double b, double c)
{
	return  min(min(a, b), c);
}

double get_v(double b, double g, double r)
{
	return max3(b, g, r);
}

double get_s(double v, double b, double g, double r)
{
	if (v == 0.0)
	{
		return 0.0;
	}
	else
	{
		return ((v - min3(b, g, r)) / v);
	}
}

double get_h(double v, double b, double g, double r)
{
	double h;

	if (v == min3(r, g, b)) {
		h = 0.0;
	}
	else if ((v == r) && (g >= b)) {
		h = (60.0 * (g - b)) / (v - min3(r, g, b));
	}
	else if ((v == r) && (g < b)) {
		h = (60.0 * (g - b)) / (v - min3(r, g, b)) + 360.0;
	}
	else if (v == g) {
		h = (60.0 * (b - r)) / (v - min3(r, g, b)) + 120.0;
	}
	else if (v == b) {
		h = (60.0 * (r - g)) / (v - min3(r, g, b)) + 240.0;
	}

	return h;
}

Vec3d get_Lab(double r, double g, double b)
{
	double var_R = (r / 255.0);
	double var_G = (g / 255.0);
	double var_B = (b / 255.0);

	if (var_R > 0.04045) {
		var_R = pow(((var_R + 0.055) / 1.055), 2.4);
	}
	else {
		var_R = var_R / 12.92;
	}

	if (var_G > 0.04045) {
		var_G = pow(((var_G + 0.055) / 1.055), 2.4);
	}
	else {
		var_G = var_G / 12.92;
	}

	if (var_B > 0.04045) {
		var_B = pow(((var_B + 0.055) / 1.055), 2.4);
	}
	else {
		var_B = var_B / 12.92;
	}

	var_R = var_R * 100.0;
	var_G = var_G * 100.0;
	var_B = var_B * 100.0;

	double X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
	double Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
	double Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

	//95.047 100.00 108.883
	double var_X = X / 95.047;
	double var_Y = Y / 100.00;
	double var_Z = Z / 108.883;

	if (var_X > 0.008856) {
		var_X = pow(var_X, (1.0 / 3.0));
	}
	else {
		var_X = (7.787 * var_X) + (16.0 / 116.0);
	}

	if (var_Y > 0.008856) {
		var_Y = pow(var_Y, (1.0 / 3.0));
	}
	else {
		var_Y = (7.787 * var_Y) + (16.0 / 116.0);
	}

	if (var_Z > 0.008856) {
		var_Z = pow(var_Z, (1.0 / 3.0));
	}
	else {
		var_Z = (7.787 * var_Z) + (16.0 / 116.0);
	}

	double get_L = (116 * var_Y) - 16.0;
	double get_a = 500.0 * (var_X - var_Y);
	double get_b = 200.0 * (var_Y - var_Z);

	Vec3d lab;
	lab[0] = get_L;
	lab[1] = get_a;
	lab[2] = get_b;

	return lab;
}

vector<Mat> CutPics_direct(Mat srcImg, int c_h, int c_v)
{
	vector<Mat> cell;

	int height = srcImg.rows;
	int width = srcImg.cols;

	int cell_h = (int)(height / c_v);
	int cell_w = (int)(width / c_h);
	int cell_lower_h = height - (c_v - 1) * cell_h;
	int cell_right_w = width - (c_h - 1) * cell_w;

	for (int i = 0; i < (c_v - 1); i++)
		for (int j = 0; j < c_h; j++)
		{
			if (j < (c_h - 1))
			{
				Rect rect(j * cell_w, i * cell_h, cell_w, cell_h);
				cell.push_back(srcImg(rect));

			} else{
				Rect rect((c_h - 1) * cell_w, i * cell_h, cell_right_w, cell_h);
				cell.push_back(srcImg(rect));
			}
		}

	for (int i = 0; i < c_h; i++)
	{
		if (i < (c_h - 1))
		{
			Rect rect(i * cell_w, (c_v - 1) * cell_h, cell_w, cell_lower_h);
			cell.push_back(srcImg(rect));
		} else {
			Rect rect((c_h - 1) * cell_w, (c_v - 1) * cell_h, cell_right_w, cell_lower_h);
			cell.push_back(srcImg(rect));
		}
	}

	return cell;
}

string color_image(int h, int s, int v)
{
	if ((h >= 0) && (h <= 360) && (s >= 0) && (s <= 100) && (v >= 0) && (v <= 18))
	{
		return "black.yaml";
	}//black

	if ((h >= 0) && (h <= 360) && (s >= 0) && (s <= 17) && (v >= 18) && (v <= 86))
	{
		return "grey.yaml";
	}//grey

	if ((h >= 0) && (h <= 360) && (s >= 0) && (s <= 17) && (v >= 80) && (v <= 100))
	{
		return "white.yaml";
	}//white

	if ((h >= 0) && (h <= 20) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "red.yaml";
	}//red
	if ((h >= 311) && (h <= 360) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "red.yaml";
	}//red

	if ((h >= 21) && (h <= 50) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "orange.yaml";
	}//orange

	if ((h >= 51) && (h <= 68) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "yellow.yaml";
	}//yellow

	if ((h >= 69) && (h <= 154) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "green.yaml";
	}//green

	if ((h >= 155) && (h <= 198) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "cyan.yaml";
	}//cyan

	if ((h >= 199) && (h <= 248) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "blue.yaml";
	}//blue

	if ((h >= 249) && (h <= 310) && (s >= 17) && (s <= 100) && (v >= 18) && (v <= 100))
	{
		return "purple.yaml";
	}//purple

	return "blank";
}

double color_dis(Vec3i lab1, Vec3i lab2)
{
	double L1 = (double)(lab1[0]);
	double L2 = (double)(lab2[0]);
	double a1 = (double)(lab1[1]);
	double a2 = (double)(lab2[1]);
	double b1 = (double)(lab1[2]);
	double b2 = (double)(lab2[2]);

	double c1 = sqrt((a1 * a1) + (b1 * b1));
	double c2 = sqrt((a2 * a2) + (b2 * b2));
	double delta_L = L1 - L2;
	double delta_C = c1 - c2;
	double delta_a = a1 - a2;
	double delta_b = b1 - b2;
	double delta_H = (delta_a * delta_a) + (delta_b * delta_b) - (delta_C * delta_C);
	double sc = 1.0 + 0.045 * c1;
	double sh = 1.0 + 0.015 * c1;

	double first = (delta_L);
	double second = (delta_C) / (sc);
	double third = (delta_H) / (sh * sh);
	double delta = sqrt((first * first) + (second * second) + (third));

	return delta;
}

double finger_dis(int L1[64], int a1[64], int b1[64], int L2[64], int a2[64], int b2[64])
{
	Vec3i temp1, temp2;
	int p;
	double dis_finger = 0.0;
	for (p = 0; p < 64; p++)
	{
		temp1[0] = L1[p];
		temp1[1] = a1[p];
		temp1[2] = b1[p];
		temp2[0] = L2[p];
		temp2[1] = a2[p];
		temp2[2] = b2[p];

		dis_finger += color_dis(temp1, temp2);
	}
	return (dis_finger / 64.0);
}


int check_repeat_un(int img_index, int list[64])
{
	int count = 0;
	for (int tmp_index = 0; tmp_index < 64; tmp_index++){
		if (list[tmp_index] == img_index){
			count++;
		}
	}
	if (count < 5){
		return 1;
	} else {
		return 0;
	}
}

int check_repeat_un_strict(int img_index, int list[64])
{
	int count = 0;
	for (int tmp_index = 0; tmp_index < 64; tmp_index++){
		if (list[tmp_index] == img_index){
			count++;
		}
	}
	if (count < 2){
		return 1;
	} else {
		return 0;
	}
}

void setRectInMask() {
	rect_cut.x = max(0, rect_cut.x);
	rect_cut.y = max(0, rect_cut.y);
	rect_cut.width = min(rect_cut.width, image_cut.cols - rect_cut.x);
	rect_cut.height = min(rect_cut.height, image_cut.rows - rect_cut.y);

}

static void getBinMask(const Mat& comMask, Mat& binMask) 
{
	binMask.create(comMask.size(), CV_8UC1);
	binMask = comMask & 1;
}

void on_mouse(int event, int x, int y, int flags, void*)
{
	Point pt1, pt2;
	switch (event) {
	case CV_EVENT_LBUTTONDOWN:
		mouse_flag = LS_LEFT;
		if (rectState == NOT_SET) {
			rectState = IN_PROCESS;
			rect_cut = Rect(x, y, 1, 1);
		}
		break;
	case CV_EVENT_LBUTTONUP:
		if (rectState == IN_PROCESS) {
			pt1.x = rect_cut.x; pt1.y = rect_cut.y;
			pt2.x = x; pt2.y = y;

			rect_cut = Rect(pt1, pt2);
			rectState = SET;
			(mask(rect_cut)).setTo(Scalar(GC_PR_FGD));
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		mouse_flag = LS_RIGHT;
		line0 = x;
		line1 = y;
		break;
	case CV_EVENT_RBUTTONUP:
		mouse_flag = LS_NONE;
		line0 = 0; line1 = 0; line2 = 0; line3 = 0;
		
		imwrite("tmp/tmp.jpg", image_cut);
		break;
	case CV_EVENT_MOUSEMOVE:
		if (mouse_flag == LS_LEFT) {
			if (rectState == IN_PROCESS) {
				pt1.x = rect_cut.x; pt1.y = rect_cut.y;
				pt2.x = x; pt2.y = y;

				rect_cut = Rect(pt1, pt2);
				pt1.x = rect_cut.x; pt1.y = rect_cut.y;
				pt2.x = rect_cut.x + rect_cut.width; pt2.y = rect_cut.y + rect_cut.height;

				image_cut = imread("tmp/tmp.jpg", 1);
				rectangle(image_cut, pt1, pt2, GREEN, 2);
				imshow("show", image_cut);
			}
		}
		else if (mouse_flag == LS_RIGHT) {
			IplImage ori_img = image_cut;
			IplImage mask_img = mask;
			line2 = x;
			line3 = y;
			if ((flags & BGD_KEY) != 0) {
				pt1.x = line0; pt1.y = line1;
				pt2.x = line2; pt2.y = line3;
				cvLine(&ori_img, pt1, pt2, RED, 3);
				cvLine(&mask_img, pt1, pt2, cvScalar(0, 0, 0), 3);
			}
			else if ((flags & FGD_KEY) != 0) {
				pt1.x = line0; pt1.y = line1;
				pt2.x = line2; pt2.y = line3;
				cvLine(&ori_img, pt1, pt2, BLUE, 3);
				cvLine(&mask_img, pt1, pt2, cvScalar(1, 0, 0), 3);
			}
			line0 = x;
			line1 = y;
			imshow("show", image_cut);
		}
		break;
	}
}

int main()
{
	/*********************/
	cv::FileStorage fs1("black.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs2("grey.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs3("white.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs4("red.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs5("orange.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs6("yellow.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs7("green.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs8("cyan.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs9("blue.yaml", cv::FileStorage::WRITE);
	cv::FileStorage fs10("purple.yaml", cv::FileStorage::WRITE);

	/*USE PIC_ID TO STORE IN DATABASE
	*LIBRARY IMAGES
	*/
	cout << "Hello World! Welcome to TerryMosaics." << endl;
	cout << "*************************************" << endl;
	cout << "*                                   *" << endl;
	cout << "*           TerryMosaics            *" << endl;
	cout << "*                                   *" << endl;
	cout << "*************************************" << endl << endl;

	cout << "How many library images: ";
	cin >> num_library;

	cout << endl << "Initializing library images:" << endl;
	cout << "--------------------" << endl;

	double progress = 0.0;
	for (int i = 1; i <= num_library; i++)
	{
		progress += 1.0;
		if ((progress / num_library) >= 0.05) {
			progress = 0.0;
			cout << "*";
		}

		ss = "library/" + to_string(i) + ".jpg";
		src = imread(ss);

		//get mean from BGR
		mean_src = mean(src);
		//mean BGR & get hsv from rgb
		meanB = mean_src[0] / 255;
		meanG = mean_src[1] / 255;
		meanR = mean_src[2] / 255;
		v_d = get_v(meanB, meanG, meanR);
		s_d = get_s(v_d, meanB, meanG, meanR);
		h_d = get_h(v_d, meanB, meanG, meanR);

		//around to int
		//USE THESE H S V TO STORE IN THE DATABASE
		//use them to sort the image to which color bin
		v = (int)(v_d * 100 + 0.5);
		s = (int)(s_d * 100 + 0.5);
		h = (int)(h_d + 0.5);

		//sort image into color bin
		color_of_image = color_image(h, s, v);

		//cut each library image into 8*8 = 64 pieces
		blocks = CutPics_direct(imread(ss), 8, 8);

		n = 0;

		/* convert mean colour of each
		* block to Lab mode
		*/
		it = blocks.begin();
		while (it != blocks.end())
		{
			tmp = *it;
			Scalar mean_blocks = mean(tmp);

			lab_block = get_Lab(mean_blocks[2], mean_blocks[1], mean_blocks[0]);
			L_color_blocks[n] = (int)(lab_block[0] + 0.5);
			a_color_blocks[n] = (int)(lab_block[1] + 0.5);
			b_color_blocks[n] = (int)(lab_block[2] + 0.5);

			n++;
			it++;
		}//while

		//write to files
		s1 = "pic" + to_string(i);

		if (color_of_image == "black.yaml")
		{
			black[num_of_each_color[0]] = i;
			num_of_each_color[0]++;

			fs1 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs1 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs1 << "]";
		}
		else if (color_of_image == "grey.yaml")
		{
			grey[num_of_each_color[1]] = i;
			num_of_each_color[1]++;

			fs2 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs2 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs2 << "]";
		}
		else if (color_of_image == "white.yaml")
		{
			white[num_of_each_color[2]] = i;
			num_of_each_color[2]++;

			fs3 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs3 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs3 << "]";
		}
		else if (color_of_image == "red.yaml")
		{
			red[num_of_each_color[3]] = i;
			num_of_each_color[3]++;

			fs4 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs4 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs4 << "]";
		}
		else if (color_of_image == "orange.yaml")
		{
			orange[num_of_each_color[4]] = i;
			num_of_each_color[4]++;

			fs5 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs5 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs5 << "]";
		}
		else if (color_of_image == "yellow.yaml")
		{
			yellow[num_of_each_color[5]] = i;
			num_of_each_color[5]++;

			fs6 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs6 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs6 << "]";
		}
		else if (color_of_image == "green.yaml")
		{
			green[num_of_each_color[6]] = i;
			num_of_each_color[6]++;

			fs7 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs7 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs7 << "]";
		}
		else if (color_of_image == "cyan.yaml")
		{
			cyan[num_of_each_color[7]] = i;
			num_of_each_color[7]++;

			fs8 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs8 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs8 << "]";
		}
		else if (color_of_image == "blue.yaml")
		{
			blue[num_of_each_color[8]] = i;
			num_of_each_color[8]++;

			fs9 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs9 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs9 << "]";
		}
		else if (color_of_image == "purple.yaml")
		{
			purple[num_of_each_color[9]] = i;
			num_of_each_color[9]++;

			fs10 << s1 << "[";
			for (k = 0; k < 64; k++) {
				fs10 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs10 << "]";
		}

	}//for each library images

	cout << "100%" << endl << "--------------------" << endl << endl;

	fs1.open("black.yaml", FileStorage::READ);
	fs2.open("grey.yaml", FileStorage::READ);
	fs3.open("white.yaml", FileStorage::READ);
	fs4.open("red.yaml", FileStorage::READ);
	fs5.open("orange.yaml", FileStorage::READ);
	fs6.open("yellow.yaml", FileStorage::READ);
	fs7.open("green.yaml", FileStorage::READ);
	fs8.open("cyan.yaml", FileStorage::READ);
	fs9.open("blue.yaml", FileStorage::READ);
	fs10.open("purple.yaml", FileStorage::READ);

	/* Operation to master image
	* mean colour
	* 8 x 8 fingerprint store in master.yaml
	*/
	cout << "The name of master image: ";
	cin >> master;

	//separate object from background
	Mat res;
	Mat binMask;
	master = "master/" + master;

	image_cut = imread(master,1);
	imshow("show", image_cut);
	imwrite("tmp/tmp.jpg", image_cut);
	mask.create(image_cut.size(), CV_8UC1);
	rectState = NOT_SET;
	mask.setTo(GC_BGD);

	setMouseCallback("show", on_mouse, 0);

	while (1) {
		int c = waitKey(0);
		if (c == 's') {
			image_cut = imread(master,1);
			grabCut(image_cut, mask, rect_cut, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK);
			getBinMask(mask, binMask);

			image_cut.copyTo(res, binMask);
			imwrite("tmp/result.jpg", res);
		}
		break;
	}
	destroyWindow("show");

	cout << "The name of output image: ";
	cin >> output;

	cout << endl;
	cout << "The width/height of the image are " << image_cut.cols << " & " << image_cut.rows <<endl << endl;
	cout << "The number of images on row (horizon): ";
	cin >> cut_hor;
	cout << "The number of images on column (verticle): ";
	cin >> cut_ver;
	cout << endl;

	src = res;
	int adjust_row = (src.rows / 8) * 8;
	int adjust_colomn = (src.cols / 8) * 8;
	resize(src, src, Size(adjust_colomn, adjust_row), 0, 0, INTER_NEAREST);

	vector<Mat> blocks_of_blocks_master;

	src = imread(master);
	master_blocks = CutPics_direct(src, cut_hor, cut_ver);
	mask_blocks = CutPics_direct(binMask, cut_hor, cut_ver);

	cv::FileStorage fs11("master.yaml", cv::FileStorage::WRITE);

	progress = 0.0;
	cout << endl << "Cutting, Matching and Copying:" << endl;
	cout << "--------------------" << endl;

	/* For each bloc, do the matching process
	*
	* very long due to the repeated operation
	*/
	for (int i = 1; i <= (cut_hor * cut_ver); i++)
	{
		progress += 1.0;
		if ((progress / (cut_hor * cut_ver)) >= 0.05) {
			progress = 0.0;
			cout << "*";
		}

		large_block.clear();
		cut_level1.clear();
		cut_level2.clear();
		cut_lvl1.clear();
		cut_lvl2.clear();
		cut_lvl3.clear();

		object_portion = mean(mask_blocks[i - 1]);

		int random_cut = rand() % 2;
		//the largest block is all background
		if ((object_portion.val[0] <= 0.01) && (random_cut != 0)){
			large_block.push_back(master_blocks[i-1]);
			how_cut[0][0] = -1; how_cut[0][1] = -1; how_cut[0][2] = -1; how_cut[0][3] = -1;
			how_cut[1][0] = -1; how_cut[1][1] = -1; how_cut[1][2] = -1; how_cut[1][3] = -1;
			how_cut[2][0] = -1; how_cut[2][1] = -1; how_cut[2][2] = -1; how_cut[2][3] = -1;
			how_cut[3][0] = -1; how_cut[3][1] = -1; how_cut[3][2] = -1; how_cut[3][3] = -1;
		}

		//the largest block contains object
		if ((object_portion.val[0] > 0.01) || (random_cut == 0)){
			//mask -> 2*2
			cut_level1 = CutPics_direct(mask_blocks[i - 1], 2, 2);
			//master -> 2*2
			cut_lvl1 = CutPics_direct(master_blocks[i - 1], 2, 2);

			for (int lvl1 = 0; lvl1 < 4; lvl1++){
				
				object_portion = mean(cut_level1[lvl1]);
				//cout << object_portion.val[0] << endl;

				//if sencond largest block is all background
				if (object_portion.val[0] <= 0.01){
					how_cut[lvl1][0] = 0; how_cut[lvl1][1] = 0; how_cut[lvl1][2] = 0; how_cut[lvl1][3] = 0;
					large_block.push_back(cut_lvl1[lvl1]);
				}

				//the second largest block contains object
				if (object_portion.val[0] > 0.01){
					//mask -> 4*4
					cut_level2 = CutPics_direct(cut_level1[lvl1], 2, 2);
					//master -> 4*4
					cut_lvl2 = CutPics_direct(cut_lvl1[lvl1], 2, 2);

					for (int lvl2 = 0; lvl2 < 4; lvl2++){
						object_portion = mean(cut_level2[lvl2]);
						//cout << object_portion.val[0] << endl;
						
						//if third largest block is all background
						if (object_portion.val[0] <= 0.01){
							how_cut[lvl1][lvl2] = 1;
							large_block.push_back(cut_lvl2[lvl2]);
						}

						//the third largest block contains object
						if (object_portion.val[0] > 0.01){
							how_cut[lvl1][lvl2] = 4;
							cut_lvl3 = CutPics_direct(cut_lvl2[lvl2], 2, 2);
							large_block.push_back(cut_lvl3[0]);
							large_block.push_back(cut_lvl3[1]);
							large_block.push_back(cut_lvl3[2]);
							large_block.push_back(cut_lvl3[3]);
						}
					}// for lvl2
				}// if second larges block has object
			}// for lvl1
		}// if larges block has object
		
		for (int b_b = 0; b_b < large_block.size(); b_b++){
			mean_src = mean(large_block[b_b]);
			//cout << object_portion.val[0] << endl;
			meanB = mean_src[0] / 255;
			meanG = mean_src[1] / 255;
			meanR = mean_src[2] / 255;

			v_d = get_v(meanB, meanG, meanR);
			s_d = get_s(v_d, meanB, meanG, meanR);
			h_d = get_h(v_d, meanB, meanG, meanR);

			v = (int)(v_d * 100 + 0.5);
			s = (int)(s_d * 100 + 0.5);
			h = (int)(h_d + 0.5);

			color_of_image = color_image(h, s, v);
			//cout << color_of_image << " " << h << " " << s << " " << v << endl;

			//cut_hor = 8;
			//cut_ver = 8;

			blocks_of_blocks_master = CutPics_direct(large_block[b_b], 8, 8);
			it_m = blocks_of_blocks_master.begin();

			n = 0;
			while (it_m != blocks_of_blocks_master.end())
			{
				tmp = *it_m;
				Scalar mean_blocks = mean(tmp);

				lab_block = get_Lab(mean_blocks[2], mean_blocks[1], mean_blocks[0]);
				L_color_blocks[n] = (int)(lab_block[0] + 0.5);
				a_color_blocks[n] = (int)(lab_block[1] + 0.5);
				b_color_blocks[n] = (int)(lab_block[2] + 0.5);

				n++;
				it_m++;
			}

			s1 = "m" + to_string(i);

			fs11 << s1 << "[";
			for (k = 0; k < 64; k++)
			{
				fs11 << L_color_blocks[k] << a_color_blocks[k] << b_color_blocks[k];
			}
			fs11 << "]";

			int store_cout = 0;
			//store top 6 unrepeated matching
			double top_six_dis[6] = { 999.0, 999.0, 999.0, 999.0, 999.0, 999.0 };
			int top_six_index[6] = { 1, 1, 1, 1, 1, 1 };
			int max_index, min_index;
			int tmp_max_index;
			min_dis = 999.0;
			double max_dis = 999.0;

			//search color bin
			if (color_of_image == "black.yaml")
			{
				//fs1.open("black.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[0]; e++)
				{
					ss = "pic" + to_string(black[e]);
					FileNode node = fs1[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);

					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = black[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(black[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = black[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//fs2.open("grey.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[1]; e++)
				{
					ss = "pic" + to_string(grey[e]);
					FileNode node = fs2[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);

					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = grey[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(grey[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = grey[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//choose
				// if (check_repeat_bigger(min_index, i) == 1) {
				// 	min_dis_pic[i] = min_index;
				// }
				// else {
				// 	min_dis_pic[i] = top_six_index[rand() % 6];
				// }
				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}		
			}
			else if (color_of_image == "grey.yaml")
			{
				//fs1.open("black.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[0]; e++)
				{
					ss = "pic" + to_string(black[e]);
					FileNode node = fs1[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = black[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(black[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = black[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//fs2.open("grey.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[1]; e++)
				{
					ss = "pic" + to_string(grey[e]);
					FileNode node = fs2[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = grey[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(grey[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = grey[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//fs3.open("white.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[2]; e++)
				{
					ss = "pic" + to_string(white[e]);
					FileNode node = fs3[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = white[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(white[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = white[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "white.yaml")
			{
				//fs2.open("grey.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[1]; e++)
				{
					ss = "pic" + to_string(grey[e]);
					FileNode node = fs2[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = grey[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(grey[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = grey[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//fs3.open("white.yaml", FileStorage::READ);
				for (int e = 0; e < num_of_each_color[2]; e++)
				{
					ss = "pic" + to_string(white[e]);
					FileNode node = fs3[ss];
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = white[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(white[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = white[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "red.yaml")
			{
				//red
				for (int e = 0; e < num_of_each_color[3]; e++) //change
				{
					ss = "pic" + to_string(red[e]); //change
					FileNode node = fs4[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = red[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(red[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = red[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//orange
				for (int e = 0; e < num_of_each_color[4]; e++) //change
				{
					ss = "pic" + to_string(orange[e]); //change
					FileNode node = fs5[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = orange[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(orange[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = orange[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//purple
				for (int e = 0; e < num_of_each_color[9]; e++) //change
				{
					ss = "pic" + to_string(purple[e]); //change
					FileNode node = fs10[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = purple[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(purple[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = purple[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "orange.yaml")
			{
				//red
				for (int e = 0; e < num_of_each_color[3]; e++) //change
				{
					ss = "pic" + to_string(red[e]); //change
					FileNode node = fs4[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = red[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(red[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = red[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//orange
				for (int e = 0; e < num_of_each_color[4]; e++) //change
				{
					ss = "pic" + to_string(orange[e]); //change
					FileNode node = fs5[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = orange[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(orange[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = orange[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//yellow
				for (int e = 0; e < num_of_each_color[5]; e++) //change
				{
					ss = "pic" + to_string(yellow[e]); //change
					FileNode node = fs6[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = yellow[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(yellow[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = yellow[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "yellow.yaml")
			{
				//green
				for (int e = 0; e < num_of_each_color[6]; e++) //change
				{
					ss = "pic" + to_string(green[e]); //change
					FileNode node = fs7[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = green[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(green[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = green[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//orange
				for (int e = 0; e < num_of_each_color[4]; e++) //change
				{
					ss = "pic" + to_string(orange[e]); //change
					FileNode node = fs5[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = orange[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(orange[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = orange[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//yellow
				for (int e = 0; e < num_of_each_color[5]; e++) //change
				{
					ss = "pic" + to_string(yellow[e]); //change
					FileNode node = fs6[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = yellow[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(yellow[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = yellow[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "green.yaml")
			{
				//green
				for (int e = 0; e < num_of_each_color[6]; e++) //change
				{
					ss = "pic" + to_string(green[e]); //change
					FileNode node = fs7[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = green[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(green[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = green[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//cyan
				for (int e = 0; e < num_of_each_color[7]; e++) //change
				{
					ss = "pic" + to_string(cyan[e]); //change
					FileNode node = fs8[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = cyan[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(cyan[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = cyan[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//yellow
				for (int e = 0; e < num_of_each_color[5]; e++) //change
				{
					ss = "pic" + to_string(yellow[e]); //change
					FileNode node = fs6[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = yellow[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(yellow[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = yellow[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "cyan.yaml")
			{
				//green
				for (int e = 0; e < num_of_each_color[6]; e++) //change
				{
					ss = "pic" + to_string(green[e]); //change
					FileNode node = fs7[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = green[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(green[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = green[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//cyan
				for (int e = 0; e < num_of_each_color[7]; e++) //change
				{
					ss = "pic" + to_string(cyan[e]); //change
					FileNode node = fs8[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = cyan[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(cyan[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = cyan[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//blue
				for (int e = 0; e < num_of_each_color[8]; e++) //change
				{
					ss = "pic" + to_string(blue[e]); //change
					FileNode node = fs9[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = blue[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(blue[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = blue[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "blue.yaml")
			{
				//purple
				for (int e = 0; e < num_of_each_color[9]; e++) //change
				{
					ss = "pic" + to_string(purple[e]); //change
					FileNode node = fs10[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = purple[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(purple[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = purple[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//cyan
				for (int e = 0; e < num_of_each_color[7]; e++) //change
				{
					ss = "pic" + to_string(cyan[e]); //change
					FileNode node = fs8[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = cyan[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(cyan[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = cyan[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//blue
				for (int e = 0; e < num_of_each_color[8]; e++) //change
				{
					ss = "pic" + to_string(blue[e]); //change
					FileNode node = fs9[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = blue[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(blue[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = blue[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}
			else if (color_of_image == "purple.yaml")
			{
				//purple
				for (int e = 0; e < num_of_each_color[9]; e++) //change
				{
					ss = "pic" + to_string(purple[e]); //change
					FileNode node = fs10[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = purple[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(purple[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = purple[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//red
				for (int e = 0; e < num_of_each_color[3]; e++) //change
				{
					ss = "pic" + to_string(red[e]); //change
					FileNode node = fs4[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = red[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(red[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = red[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				//blue
				for (int e = 0; e < num_of_each_color[8]; e++) //change
				{
					ss = "pic" + to_string(blue[e]); //change
					FileNode node = fs9[ss]; //change
					FileNodeIterator it_read = node.begin();
					store_cout = 0;
					while (it_read != node.end())
					{
						if (store_cout % 3 == 0) {
							L_compare[store_cout / 3] = (int)*it_read;
						}
						else if (store_cout % 3 == 1) {
							a_compare[(store_cout / 3)] = (int)*it_read;
						}
						else if (store_cout % 3 == 2) {
							b_compare[(store_cout / 3)] = (int)*it_read;
						}
						it_read++;
						store_cout++;
					}//while

					//compute color dis of blocks
					dis_finger = finger_dis(L_color_blocks, a_color_blocks, b_color_blocks, L_compare, a_compare, b_compare);
					if (dis_finger <= min_dis) {
						min_dis = dis_finger;
						min_index = blue[e];
					}

					tmp_max_index = max6_index(top_six_dis);
					max_dis = top_six_dis[tmp_max_index];
					max_index = top_six_index[tmp_max_index];

					if ((dis_finger < max_dis) && (check_repeat_un(blue[e], block_block_index) == 1)) {
						top_six_dis[tmp_max_index] = dis_finger;
						top_six_index[tmp_max_index] = blue[e];
						max_dis = top_six_dis[max6_index(top_six_dis)];
					}//if
				}//for e

				if  (large_block.size() == 1) {
					block_block_index[b_b] = top_six_index[rand() % 6];
				} else {
					if (check_repeat_un_strict(min_index, block_block_index) == 1){
						block_block_index[b_b] = min_index;
					} else {
						block_block_index[b_b] = top_six_index[rand() % 6];
					}
				}
			}

		}

		//Now, we have block_block_index stores the matched image index;
		//and howcut[4][4] stores the image distribution;
		if (how_cut[0][0] == -1){
			Mat tmp_result = imread("library/" + to_string(block_block_index[0]) + ".jpg");
			result_block.push_back(tmp_result);

		} else {

			//change the factor here to change the final resolution
			size1_h = master_blocks[i - 1].rows * 8;
			size1_w = master_blocks[i - 1].cols * 8;
			int size2_h = size1_h / 2;
			int size2_w = size1_w / 2;
			int size3_h = size2_h / 2;
			int size3_w = size2_w / 2;
			int size4_h = size3_h / 2;
			int size4_w = size3_w / 2;

			Mat3b block_target(size1_h, size1_w);
			int tmp_col, tmp_row;

			for (int copy_i = 0; copy_i < 4; copy_i++){
				for (int copy_j = 0; copy_j < 4; copy_j++){

					if (how_cut[copy_i][copy_j] == 0){
						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size2_w, size2_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w * (copy_i % 2));
						tmp_row = (size2_h * (copy_i / 2));
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size2_w, size2_h)));
						break;
					}

					if (how_cut[copy_i][copy_j] == 1){
						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size3_w, size3_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w*(copy_i % 2) + size3_w*(copy_j % 2));
						tmp_row = (size2_h*(copy_i / 2) + size3_h*(copy_j / 2));
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size3_w, size3_h)));
					}

					if (how_cut[copy_i][copy_j] == 4){
						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size4_w, size4_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w*(copy_i % 2) + size3_w*(copy_j % 2));
						tmp_row = (size2_h*(copy_i / 2) + size3_h*(copy_j / 2));
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size4_w, size4_h)));

						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size4_w, size4_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w*(copy_i % 2) + size3_w*(copy_j % 2) + size4_w);
						tmp_row = (size2_h*(copy_i / 2) + size3_h*(copy_j / 2));
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size4_w, size4_h)));
					
						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size4_w, size4_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w*(copy_i % 2) + size3_w*(copy_j % 2));
						tmp_row = (size2_h*(copy_i / 2) + size3_h*(copy_j / 2) + size4_h);
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size4_w, size4_h)));
					
						copy_tmp = imread("library/" + to_string(block_block_index[copy_num]) + ".jpg");
						copy_num++;
						resize(copy_tmp, copy_tmp_dst, Size(size4_w, size4_h), 0, 0, CV_INTER_LINEAR);
						tmp_col = (size2_w*(copy_i % 2) + size3_w*(copy_j % 2) + size4_w);
						tmp_row = (size2_h*(copy_i / 2) + size3_h*(copy_j / 2) + size4_h);
						copy_tmp_dst.copyTo(block_target(Rect(tmp_col, tmp_row, size4_w, size4_h)));
					}
				}//for j
			}//for i

			result_block.push_back(block_target);

			copy_num = 0;
		}//else

	} //for each block of master image

	cout << "100%" << endl << "--------------------" << endl;

	fs1.release();
	fs2.release();
	fs3.release();
	fs4.release();
	fs5.release();
	fs6.release();
	fs7.release();
	fs8.release();
	fs9.release();
	fs10.release();
	fs11.release();

	// //create the backgroud of the result
	Mat3b target(size1_h * cut_ver, size1_w * cut_hor);

	progress = 0.0;
	cout << endl << "Generating imagemosaics:" << endl;
	cout << "--------------------" << endl;

	it = result_block.begin();
	for (int i = 1; i <= (cut_hor * cut_ver); i++)
	{
		progress += 1.0;
		if ((progress / (cut_hor * cut_ver)) >= 0.05) {
			progress = 0.0;
			cout << "*";
		}

		Mat resize_ori, resize_dst, tmp_dst, final_image;
		tmp = *it;
	 	resize(tmp, tmp_dst, Size(size1_w, size1_h), 0, 0, CV_INTER_LINEAR);

		int cols = ((i - 1) % cut_hor) * size1_w;
		int rows = ((i - 1) / cut_hor) * size1_h;
		tmp_dst.copyTo(target(Rect(cols, rows, size1_w, size1_h)));

	 	it++;
	}//for each block

	cout << "100%" << endl << "--------------------" << endl;
	output = "target/" + output;
	imwrite(output, target);

	system("pause");
	return 0;
}