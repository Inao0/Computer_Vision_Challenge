#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
Mat src;
Mat dst;
void callibration_on_mouse(int event, int x, int y, int flags, void* ustc);
void on_mouse_measurements(int event, int x, int y, int flags, void* ustc);
double getDistance(CvPoint pointO, CvPoint pointA);

int main(int argc, char** argv)
{
	double pixelDistance=-1;
	double ratio = 1; //mm/pixel
	const std::string mainWindowName = "Live Camera";
	const std::string callibrationWindowName = "Calibration";

	cv::namedWindow(mainWindowName, cv::WINDOW_AUTOSIZE);

	cv::VideoCapture cap;
	if (argc == 1)
	{
		cap.open(0);  //open the first camera
	}
	else
	{
		cap.open(argv[1]);
	}
	if (!cap.isOpened())
	{
		std::cerr << "Couldn't open capture." << std::endl;
		return (-1);
	}

	cv::Mat frame, img;
	char key;
	bool keepGoing = true;
	while (keepGoing)
	{
		cap >> frame;
		if (frame.empty())
		{
			break;
		}
		img = frame.clone();
		int font = CV_FONT_HERSHEY_SIMPLEX;
		cv::putText(img, "Press q for quit", CvPoint(10, 300), font, 0.5, CvScalar(255, 255, 255));
		cv::putText(img, "Press c for callibrating", CvPoint(10, 320), font, 0.5, CvScalar(255, 255, 255));

		cv::imshow(mainWindowName, img);
		setMouseCallback(mainWindowName, on_mouse_measurements,&ratio);

		if ((key=cv::waitKey(30)) >= 0)
		{
			switch (key)
			{
			case 'q':
				std::cout << " q has been pressed \n";
				keepGoing = false;
				break;
			case 'c':
				std::cout << " c has been pressed \n";
				cv::namedWindow(callibrationWindowName, cv::WINDOW_AUTOSIZE);
				frame.copyTo(dst);
				setMouseCallback(callibrationWindowName, callibration_on_mouse, &pixelDistance);
				cv::imshow(callibrationWindowName, frame);
				break;
			case 'v':
				std::cout << " v has been pressed \n";
				if (cvGetWindowHandle("Calibration")) {
					int actualDistance;
					std::cout << cvGetWindowHandle("Calibration") << '\n';
					std::cout << "curent pixel distance selected : " << pixelDistance << std::endl;
					std::cout << "Please enter the distance between the two selected points in mm and press enter : \n" << std::endl;
					std::cin >> actualDistance;
					ratio = actualDistance / pixelDistance;
					std::cout << "ratio : " << ratio <<" mm/px"<< std::endl;
					destroyWindow(callibrationWindowName);
				}
				break;
			default:
				break;
			}
		}
	}
	cvDestroyWindow("Live Camera");
	return (0);
}


void callibration_on_mouse(int event, int x, int y, int flags, void* userData)
{
	static Point pre_pt;
	static Point cur_pt;
	char temp_1[20];
	char temp_2[20];
	double length;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		dst.copyTo(src);
		pre_pt = Point(x, y);
		circle(src, pre_pt, 0.5, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		imshow("Calibration", src);
	}

	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		dst.copyTo(src);
		cur_pt = Point(x, y);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		imshow("Calibration", src);
	}

	else if (event == CV_EVENT_LBUTTONUP)
	{

		dst.copyTo(src);
		cur_pt = Point(x, y);
		double dist = getDistance(pre_pt, cur_pt);


		//circle(src, cur_pt, 3, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		imshow("Calibration", src);
	}
	*((double*)userData) = getDistance(pre_pt, cur_pt);
}

void on_mouse_measurements(int event, int x, int y, int flags, void* ustc)
{
	static Point pre_pt;
	static Point cur_pt;
	char temp_1[20];
	char temp_2[20];
	double length;

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		dst.copyTo(src);
		pre_pt = Point(x, y);
		sprintf_s(temp_1, "x:%d,y:%d", x, y);

		putText(src, temp_1, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255));
		circle(src, pre_pt, 0.5, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		imshow("Live Camera", src);
	}

	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		dst.copyTo(src);
		cur_pt = Point(x, y);
		sprintf_s(temp_2, "x:%d,y:%d", x, y);

		putText(src, temp_2, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(0, 255, 255));
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		imshow("Live Camera", src);
	}

	else if (event == CV_EVENT_LBUTTONUP)
	{

		dst.copyTo(src);
		cur_pt = Point(x, y);
		double dist = getDistance(pre_pt, cur_pt);


		sprintf_s(temp_2, "x:%d,y:%d", x, y);


		putText(src, temp_2, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 255, 255));

		circle(src, cur_pt, 3, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		imshow("Live Camera", src);
	}
	//double dist = getDistance(pre_pt, cur_pt);

}


double getDistance(CvPoint pointO, CvPoint pointA)

{
	double distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);

	return distance;

}
