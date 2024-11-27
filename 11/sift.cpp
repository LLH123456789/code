#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>
#include<windows.h>
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

int main() {

	Mat img1 = imread("E://2//5//Left301.jpg");
	Mat img2 = imread("E://2//5//Right301.jpg");
	if (img1.empty() or img2.empty()) {
		printf("kong\n");
		return -1;
	}
	//Mat img1;
	//Mat img2;
	//cvtColor(src1, img1, COLOR_BGR2GRAY);
	//cvtColor(src2, img2, COLOR_BGR2GRAY);
	// SURF 特征检测与匹配
	int minHessian = 1000;
	Ptr<SURF> detector = SURF::create(minHessian);
	Ptr<DescriptorExtractor> descriptor = SIFT::create();
	Ptr<DescriptorMatcher> matcher1 = DescriptorMatcher::create("BruteForce");
	//    BFMatcher matcher1(NORM_L2);

	std::vector<KeyPoint> keyPoint1, keyPoint2;
	Mat descriptors1, descriptors2;
	std::vector<DMatch> matches;

	// 检测特征点
	detector->detect(img1, keyPoint1);
	detector->detect(img2, keyPoint2);
	// 提取特征点描述子
	descriptor->compute(img1, keyPoint1, descriptors1);
	descriptor->compute(img2, keyPoint2, descriptors2);
	// 匹配图像中的描述子
	matcher1->match(descriptors1, descriptors2, matches);

	Mat img_keyPoint1, img_keyPoint2;
	drawKeypoints(img1, keyPoint1, img_keyPoint1, Scalar(0,0,255), DrawMatchesFlags::DEFAULT);
	imshow("keyPoint1 SURF", img_keyPoint1);
	imwrite("E://2//7//sift.jpg", img_keyPoint1);
	drawKeypoints(img2, keyPoint2, img_keyPoint2, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	imshow("keyPoint2 SURF", img_keyPoint2);
	cout << "find " << keyPoint1.size() << " points" << endl;
	Mat img_matches;
	drawMatches(img1, keyPoint1, img2, keyPoint2, matches, img_matches);
	imshow("img_matches", img_matches);
	//imwrite("E://2//3//sift//30sift.png", img_matches);

	cout << "keyPoint1.size = " << keyPoint1.size() << endl;
	cout << "keyPoint2.size = " << keyPoint2.size() << endl;
	cout << "descriptors1.size = " << descriptors1.size() << endl;
	cout << "descriptors1.size = " << descriptors2.size() << endl;
	cout << "matches.size = " << matches.size() << endl;
	//    for (int i = 0; i < matches.size(); i++)
	//        cout << matches[i].distance << ' ';
	//    cout << endl;
	int i = 0;
	double t1 = cv::getTickCount();
	while (i < 1000)
	{
		//do sth      
		i++;
	}

	double t2 = cv::getTickCount();
	double	time = 1000 * (t2 - t1) / cv::getTickFrequency();
	cout << "匹配耗时： " << time << "ms" << endl;//输出程序运行1000次的时间
	waitKey(0);
	return 0;
}