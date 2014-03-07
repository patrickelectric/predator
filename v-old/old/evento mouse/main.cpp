#include <iostream>
#include <time.h>
#include <cstdio>
#include <cstring>
#include <cv.h>
#include <highgui.h>
using namespace cv;

Mat empty_image = Mat::zeros( 600, 480, CV_8UC3 );


void draw_box( Mat images, CvRect rect )
{
	CvRect box;
	rectangle( images, Point(box.x, box.y), Point( box.x+box.width,box.y+box.height), Scalar( 0, 255, 255 ), 0,8 );
}

bool state=false;
Point box_coord[2];

void onMouse( int event, int x, int y, int, void* )
{
	//printf("event: %d\n",event);
	switch( event )
	{

	case CV_EVENT_MOUSEMOVE:
		if(state==true)
		{
			box_coord[1].x=x;
			box_coord[1].y=y;
		}
		break;

	case CV_EVENT_LBUTTONDOWN:
		box_coord[0].x=x;
		box_coord[0].y=y;
		state=true;
		break;

	case CV_EVENT_LBUTTONUP:
		box_coord[1].x=x;
		box_coord[1].y=y;
		state=false;
		break;
	}
}

int main(int argc,char* argv[])
{
	while(true)
	{	
		//empty_image = Mat::zeros( 600, 480, CV_8UC3 );
		setMouseCallback( "lelesco",onMouse, 0);
		imshow("lelesco",empty_image);
		waitKey(30);
		printf(">>>>>>>>>\n");
		printf("(x,y) %d,%d\n",box_coord[0].x,box_coord[0].y);
		printf("(x,y) %d,%d\n",box_coord[1].x,box_coord[1].y);
		printf("%s\n", state?"true":"false");
	}
	return 0;
}
