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

using namespace std;
using namespace cv;

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
string resolution, overlay_percent;  //resolution setting and overlay
string s1; //used to store in yaml
string master, output; //name of master image and output image
vector<Mat>::iterator it, it_m; //iterator
int block_resize; //resize factor, how many times enlarge than original one;

//main
int main() {
	
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

	cut_hor = 8;
	cut_ver = 8;

	System::String ^ num_lib = num_lib_img->Text;
	num_library = System::Convert::ToInt16(num_lib);
	
	for (int i = 1; i <= num_library; i++)
	{
		ss = "library/" + to_string(i) + ".jpg";
		src = imread(ss);

		//get mean from BGR
		mean_src = mean(src);

		//mean BGR
		meanB = mean_src[0] / 255;
		meanG = mean_src[1] / 255;
		meanR = mean_src[2] / 255;

		//get hsv from rgb
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
		//cout << color_of_image << endl;


		//BELOW PROCESS 9 BLOCKS
		//  \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
		//cut each library image into 8*8 = 64 pieces
		blocks = CutPics_direct(imread(ss), cut_hor, cut_ver);

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

	System::String ^ in_image = master_image_name->Text;
	MarshalString(in_image, master); //now we read name of master in String
	master = master;

	System::String ^ out_image = target_image->Text;
	MarshalString(out_image, output); //now we read name of target in String
	output = "target/" + output;

	string chosen_block_size;
	MarshalString(block_size_box->Text, chosen_block_size);
	if (chosen_block_size == "small") {
		cut_hor = 60;
		cut_ver = 60;
	}
	else if (chosen_block_size == "median") {
		cut_hor = 50;
		cut_ver = 50;
	}
	else {
		cut_hor = 40;
		cut_ver = 40;
	}

	src = imread(master);
	int adjust_row = (src.rows / 8) * 8;
	int adjust_colomn = (src.cols / 8) * 8;
	resize(src, src, cv::Size(adjust_colomn, adjust_row), 0, 0, INTER_NEAREST);

	vector<Mat> blocks_of_blocks_master;

	master_blocks = CutPics_direct(imread(master), cut_hor, cut_ver);

	cv::FileStorage fs11("master.yaml", cv::FileStorage::WRITE);

	/* For each bloc, do the matching process
	*
	* very long due to the repeated operation
	*/
	
	for (int i = 1; i <= (cut_hor * cut_ver); i++)
	{

		mean_src = mean(master_blocks[i - 1]);

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

		blocks_of_blocks_master = CutPics_direct(master_blocks[i - 1], 8, 8);
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

				if ((dis_finger < max_dis) && (check_repeat(black[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(grey[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = grey[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(black[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(grey[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(white[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = white[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(grey[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(white[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = white[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(red[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(orange[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(purple[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = purple[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(red[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(orange[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(yellow[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = yellow[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(green[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(orange[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(yellow[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = yellow[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(green[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(cyan[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(yellow[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = yellow[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(green[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(cyan[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(blue[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = blue[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(purple[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(cyan[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(blue[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = blue[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
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

				if ((dis_finger < max_dis) && (check_repeat(purple[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(red[e], i) == 1)) {
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

				if ((dis_finger < max_dis) && (check_repeat(blue[e], i) == 1)) {
					top_six_dis[tmp_max_index] = dis_finger;
					top_six_index[tmp_max_index] = blue[e];
					max_dis = top_six_dis[max6_index(top_six_dis)];
				}//if
			}//for e

			 //choose
			if (check_repeat_bigger(min_index, i) == 1) {
				min_dis_pic[i] = min_index;
			}
			else {
				min_dis_pic[i] = top_six_index[rand() % 6];
			}
		}

	} //for each block of master image

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

	//the final resolution
	MarshalString(resolution_box->Text, resolution);
	if (resolution == "low") {
		block_resize = 2;
	}
	else if (resolution == "median") {
		block_resize = 5;
	}
	else if (resolution == "high") {
		block_resize = 8;
	}

	//set the final block size to decide the final resolution 
	int block_height = (src.rows * block_resize) / cut_ver;
	int block_width = (src.cols * block_resize) / cut_hor;
	//create the backgroud of the result
	Mat3b target(block_height * cut_ver, block_width * cut_hor);
	Mat3b resize_master(block_height * cut_ver, block_width * cut_hor);

	it = master_blocks.begin();
	for (int i = 1; i <= (cut_hor * cut_ver); i++)
	{
		//timing
		Mat resize_ori, resize_dst, tmp_dst, final_image;
		resize_ori = imread("library/" + to_string(min_dis_pic[i]) + ".jpg");

		resize(resize_ori, resize_dst, cv::Size(block_width, block_height), 0, 0, CV_INTER_LINEAR);

		//linear blend the original blocks
		tmp = *it;
		resize(tmp, tmp_dst, cv::Size(block_width, block_height), 0, 0, CV_INTER_LINEAR);

		final_image = resize_dst;
		int cols = ((i - 1) % cut_hor) * block_width;
		int rows = ((i - 1) / cut_hor) * block_height;
		final_image.copyTo(target(Rect(cols, rows, block_width, block_height)));
		tmp_dst.copyTo(resize_master(Rect(cols, rows, block_width, block_height)));

		it++;
	}//for each block

	MarshalString(overlay_box->Text, overlay_percent);
		double alpha_1;
		if (overlay_percent == "10%") {
			alpha_1 = 0.9;
		}
		else if (overlay_percent == "20%") {
			alpha_1 = 0.8;
		}
		else if (overlay_percent == "30%") {
			alpha_1 = 0.7;
		}
		else if (overlay_percent == "40%") {
			alpha_1 = 0.6;
		}
		else if (overlay_percent == "50%") {
			alpha_1 = 0.5;
		}
		else {
			alpha_1 = 1.0;
		}
		addWeighted(target, alpha_1, resize_master, (1 - alpha_1), 0.0, target);
	
	imwrite(output, target);
	//master_image_show->ImageLocation = gcnew System::String(output.c_str());
}

void MarshalString ( System::String ^ s, string& os ) {  
	using namespace Runtime::InteropServices;  
	const char* chars =   
	   (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();  
	os = chars;  
	Marshal::FreeHGlobal(IntPtr((void*)chars));  
 } 

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

int check_repeat(int img_index, int location)
{
	//left
	if ((location - 1) > 0) {
		if (min_dis_pic[location - 1] == img_index) {
			return 0;
		}
	}

	//left left
	if ((location - 2) > 0) {
		if (min_dis_pic[location - 2] == img_index) {
			return 0;
		}
	}

	//up right
	if ((location - cut_hor + 1) > 0) {
		if (min_dis_pic[location - cut_hor + 1] == img_index) {
			return 0;
		}
	}

	//up
	if ((location - cut_hor) > 0) {
		if (min_dis_pic[location - cut_hor] == img_index) {
			return 0;
		}
	}

	//up left
	if ((location - cut_hor - 1) > 0) {
		if (min_dis_pic[location - cut_hor - 1] == img_index) {
			return 0;
		}
	}

	//up left left
	if ((location - cut_hor - 2) > 0) {
		if (min_dis_pic[location - cut_hor - 2] == img_index) {
			return 0;
		}
	}

	//up up right
	if ((location - 2 * cut_hor + 1) > 0) {
		if (min_dis_pic[location - 2 * cut_hor + 1] == img_index) {
			return 0;
		}
	}

	//up up left left
	if ((location - 2 * cut_hor - 2) > 0) {
		if (min_dis_pic[location - 2 * cut_hor - 2] == img_index) {
			return 0;
		}
	}

	//up up
	if ((location - 2 * cut_hor) > 0) {
		if (min_dis_pic[location - 2 * cut_hor] == img_index) {
			return 0;
		}
	}

	//up up left
	if ((location - 2 * cut_hor - 1) > 0) {
		if (min_dis_pic[location - 2 * cut_hor - 1] == img_index) {
			return 0;
		}
	}

	return 1;
}

int check_repeat_bigger(int img_index, int location) {
	if (check_repeat(img_index, location)) {
		//left left left
		if ((location - 3) > 0) {
			if (min_dis_pic[location - 3] == img_index) {
				return 0;
			}
		}
		else if ((location - cut_hor - 3) > 0) {
			// up l l l 
			if (min_dis_pic[location - cut_hor - 3] == img_index) {
				return 0;
			}
		} if ((location - 2 * cut_hor - 2) > 0) {
			//up up l l l
			if (min_dis_pic[location - 2 * cut_hor - 3] == img_index) {
				return 0;
			}
		}

		return 1;
	}
	else {
		return 0;
	}

	return 1;
}
