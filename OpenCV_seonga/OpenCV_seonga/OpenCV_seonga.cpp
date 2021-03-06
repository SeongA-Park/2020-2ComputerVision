// OpenCV_seonga.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp> // image processing
#include <vector>
#include "OpenCV_seonga.h"
using namespace std;
using namespace cv;

vector<Mat_<Vec3b>> pyrA, pyrB, pyrM; 
vector<Mat_<Vec3f>> C;
 
// 라플라시안 피라미드 
void Laplacian(Mat& input, int layer, char inputChar) {

	Mat inputImage = input;
	int index = layer;
	vector<Mat_<Vec3b>> curPyr;
	curPyr.clear();

	for (int i = 0; i < index; i++) {
		Mat downSample, upSample;
		
		if (i < index - 1) {

			pyrDown(inputImage, downSample);
			pyrUp(downSample, upSample, inputImage.size());
			
			Mat lapLayer = inputImage - upSample;
			curPyr.push_back(lapLayer);

		}
		else {

			Mat lapLayer = inputImage;
			curPyr.push_back(lapLayer);
		}
		inputImage = downSample;
	}

	if (inputChar == 'A') {
		pyrA.clear();
		pyrA.assign(curPyr.begin(), curPyr.end());
		
	}
	else {
		pyrB.clear();
		pyrB.assign(curPyr.begin(), curPyr.end());
	}
}

// 가우시안 피라미드
void Gaussian(Mat& input, int layer) {

	Mat inputImage = input;
	
	vector<Mat_<Vec3b>> curPyr;
	curPyr.clear();

	for (int i = 0; i < layer; i++) {
		Mat downSample;
		
		if (i != 0) {
			pyrDown(inputImage, downSample, pyrA[i].size());

			curPyr.push_back(downSample);
			inputImage = downSample;
		}
		else {
			curPyr.push_back(inputImage);
		}
	}
	pyrM.assign(curPyr.begin(), curPyr.end());
}

// 원복
Mat_<Vec3f> restoration(Mat& input, int layer) {
	Mat inputImage = input;

	for (int i = layer-1; i > 0; i--) {
		Mat upSample;

		pyrUp(inputImage, upSample, C[i-1].size());
		inputImage = upSample + C[i-1];
	}
	return inputImage;
}

int main()
{
	// 경로 임의 설정
	Mat inputA = imread("burt_apple.png");
	Mat inputB = imread("burt_orange.png");
	Mat mask = imread("burt_mask.png");

	// 레이어 장 수 
	int layer = 6; 

	Laplacian(inputA, layer, 'A');
	Laplacian(inputB, layer, 'B');
	Gaussian(mask, layer);

	C.clear();

	// alpha blending 
	for (int i = 0; i < layer; i++) {

		Mat_<Vec3f> maskF;
		Mat_<Vec3f> pyrA_F;
		Mat_<Vec3f> pyrB_F;
	
		pyrM[i].convertTo(maskF, CV_32F, 1.0 / 255.0);
		pyrA[i].convertTo(pyrA_F, CV_32F, 1.0 / 255.0);
		pyrB[i].convertTo(pyrB_F, CV_32F, 1.0 / 255.0);


		cv::Mat maskA = (pyrA_F).mul(Scalar(1.0, 1.0, 1.0) - maskF);
		cv::Mat maskB = (pyrB_F).mul(maskF);

		C.push_back(maskA+ maskB);
	}

	Mat result = restoration(C[layer-1], layer);

	imshow("MultiBlending", result);
	waitKey();
}
