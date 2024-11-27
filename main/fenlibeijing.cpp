#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <time.h>
using namespace cv;
using namespace std;

// �������
struct Inputparama {
	int thresh = 30;                               // ����ʶ����ֵ����ֵԽС����ʶ��Ǳ��������Խ�����к��ʷ�Χ��ĿǰΪ5-60
	int transparency = 255;                        // �����滻ɫ͸���ȣ�255Ϊʵ��0Ϊ͸��
	int size = 7;                                  // �Ǳ�������Ե�黯��������ֵԽ�����Ե�黯�̶�Խ����
	cv::Point p = cv::Point(1260, 20);                 // ����ɫ�����㣬��ͨ���˻�������ȡ��Ҳ����Ĭ��(0,0)����ɫ��Ϊ����ɫ
	cv::Scalar color = cv::Scalar(255, 255, 255);  // ����ɫ
};

cv::Mat BackgroundSeparation(cv::Mat src, Inputparama input);
void Clear_MicroConnected_Areas(cv::Mat src, cv::Mat& dst, double min_area);

// �����ֵ������
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

// ��������
cv::Mat BackgroundSeparation(cv::Mat src, Inputparama input)
{
	cv::Mat bgra, mask;
	// ת��ΪBGRA��ʽ����͸���ȣ�4ͨ��
	cvtColor(src, bgra, COLOR_BGR2BGRA);
	mask = cv::Mat::zeros(bgra.size(), CV_8UC1);
	int row = src.rows;
	int col = src.cols;

	// �쳣��ֵ����
	input.p.x = max(0, min(col, input.p.x));
	input.p.y = max(0, min(row, input.p.y));
	input.thresh = max(5, min(200, input.thresh));
	input.transparency = max(0, min(255, input.transparency));
	input.size = max(0, min(30, input.size));

	// ȷ������ɫ
	uchar ref_b = src.at<Vec3b>(input.p.y, input.p.x)[0];
	uchar ref_g = src.at<Vec3b>(input.p.y, input.p.x)[1];
	uchar ref_r = src.at<Vec3b>(input.p.y, input.p.x)[2];

	// �����ɰ�������Ĥ��
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

	// Ѱ����������������������ںڶ�
	vector<vector<Point>> contour;
	vector<Vec4i> hierarchy;
	// RETR_TREE����״�ṹ��ȡ����������CHAIN_APPROX_NONE��ȡ������ÿ������
	findContours(tmask, contour, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	drawContours(tmask, contour, -1, Scalar(255), FILLED, 16);

	// ��ñ�����ȡͬ����ɫ���Ƶ�����ʶ������
	cv::Mat hat;
	cv::Mat element = getStructuringElement(MORPH_ELLIPSE, Size(31, 31));
	cv::morphologyEx(tmask, hat, MORPH_BLACKHAT, element);
	hat.setTo(255, hat > 0);
	cv::Mat hatd;
	Clear_MicroConnected_Areas(hat, hatd, 450);
	tmask = tmask + hatd;
	mask = tmask(cv::Range(25, 25 + mask.rows), cv::Range(25, 25 + mask.cols)).clone();

	// ��Ĥ�˲�����Ϊ�˱�Ե�黯
	//cv::blur(mask, mask, Size(2 * input.size + 1, 2 * input.size + 1));

	// ��ɫ
	for (int i = 0; i < row; ++i)
	{
		uchar* r = bgra.ptr<uchar>(i);
		uchar* m = mask.ptr<uchar>(i);
		for (int j = 0; j < col; ++j)
		{
			// �ɰ�Ϊ0��������Ǳ�׼������
			if (m[j] == 0)
			{
				r[4 * j] = uchar(input.color[0]);
				r[4 * j + 1] = uchar(input.color[1]);
				r[4 * j + 2] = uchar(input.color[2]);
				r[4 * j + 3] = uchar(input.transparency);
			}
			// ��Ϊ0�Ҳ�Ϊ255���������������򣨱�Ե��������Ҫ�黯����
			//else if (m[j] != 255)
			//{
				// ��Ե����������ɫ
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
	// ���ݸ���
	dst = src.clone();
	std::vector<std::vector<cv::Point> > contours;  // ������������
	std::vector<cv::Vec4i> 	hierarchy;

	// Ѱ�������ĺ���
	// ���ĸ�����CV_RETR_EXTERNAL����ʾѰ������Χ����
	// ���������CV_CHAIN_APPROX_NONE����ʾ��������߽������������������㵽contours������
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());

	if (!contours.empty() && !hierarchy.empty())
	{
		std::vector<std::vector<cv::Point> >::const_iterator itc = contours.begin();
		// ������������
		while (itc != contours.end())
		{
			// ��λ��ǰ��������λ��
			cv::Rect rect = cv::boundingRect(cv::Mat(*itc));
			// contourArea����������ͨ�����
			double area = contourArea(*itc);
			// �����С�����õ���ֵ
			if (area < min_area)
			{
				// ������������λ���������ص�
				for (int i = rect.y; i < rect.y + rect.height; i++)
				{
					uchar* output_data = dst.ptr<uchar>(i);
					for (int j = rect.x; j < rect.x + rect.width; j++)
					{
						// ����ͨ����ֵ��0
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