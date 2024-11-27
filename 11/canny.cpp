#include <opencv2\opencv.hpp>
  #include <iostream>

  using namespace cv;
  using namespace std;

  int main()
  {
      //��ȡͼ�񣬺ڰ�ͼ���Ե�������Ϊ����
     Mat img1 = imread("E://2//new//1//result1.png",0);
	 Mat img2 = imread("E://2//new//1//result2.png", 0);
    if (img1.empty())
    {
        cout << "��ȷ��ͼ���ļ������Ƿ���ȷ" << endl;
        return -1;
		     }
	     Mat resultHigh1, resultLow1, resultG1;
		 Mat resultHigh2, resultLow2, resultG2;
		     //����ֵ���ͼ���Ե
	     Canny(img1, resultHigh1, 100, 200, 3);
		 Canny(img2, resultHigh2, 100, 200, 3);
	     //С��ֵ���ͼ���Ե
	     Canny(img1, resultLow1, 20, 40, 3);
		 Canny(img2, resultLow2, 20, 40, 3);
	    //��˹ģ������ͼ���Ե
	     GaussianBlur(img1, resultG1, Size(3, 3), 5);
		 GaussianBlur(img2, resultG2, Size(3, 3), 5);
    Canny(resultG1, resultG1, 100, 200, 3);
	Canny(resultG2, resultG2, 100, 200, 3);
	
	    //��ʾͼ��
		  //   imshow("resultHigh1", resultHigh1);
	    imshow("resultLow1", resultLow1);
	  //  imshow("resultG1", resultG1);
	//	imshow("resultHigh2", resultHigh2);
		imshow("resultLow2", resultLow2);
	//	imshow("resultG2", resultG2);
		//imwrite("E://2//new//1//result3.png", resultLow1);
		//imwrite("E://2//new//1//result4.png", resultLow2);
	     waitKey(0);
	    return 0;
	 }