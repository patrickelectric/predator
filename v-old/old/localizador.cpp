#include <iostream>
#include <time.h>
#include <cstdio>
#include <cstring>
    // Include OpenCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

time_t start, end;
double fps;
double sec;
int counter=0;
int numframes=0;

    using namespace cv;
    #define drawCross( center, color, d, drawing )                                 \
                line( drawing, Point( center.x - d, center.y - d ),                \
                Point( center.x + d, center.y + d ), color, 2, CV_AA, 0); \
                line( drawing, Point( center.x + d, center.y - d ),                \
                Point( center.x - d, center.y + d ), color, 2, CV_AA, 0 )

void start_fps() {time(&start);}  //inicia a contagem de tempo

void end_fps()      //finaliza a contagem de tempo
{
    time(&end);
    counter++;  // calcula o fps
    sec = difftime (end, start);
    fps = counter / sec;
    printf("FPS = %.2f\n", fps);   //mostra o fsp
}

    RNG rng(12345);
    int main( int argc, char** argv )
    {
        vector<Moments> te( 200 );
        Mat frame1;
        Mat frame1HSV;
        Mat frame1TEMP;
        Mat frame1BI;
        Mat frame1COUN;
        Mat frame1BIG;
        Mat frame1PEQ;
        Mat frame1CORTE;
        vector<Mat> slices;
        int fatordepreto=80;
        VideoCapture cap1(0); // Abre a camera 0 e bota m cap
        start_fps();
        while("1")
        {
        //frame1 = imread("praia5.png",3);
        //frame1CORTE = cvCreatImage( 640,480, frame1->depth, frame1->nChannels );//
        //etImageROI( frame1CORTE,  rect( 640/3,0, 640,480 ) );
        cap1 >> frame1;
        numframes++;
        int width = frame1COUN.cols =frame1.cols;
        int height = frame1COUN.rows =frame1.rows;
        frame1HSV.create(height,width,CV_8UC(15));
        //////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////BINARY//////////////////////////////////////////
        cvtColor(frame1,frame1HSV, CV_BGR2HSV);
        split(frame1,slices);
        slices[2].copyTo(frame1TEMP);
        int kernel_size =5;
        Mat kernel = Mat::ones( kernel_size, kernel_size, CV_32F )/ (float)(kernel_size*kernel_size);
        filter2D(frame1TEMP, frame1BIG, -1 , kernel, Point(-1, -1), 0, BORDER_DEFAULT   );
        threshold(frame1BIG,frame1BI,fatordepreto,255,CV_THRESH_BINARY);                      /////// MUDAR O VALOR DE 255 PARA 1, PARA BINARIZAR
        /////////////////////////////////CONTOURS/////////////////////////////////////////
        vector<vector<Point> > contours;
        Canny( frame1BI, frame1TEMP, height, width, 3 );
        Mat dst = Mat::zeros(frame1BI.rows, frame1BI.cols, CV_8UC3);
        vector<Vec4i> hierarchy;
        findContours( frame1TEMP, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

        /// Get the moments
        vector<Moments> mu(contours.size() );
        vector<Point2i> mc( contours.size() );
        for( int i = 0; i < contours.size(); i++ )
            {
                mu[i] = moments( Mat(contours[i]), false );
                if(mu[i].m01/mu[i].m00 >200)
                    {
                        mc[i] = Point2i( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);
                        printf("%.2d,%.2d\n", (int) (mu[i].m10/mu[i].m00) ,(int) (mu[i].m01/mu[i].m00) );
                        //inteiro++;
                    }

            }

        // iterate through all the top-level contours,
        // draw each connected component with its own random color
        string text;
        char buff[255];
        for( int i = 0; i < contours.size(); i++ )
        {
            Scalar color(  rng.uniform(0,255),  rng.uniform(0,255),  rng.uniform(0,255) );
            drawContours( frame1BI, contours, i, color, CV_FILLED, 8, hierarchy );
        }
        //printf("inteiro %d\n",inteiro);
        long int area[255];

        for (int i = 0; i < 255; ++i) area[i]=0;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                if (frame1BI.at<cv::Vec3b>(j,i)[0]==229)
                {
                    printf("%d,%d---------><><><><><><>\n",i,j );
                }
                area[frame1BI.at<cv::Vec3b>(j,i)[0]]++;
            }
        }
         Mat frame1PB=frame1BI;

        for (int i = 0; i < 255; ++i) printf("%ld-%d\n",area[i],i );

        string text2;
        int patrick=1;
        for (int i = 0; i < contours.size()-1; ++i)
        {
            sprintf(buff, "%d", patrick);
            text2 = std::string(buff);
            //printf("patrick %d\n",patrick );
            if(mu[i].m01/mu[i].m00 > 200)
                {
                    int a=0;
                    /*
                    while(abs((int)(te[a].m01/te[a].m00)-(int)(mu[i].m01/mu[i].m00))>2 && abs((int)(te[a].m10/te[a].m00)-(int)(mu[i].m10/mu[i].m00))>2)
                    {
                        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA %d\n",abs((int)(te[a].m01/te[a].m00)-(int)(mu[i].m01/mu[i].m00)) );
                        printf("-----%.2d,%.2d-----%.2d,%.2d-----\n", (int)(mu[i].m10/mu[i].m00) , (int)(mu[i].m01/mu[i].m00),(int)(te[a].m10/te[a].m00) , (int)(te[a].m01/te[a].m00) );
                        printf("defierente\n");
                        a++;
                        if(a==contours.size()) goto LALA;
                    }
                    */
                    if(area[frame1BI.at<cv::Vec3b>((int)(mu[i].m01/mu[i].m00),(int)(mu[i].m10/mu[i].m00) ) [0]]>0 && area[frame1BI.at<cv::Vec3b>((int)(mu[i].m01/mu[i].m00),(int)(mu[i].m10/mu[i].m00))[0] ] <10000){
                        goto PATRICK;
                    }
                    else{
                        goto REILEAO;
                    }
                     PATRICK:;
                    printf("igual\n");
                    //goto LALA;

                    REILEAO:;
                    printf("DIFEEEE\n");
                    //printf("%.2f,%.2f\n", mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
                    putText(frame1,text2,mc[i],0,0.5,Scalar(0,0,255),1,8,false);
                    circle(frame1, mc[i], 30, CV_RGB(50,50,100), 4, CV_AA, 0);
                    patrick++;

                }
        }
        LALA:;
        printf("salvando dados\n");
        FILE *fptr; //ponteiro para arquivo
        fptr=fopen("dados.txt","w");  //w significa arquivo para gravação
        printf("%d<<---\n",frame1BI.at<cv::Vec3b>(310,157)[0]);
        for (int i = 0; i < contours.size()-1; i++)
            {
                printf("save %d\n",i );
                //printf("(%f,%f) -\n", mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00);
                if ((int)mu[i].m01!=0 && (int)mu[i].m01!=0 && (int)mu[i].m00!=0 && frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[0] != 0 && frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[0]!=255 && area[frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[0]] > 90 && mu[i].m01/mu[i].m00 > 200)
                {
                //printf("0\n");
                //printf("%d\n", frame1BI.at<cv::Vec3b>(mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00)[0] );
                circle(frame1BI,cvPoint(((int)mu[i].m10/(int)mu[i].m00),(int)mu[i].m01/(int)mu[i].m00) , 30, CV_RGB(0,250,100), 4, CV_AA, 0);
                printf("area:  %ld- (%d,%d)--(%d,%d,%d)\n",area[frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[0]],((int)mu[i].m10/(int)mu[i].m00),(int)mu[i].m01/(int)mu[i].m00,frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[0],frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[1],frame1PB.at<cv::Vec3b>((int)mu[i].m01/(int)mu[i].m00,((int)mu[i].m10/(int)mu[i].m00)/3)[2]);
                //if (area[frame1BI.at<cv::Vec3b>((int)(mu[i].m01/mu[i].m00),(int)(mu[i].m10/mu[i].m00) ) [0]]>0 && area[frame1BI.at<cv::Vec3b>((int)(mu[i].m01/mu[i].m00),(int)(mu[i].m10/mu[i].m00) ) [0]]<1000000)
                fprintf(fptr, "area:  %ld- (%.1f,%.1f)--(%d,%d,%d)\n",area[frame1BI.at<cv::Vec3b>(mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00)[0]],mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00,frame1BI.at<cv::Vec3b>(mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00)[0],frame1BI.at<cv::Vec3b>(mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00)[1],frame1BI.at<cv::Vec3b>(mu[i].m01/mu[i].m00,mu[i].m10/mu[i].m00)[2]);
                 drawCross(cvPoint(((int)mu[i].m10/(int)mu[i].m00),(int)mu[i].m01/(int)mu[i].m00),Scalar(0,0,255), 10,frame1);
                }
            }
        end_fps();
        printf("salvando img\n");
        imshow( "frame1", frame1 );
        imshow( "temp", frame1TEMP );
        imshow("cocoa",frame1BI);
        imwrite("frame1.png", frame1);
        imwrite("frame1BI.png",frame1BI);

        if(waitKey(30) >= 100) break;
    }
       waitKey(0);
    }
