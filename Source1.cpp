/**
* @function findContours_Demo.cpp
* @brief Demo code to find contours in an image
* @author OpenCV team
*/

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

// Constants
#define PI 3.14159265

Mat src; Mat src_gray; Mat drawing; Mat path_drawing;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
Point current_point = Point(0, 0);
vector<vector<Point> > contours;
vector<Point> path;
unsigned short ball_path = 0;
unsigned short path_start = 0;
unsigned short path_finish = 0;
int path_angle = 50;
int max_angle = 200;


/// Function header
void thresh_callback(int, void*);
void onMouse(int evt, int x, int y, int flags, void* param);
void get_path(int, void*);
void find_nearest_point(int, void*);
void update_angle(int, void*);
void onMouse2(int evt, int x, int y, int flags, void* param);
/**
* @function main
*/
int main(int, char** argv)
{
	/// Load source image
	src = imread("wooden-labyrinth-maze.jpg", IMREAD_COLOR);
	if (src.empty())
	{
		cerr << "No image supplied ..." << endl;
		return -1;
	}

	/// Convert image to gray and blur it
	cvtColor(src, src_gray, COLOR_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3));

	/// Create Window
	const char* source_window = "Source";
	namedWindow(source_window, WINDOW_AUTOSIZE);
	imshow(source_window, src);

	
	createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);

	setMouseCallback("Contours", onMouse, &current_point);
	int X, Y, X2, Y2;
	while (1)
	{
		X = current_point.x;
		Y = current_point.y;
		//cout << "path start1: " << path_start << endl;
		if (X > 0) {
			printf("X1:%i  Y1:%i \n", X, Y);
			break;
		}
		waitKey(10);
	}
	find_nearest_point(0, 0);
	//imshow("Contours", drawing);
	//waitKey(10);
	// close the first window so that we cannot alter the contours after we have already chosen the start point
	destroyWindow(source_window);

	setMouseCallback("Contours", onMouse, &current_point);
	while (1)
	{
		X2 = current_point.x;
		Y2 = current_point.y;
		//cout << "path start1: " << path_start << endl;
		if (path_start > 0 && X2 != X) {
			cout << "path start: " << path_start << endl;
			printf("X:%i  Y:%i \n", X, Y);
			printf("X2:%i  Y2:%i \n", X2, Y2);
			break;
		
		}
		waitKey(10);
	}

	// Get path through contours
	find_nearest_point(0,0);
	cout << "path finish: " << path_finish << endl;
	//imshow("Contours", drawing);
	//waitKey(10);
	createTrackbar(" Angle x10:","Contours" , &path_angle, max_angle, update_angle);
	//createButton(" Update", get_path, &path_angle, QT_PUSH_BUTTON, false );
	//update_angle(0, 0);
	setMouseCallback("Contours", onMouse2, &path_angle);
	get_path(0, 0);
	
	waitKey(0);
	return(0);
}


void onMouse(int evt, int x, int y, int flags, void* param) {
	if (evt == CV_EVENT_LBUTTONDOWN) {
		Point* ptPtr = (Point*)param;
		ptPtr->x = x;
		ptPtr->y = y;
	}
}
void onMouse2(int evt, int x, int y, int flags, void* param) {
	if (evt == CV_EVENT_LBUTTONDOWN) {
		get_path(0, 0);
	}
}

/**
* @function thresh_callback
*/
void thresh_callback(int, void*)
{
	Mat canny_output=Mat::zeros(src.size(), CV_8UC3);
	//vector<vector<Point> > contours2 = contours;
	vector<Vec4i> hierarchy;
	cout << "path start1: " << path_start << endl;
	/// Detect edges using canny
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);
	
	/// Show in a window
	namedWindow("Canny", WINDOW_AUTOSIZE);
	imshow("Canny", canny_output);
	
	/// Find contours
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	double max_area = 1; // area threshold
						 
	/// Draw contours
	drawing = Mat::zeros(src.size(), CV_8UC3);

	// find the main path
	for (unsigned short i = 0; i < contours.size(); i++)
	{
		double area = contourArea((contours[i]), false);  //  Find the area of contour
		if (area > max_area && area < 100000)
		{
			max_area = area;
			ball_path = i;
		}
	}

	// find the holes
	vector<Vec3f> circles;

	/// Apply the Hough Transform to find the circles
	HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, 5, thresh/3, thresh/3, 10, 16);

	/// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// circle center
		circle(drawing, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		// circle outline
		circle(drawing, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}



	//printf("size of path : %i \n", contours[ball_path].size());

	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	drawContours(drawing, contours, ball_path, color, 1, 8, hierarchy, 0, Point());
	/// Show in a window
	imshow("Contours", drawing);
	//waitKey(10);
	
	
}

void find_nearest_point(int, void*) {
	
	double angleMax = path_angle * PI / 180;
	double dist_from_path = 0;
	double dist_to_point = 0;
	dist_from_path = pointPolygonTest(contours[ball_path], current_point, true);
	double x1, x2, y1, y2;
	x1 = current_point.x;
	y1 = current_point.y;
	double diff = 0;
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

	for (int i = 0; i < contours[ball_path].size(); i++)
	{
		//cout << "i: " << i << endl;
		x2 = contours[ball_path][i].x;
		y2 = contours[ball_path][i].y;
		dist_to_point = sqrt(pow((y2 - y1), 2) + pow((x2 - x1), 2));  //  Find the distance to the point on path
		diff = abs(dist_from_path) - dist_to_point;
		if (abs(diff) < 1)
		{
			printf("found it");
			
			if (path_start > 0) {
				path_finish = i;
				circle(drawing, contours[ball_path][path_finish], 5, color, 2, 8, 0);
				imshow("Contours", drawing);
				waitKey(10);
			}
			else {
				path_start = i;
				path.push_back(contours[ball_path][i]);
				circle(drawing, contours[ball_path][path_start], 5, color, 2, 8, 0);
				imshow("Contours", drawing);
				waitKey(10);
			}
			
			break;
		}
	}
}

void get_path(int, void*) {
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	double angleMax = path_angle * PI / 1800;

	// make sure path is empty incase of callback
	path.clear();
	path.push_back(contours[ball_path][path_start]);
	//path_drawing = Mat::zeros(src.size(), CV_8UC3);
	drawing.copyTo(path_drawing);
	double dist_from_path = 0;
	double dist_to_point = 0;
	double x1, x2, y1, y2;
	double diff = 0;

	int Newi = 0;
	int j = 0;
	double angle1, angle2;

	x1 = path[j].x;
	y1 = path[j].y;
	x2 = contours[ball_path][path_start + 1].x;
	y2 = contours[ball_path][path_start + 1].y;
	if (x2 - x1 == 0)
		angle1 = 0;
	else
		angle1 = atan((y2 - y1) / (x2 - x1));
	path.push_back(contours[ball_path][path_start + 1]);
	j++;
	// change colour again
	color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	for (unsigned short i = 2; i < contours[ball_path].size(); i += 5)
	{
		Newi = (i+path_start) % contours[ball_path].size();
		if (abs(Newi - path_finish) < 11)
			break;
		x1 = path[j].x;
		y1 = path[j].y;
		x2 = contours[ball_path][Newi].x;
		y2 = contours[ball_path][Newi].y;

		if (x2 - x1 == 0)
			angle2 = 0;
		else

			angle2 = atan((y2 - y1) / (x2 - x1));

		if (abs(angle1 - angle2) > angleMax)
		{
			path.push_back(contours[ball_path][Newi]);
			j++;
			angle1 = angle2;
			circle(path_drawing, contours[ball_path][Newi], 5, color, 2, 8, 0);
			imshow("Contours", path_drawing);
			waitKey(10);
		}
	}


	for (int i = 1; i < path.size(); i++) {
		arrowedLine(path_drawing, path[i - 1], path[i], Scalar(255, 0, 0), 1, 8, 0, 0.5);
	}

	/// Show in a window
	//namedWindow("Contours", WINDOW_AUTOSIZE);
	imshow("Contours", path_drawing);
	waitKey(10);
}

void update_angle(int, void*) {}