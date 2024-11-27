#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <time.h>
using namespace cv;
using namespace std;

// 输入参数
struct Inputparama {
	int thresh = 30;                               // 背景识别阈值，该值越小，则识别非背景区面积越大，需有合适范围，目前为5-60
	int transparency = 255;                        // 背景替换色透明度，255为实，0为透明
	int size = 7;                                  // 非背景区边缘虚化参数，该值越大，则边缘虚化程度越明显
	cv::Point p = cv::Point(1260, 20);                 // 背景色采样点，可通过人机交互获取，也可用默认(0,0)点颜色作为背景色
	cv::Scalar color = cv::Scalar(255, 255, 255);  // 背景色
};

cv::Mat BackgroundSeparation(cv::Mat src, Inputparama input);
void Clear_MicroConnected_Areas(cv::Mat src, cv::Mat& dst, double min_area);

// 计算差值均方根
int geiDiff(uchar b, uchar g, uchar r, uchar tb, uchar tg, uchar tr)
{
	return  int(sqrt(((b - tb) * (b - tb) + (g - tg) * (g - tg) + (r - tr) * (r - tr)) / 3));
}

int main()
{
	cv::Mat src = imread("E://2//IMAGE//left//left30.jpg");
	Inputparama input;
	input.thresh = 100;
	input.transparency = 255;
	input.size = 6;
	input.color = cv::Scalar(255, 255, 255);

	clock_t s, e;
	s = clock();
	cv::Mat result = BackgroundSeparation(src, input);
	e = clock();
	double dif = e - s;
	cout << "time:" << dif << endl;

	imshow("original", src);
	imshow("result", result);
	//imwrite("result2.png", result);
	waitKey(0);
	return 0;
}

// 背景分离
cv::Mat BackgroundSeparation(cv::Mat src, Inputparama input)
{
	cv::Mat bgra, mask;
	// 转化为BGRA格式，带透明度，4通道
	cvtColor(src, bgra, COLOR_BGR2BGRA);
	mask = cv::Mat::zeros(bgra.size(), CV_8UC1);
	int row = src.rows;
	int col = src.cols;

	// 异常数值修正
	input.p.x = max(0, min(col, input.p.x));
	input.p.y = max(0, min(row, input.p.y));
	input.thresh = max(5, min(200, input.thresh));
	input.transparency = max(0, min(255, input.transparency));
	input.size = max(0, min(30, input.size));

	// 确定背景色
	uchar ref_b = src.at<Vec3b>(input.p.y, input.p.x)[0];
	uchar ref_g = src.at<Vec3b>(input.p.y, input.p.x)[1];
	uchar ref_r = src.at<Vec3b>(input.p.y, input.p.x)[2];

	// 计算蒙版区域（掩膜）
	for (int i = 0; i < row; ++i)
	{
		uchar* m = mask.ptr<uchar>(i);
		uchar* b = src.ptr<uchar>(i);
		for (int j = 0; j < col; ++j)
		{
			if ((geiDiff(b[3 * j], b[3 * j + 1], b[3 * j + 2], ref_b, ref_g, ref_r)) > input.thresh)
			{
				m[j] = 255;
			}
		}
	}

	cv::Mat tmask = cv::Mat::zeros(row + 50, col + 50, CV_8UC1);
	mask.copyTo(tmask(cv::Range(25, 25 + mask.rows), cv::Range(25, 25 + mask.cols)));

	// 寻找轮廓，作用是填充轮廓内黑洞
	vector<vector<Point>> contour;
	vector<Vec4i> hierarchy;
	// RETR_TREE以网状结构提取所有轮廓，CHAIN_APPROX_NONE获取轮廓的每个像素
	findContours(tmask, contour, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	drawContours(tmask, contour, -1, Scalar(255), FILLED, 16);

	// 黑帽运算获取同背景色类似的区域，识别后填充
	cv::Mat hat;
	cv::Mat element = getStructuringElement(MORPH_ELLIPSE, Size(31, 31));
	cv::morphologyEx(tmask, hat, MORPH_BLACKHAT, element);
	hat.setTo(255, hat > 0);
	cv::Mat hatd;
	Clear_MicroConnected_Areas(hat, hatd, 450);
	tmask = tmask + hatd;
	mask = tmask(cv::Range(25, 25 + mask.rows), cv::Range(25, 25 + mask.cols)).clone();

	// 掩膜滤波，是为了边缘虚化
	//cv::blur(mask, mask, Size(2 * input.size + 1, 2 * input.size + 1));

	// 改色
	for (int i = 0; i < row; ++i)
	{
		uchar* r = bgra.ptr<uchar>(i);
		uchar* m = mask.ptr<uchar>(i);
		for (int j = 0; j < col; ++j)
		{
			// 蒙版为0的区域就是标准背景区
			if (m[j] == 0)
			{
				r[4 * j] = uchar(input.color[0]);
				r[4 * j + 1] = uchar(input.color[1]);
				r[4 * j + 2] = uchar(input.color[2]);
				r[4 * j + 3] = uchar(input.transparency);
			}
			// 不为0且不为255的区域是轮廓区域（边缘区），需要虚化处理
			//else if (m[j] != 255)
			//{
				// 边缘处按比例上色
				//int newb = (r[4 * j] * m[j] * 0.3 + input.color[0] * (255 - m[j]) * 0.7) / ((255 - m[j]) * 0.7 + m[j] * 0.3);
				//int newg = (r[4 * j + 1] * m[j] * 0.3 + input.color[1] * (255 - m[j]) * 0.7) / ((255 - m[j]) * 0.7 + m[j] * 0.3);
				//int newr = (r[4 * j + 2] * m[j] * 0.3 + input.color[2] * (255 - m[j]) * 0.7) / ((255 - m[j]) * 0.7 + m[j] * 0.3);
				//int newt = (r[4 * j + 3] * m[j] * 0.3 + input.transparency * (255 - m[j]) * 0.7) / ((255 - m[j]) * 0.7 + m[j] * 0.3);
				//newb = max(0, min(255, newb));
				//newg = max(0, min(255, newg));
			//	newr = max(0, min(255, newr));
				//newt = max(0, min(255, newt));
				//r[4 * j] = newb;
				//r[4 * j + 1] = newg;
				//r[4 * j + 2] = newr;
				//r[4 * j + 3] = newt;
			//}
		}
	}
	return bgra;
}

void Clear_MicroConnected_Areas(cv::Mat src, cv::Mat& dst, double min_area)
{
	// 备份复制
	dst = src.clone();
	std::vector<std::vector<cv::Point> > contours;  // 创建轮廓容器
	std::vector<cv::Vec4i> 	hierarchy;

	// 寻找轮廓的函数
	// 第四个参数CV_RETR_EXTERNAL，表示寻找最外围轮廓
	// 第五个参数CV_CHAIN_APPROX_NONE，表示保存物体边界上所有连续的轮廓点到contours向量内
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());

	if (!contours.empty() && !hierarchy.empty())
	{
		std::vector<std::vector<cv::Point> >::const_iterator itc = contours.begin();
		// 遍历所有轮廓
		while (itc != contours.end())
		{
			// 定位当前轮廓所在位置
			cv::Rect rect = cv::boundingRect(cv::Mat(*itc));
			// contourArea函数计算连通区面积
			double area = contourArea(*itc);
			// 若面积小于设置的阈值
			if (area < min_area)
			{
				// 遍历轮廓所在位置所有像素点
				for (int i = rect.y; i < rect.y + rect.height; i++)
				{
					uchar* output_data = dst.ptr<uchar>(i);
					for (int j = rect.x; j < rect.x + rect.width; j++)
					{
						// 将连通区的值置0
						if (output_data[j] == 255)
						{
							output_data[j] = 0;
						}
					}
				}
			}
			itc++;
		}
	}
}