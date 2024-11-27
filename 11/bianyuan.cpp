#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


Mat g_src;
int T_value = 50;
int T_max = 255;

void Callcanny(int, void*) {
	Mat gauss, gray, edge, dst;
	GaussianBlur(g_src, gauss, Size(3, 3), 0, 0, 4);
	cvtColor(gauss, gray, COLOR_BGR2GRAY);
	Canny(gray, edge, T_value / 2, T_value, 3, false);
	g_src.copyTo(dst, edge);
	imshow("Canny", dst);
}
void myCannyTrackbar() {
	namedWindow("Canny", WINDOW_AUTOSIZE);
	createTrackbar("阈值大小", "Canny", &T_value, T_max, Callcanny);
	Callcanny(0, 0);
}

int main(int argc, char** argv) {
	const string path = "E:/2/new/1/result1.png";//将这里换成你的图片保存的路径即可
	g_src = imread(path);
	if (!g_src.data) {
		cout << "could not load the image" << endl;
		return -1;
	}
	double t1 = cv::getTickCount();
	myCannyTrackbar();
	double t2 = cv::getTickCount();
	double t = (t2 - t1) / cv::getTickFrequency();
	cout << "time cost " << t << endl;
	waitKey(0);
	destroyAllWindows();
	return 0;
}
