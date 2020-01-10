#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
/**************************************************************\
This programm is meant to do semi-automatic measurements.
It requires a camera be perpendicular to the surface on which
distances will be measured.

The first step is to callibrate the programm by pressing c when
a reference distance, or an reference object is in the picture.
Callibration window opens. Then the user shall select a known
distance. If the user is satisfy with the selected distance,
he presses v and enters the distance in mm in the command line 
interface. Otherwise he selects another distance. The reference 
distance should be measured as the same distance from the camera
that the future measurements shall take place.

Once the callibration is done, the user is back to the main
live camera window. Once an object that the user wants to
measure enters the picture, press m. A new window will open
with the last camera picture. On this window the user shall
left click and drag the mouse to get measurements.

Press q for exit the program.
\**************************************************************\





/* Global variable used to store the current image of the camera 
* so that mouse events can refresh callibration and measurements
* window. Should be remove at some point. Maybe by using the user 
* data pointer to pass two information for mouse events.
*/
Mat dst;

void callibration_on_mouse(int event, int x, int y, int flags, void* ustc);
void on_mouse_measurements(int event, int x, int y, int flags, void* ustc);
double getDistance(CvPoint pointO, CvPoint pointA);

/* main handles the live camera display and keyboard events
*/
int main(int argc, char** argv)
{
	double pixelDistance=-1;
	double ratio = 1; //mm/pixel
	const std::string mainWindowName = "Live Camera";
	const std::string callibrationWindowName = "Calibration";
	const std::string measurementWindowName = "Measurements";

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
		cv::putText(img, "Press m for measuring", CvPoint(10, 340), font, 0.5, CvScalar(255, 255, 255));

		cv::imshow(mainWindowName, img);

		/* Handles keyboard input */
		if ((key=cv::waitKey(30)) >= 0)
		{
			switch (key)
			{

			/*Exit the program*/
			case 'q':
				std::cout << " q has been pressed \n";
				keepGoing = false;
				break;

			/* Open the callibration window with the current frame and set mouse callback */
			case 'c':
				std::cout << " c has been pressed \n";
				cv::namedWindow(callibrationWindowName, cv::WINDOW_AUTOSIZE);
				frame.copyTo(dst);
				setMouseCallback(callibrationWindowName, callibration_on_mouse, &pixelDistance);
				cv::imshow(callibrationWindowName, frame);
				break;

			/* Prompt the user for the distance in mm of the measure done in the callibration window */
			/* Compute the mm/px ratio and close the callibration window */
			case 'v':
				std::cout << " v has been pressed \n";
				if (cvGetWindowHandle("Calibration")) {
					int actualDistance;
					//std::cout << cvGetWindowHandle("Calibration") << '\n';
					std::cout << "curent pixel distance selected : " << pixelDistance << std::endl;
					std::cout << "Please enter the distance between the two selected points in mm and press enter : \n" << std::endl;
					std::cin >> actualDistance;
					ratio = actualDistance / pixelDistance;
					std::cout << "ratio : " << ratio <<" mm/px"<< std::endl;
					destroyWindow(callibrationWindowName);
				}
				break;
			
			/* Open a measurement window with the current frame */
			case 'm':
				std::cout << " m has been pressed \n";
				frame.copyTo(dst);
				cv::imshow(measurementWindowName, frame);
				setMouseCallback(measurementWindowName, on_mouse_measurements, &ratio);
				break;

			default:
				break;
			}
		}
	}
	cvDestroyWindow("Live Camera");
	return (0);
}

/*This callback function is set on the callibration window
* It handles mouse events to display the selected reference
* line on the image.
*/
void callibration_on_mouse(int event, int x, int y, int flags, void* userData)
{
	Mat src;
	static Point pre_pt;
	static Point cur_pt;
	const char validation_message[52] = "Press v for keeping this line or trace another one";
	double length;

	/*
	* Register the starting point when the left button of the mouse is pressed
	*/
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		dst.copyTo(src);
		pre_pt = Point(x, y);
		circle(src, pre_pt, 0.5, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		imshow("Calibration", src);
	}

	/* When the mouse is mooved and the left button is still pressed
	* a green line is drawn between the starting point and the current 
	* position of the mouse.
	*/
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		dst.copyTo(src);
		cur_pt = Point(x, y);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		imshow("Calibration", src);
	}

	/* When the left button of the mouse is realeased, we draw the last
	* green line and we measure the distance in pixel between those two points
	*/
	else if (event == CV_EVENT_LBUTTONUP)
	{

		dst.copyTo(src);
		cur_pt = Point(x, y);
		double dist = getDistance(pre_pt, cur_pt);
		//circle(src, cur_pt, 3, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		putText(src, validation_message, CvPoint(50,50), FONT_HERSHEY_DUPLEX, 0.6, CvScalar(0, 255, 0));
		imshow("Calibration", src);
	}
	*((double*)userData) = getDistance(pre_pt, cur_pt);
}

/*This callback function is set on the measurement window
* It handles mouse events to display measurements information
* in the measurement window (line coresponding to the distance
* measured and measure value according to the mm/px ratio)
*/
void on_mouse_measurements(int event, int x, int y, int flags, void* userData)
{
	Mat src;
	static Point pre_pt;
	static Point cur_pt;
	char temp_1[20];
	char temp_2[20];
	double length;

	/* Register the starting point when the left button of the mouse is pressed */
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		dst.copyTo(src);
		pre_pt = Point(x, y);
		circle(src, pre_pt, 0.5, cvScalar(255, 0, 0), CV_FILLED, CV_AA, 0);
		imshow("Measurements", src);
	}

	/* When the mouse is mooved and the left button is still pressed 
	* or when the left button is realeased, the line is drawn between
	* the starting point and the current position of the mouse.
	* Additionnaly we display a text with the value in mm of the distance
	* measured by the green line.
	*/
	else if ((event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))||event== EVENT_LBUTTONUP)
	{
		dst.copyTo(src);
		cur_pt = Point(x, y);
		line(src, pre_pt, cur_pt, cvScalar(0, 255, 0), 1, CV_AA, 0);
		double dist = getDistance(pre_pt, cur_pt);
		double ratio = *((double*)userData);
		sprintf_s(temp_1, "%f mm", ratio*dist);

		putText(src, temp_1, (pre_pt+cur_pt)/2, FONT_HERSHEY_SIMPLEX, 0.4, CvScalar(0, 255, 0));
		imshow("Measurements", src);
	}
}


/* This function return the distance in pixels between the points passed as parameters
*/
double getDistance(CvPoint pointO, CvPoint pointA)
{
	double distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);

	return distance;

}
