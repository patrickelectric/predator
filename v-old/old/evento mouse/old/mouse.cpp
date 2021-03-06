    #include <iostream>
#include <time.h>
#include <cstdio>
#include <cstring>
    // Include OpenCV
    #include <opencv/cv.h>
    #include <opencv/highgui.h>
int xx=0,yy=0;
int estado=0;
void my_mouse_callback( int event, int x, int y, int flags, void* param );

CvRect box;
bool drawing_box = false;

void draw_box( IplImage* img, CvRect rect ){
	cvRectangle( img, cvPoint(box.x, box.y), cvPoint(box.x+box.width,box.y+box.height),
				cvScalar(0xff,0x00,0x00) );
}

// Implement mouse callback
void my_mouse_callback( int event, int x, int y, int flags, void* param ){
	IplImage* image = (IplImage*) param;

	switch( event ){
		case CV_EVENT_MOUSEMOVE:
			if( drawing_box ){
				box.width = x-box.x;
				box.height = y-box.y;
			}
			break;

		case CV_EVENT_LBUTTONDOWN:
			estado=1;
			xx=x;
			yy=y;
			drawing_box = true;
			box = cvRect( x, y, 0, 0 );
			break;

		case CV_EVENT_LBUTTONUP:
			xx=x;
			yy=y;
			estado=2;
			drawing_box = false;
			if( box.width < 0 ){
				box.x += box.width;
				box.width *= -1;
			}
			if( box.height < 0 ){
				box.y += box.height;
				box.height *= -1;
			}
			draw_box( image, box );
			break;
	}
}

int main(int argc,char* argv[])
{
	const char* name = "Box Example";
	box = cvRect(-1,-1,0,0);

	IplImage* image = cvLoadImage( "frame1.png" );
	cvZero( image );
	IplImage* temp = cvCloneImage( image );

	cvNamedWindow( name );

	// Set up the callback
	cvSetMouseCallback( name, my_mouse_callback, (void*) image);

	// Main loop
	while( 1 ){
	printf("%d,%d-%d\n",xx,yy,estado);
		cvCopyImage( image, temp );
		if( drawing_box ){
			draw_box( temp, box );
			}
		cvShowImage( name, temp );
		if( cvWaitKey( 15 )==27 )
			break;
			
			
	}

	cvReleaseImage( &image );
	cvReleaseImage( &temp );
	cvDestroyWindow( name );

	return 0;
}
