#ifdef WIN32
  #include <time.h>
#else // UNIX
  #include <sys/time.h>
#endif

#include <iostream>
#include <cstdio>
#include <cstring>
#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace cv;   // declara o namespaec cv::Mat

// start_fps();
// end_fps();

// drawCross( center, color, d, drawing )  

// limpa_linha(int numero_de_linhas);
// desenha_seta(Mat image, Point2f coco1, Point2f coco2,Scalar cor);

/*DEFININDO ALGUMAS VARIAVEIS LEGAIS*/
Scalar red (Scalar(0,0,255));
Scalar green (Scalar(0,255,0));
Scalar blue (Scalar(255,0,0));   
Scalar cinza (Scalar(190,190,190));
static const double pi = 3.14159265358979323846;

time_t start, end;
double fps;
double sec;
int counter=0;
int numframes=0;

#define start_fps() time(&start)  //inicia a contagem de tempo

void end_fps()      //finaliza a contagem de tempo
{
    time(&end);
    counter++;  // calcula o fps
    sec = difftime (end, start);
    fps = counter / sec;
    printf("FPS = %.2f\n", fps);   //mostra o fsp
}

#define drawCross( center, color, d, drawing )                      	\
    	line( drawing, Point( center.x - d, center.y - d ),             \
    	Point( center.x + d, center.y + d ), color, 2, CV_AA, 0);       \
    	line( drawing, Point( center.x + d, center.y - d ),             \
    	Point( center.x - d, center.y + d ), color, 2, CV_AA, 0 )
///// EXEMPLO drawCross(cvPoint(X,Y),Scalar(0,0,255), int RAIO,cv Mat);

void limpa_linha(int numero_de_linhas)
{
	for (int i = 0; i < numero_de_linhas; ++i)
	{
		printf("\033[K" );
		printf("\033[1A");
        printf("\033[K" );
	}
}

void desenha_seta(Mat image, Point2f coco1, Point2f coco2,Scalar cor)
{
    line( image, coco1, coco2, cor);
    double Theta=atan2( (double) coco1.y - coco2.y, (double) coco1.x - coco2.x );
    double hypotenuse = 10;//sqrt( pow(coco1.y - coco2.y,2) + pow(coco1.x - coco2.x,2) );
    coco1.x=(float)floor(hypotenuse*cos(Theta)/4-hypotenuse*sin(Theta)/4+coco2.x);
    coco1.y=(float)floor(hypotenuse*sin(Theta)/4+hypotenuse*cos(Theta)/4+coco2.y);
    line(image,coco1,coco2,cor);  //Draw Line
    coco1.x=(float)floor(hypotenuse*cos(Theta)/4+hypotenuse*sin(Theta)/4+coco2.x);
    coco1.y=(float)floor(hypotenuse*sin(Theta)/4-hypotenuse*cos(Theta)/4+coco2.y);
    line(image,coco1,coco2,cor);  //Draw Lin
}

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define printBranco   printf("%s", KNRM)
#define printVerde    printf("%s", KGRN)
#define printVermelho printf("%s", KRED)
#define Cwarning printf("%s[WARNING]   ", KYEL); printBranco
#define Cerro printf("%s[ERROR]     ", KRED); printBranco
#define Cok printf("%s[OK]        ", KGRN); printBranco
#define Caviso printf("%s[AVISO]     ", KCYN); printBranco
/*
    printf("%sred\n", KRED);
    printf("%sgreen\n", KGRN);
    printf("%syellow\n", KYEL);
    printf("%sblue\n", KBLU);
    printf("%smagenta\n", KMAG);
    printf("%scyan\n", KCYN);
    printf("%swhite\n", KWHT);
    printf("%snormal\n", KNRM);
*/

/// http://rosettacode.org/wiki/Percentage_difference_between_images
float diffMat(Mat im1, Mat im2)
{
  float totalDiff=0.0f;
  if ( (im1.cols != im2.cols) || (im1.rows !=im2.rows) )
  {
    printf("width/height of the images must match!\n");
  } 
  else 
  {
    /* BODY: the "real" part! */
    for(int x=0; x < im1.rows; x++)
    {
      for(int y=0; y < im1.cols; y++)
      {
        totalDiff += (float)abs(im1.at<uchar>(x,y) - im2.at<uchar>(x,y))/255;
      }
    }
  } 
  return (100*totalDiff)/(im1.cols * im1.rows);
}

void detecCorners(Mat img, Mat& result)
{   
    #if 0
    /// Detector parameters
    int blockSize = 2;
    int apertureSize = 4;
    double k = 1;

    /// Detecting corners
    Mat dst;
    cornerHarris( img, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

    /// Normalizing
    Mat dst_norm;
    normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
    convertScaleAbs( dst_norm, result );
    #else
    #if 0
    Canny( img, result, 4, 100, 3 );
    #else
    result=img.clone();
    #endif
    #endif
}

void MatHistogram(Mat src, Mat& histImage)
{

  /// Separate the image in 3 places ( B, G and R )
  vector<Mat> bgr_planes;
  split( src, bgr_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;

  /// Compute the histograms:
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for B, G and R
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  histImage.create( hist_h, hist_w, CV_8UC3);
  histImage = Scalar(0,0,0);

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  /// Display
}

int DistTwoPoints(Point a, Point b)
{
    return (int)sqrt(pow((double)(a.x-b.x),2)+pow((double)(a.y-b.y),2));
}

void DrawBox(Mat& frame, Point a, int size1, int size2, Scalar colour, int tickness, int line_type, int shift )
{
    line(frame, Point(a.x-size1/2,a.y-size2/2), Point(a.x-size1/2,a.y+size2/2), colour, tickness, line_type, shift);
    line(frame, Point(a.x-size1/2,a.y-size2/2), Point(a.x+size1/2,a.y-size2/2), colour, tickness, line_type, shift);
    line(frame, Point(a.x+size1/2,a.y-size2/2), Point(a.x+size1/2,a.y+size2/2), colour, tickness, line_type, shift);
    line(frame, Point(a.x-size1/2,a.y+size2/2), Point(a.x+size1/2,a.y+size2/2), colour, tickness, line_type, shift);
}