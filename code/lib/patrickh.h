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
        totalDiff += (float)abs(im1.at<uchar>(x,y) - im2.at<uchar>(x,y))/255.0;
      }
    }
  } 
  return 100.0*totalDiff/(double)(im1.cols * im1.rows);
}

void detecCorners(Mat img, Mat& result)
{   
    #if 0
    /// Detector parameters
    int blockSize = 5;
    int apertureSize = 6;
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
    Canny( img, result, 4, 1, 3 );
    #else
    result=img.clone();
    #endif
    #endif
}