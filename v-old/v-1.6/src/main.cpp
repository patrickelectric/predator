#include "../lib/lib.h"
#define PTHREAD_THREADS_MAX 1024    //define o max de threads


/**********************MUTEX*********************/ 
pthread_mutex_t in_frame = PTHREAD_MUTEX_INITIALIZER;    
pthread_mutex_t in_mouseInfo = PTHREAD_MUTEX_INITIALIZER;    
/************************************************/  

/********************FUNCTIONS*******************/
void *streaming(void *);                //thread que faz a captura de imagens da camera deixando tudo atualizado o quanto poder
void *image_show (void *);             
void CallBackFunc(int event, int x, int y, int flags, void* userdata);    
/********************FUNCTIONS*******************/

VideoCapture cap(0);
data_mouse mouseInfo;
Mat frame;

int main(int argc, char *argv[])
{
    start_fps();
    sleep(1);

    /*********************PARAMETROS*****************/  
    if(argc<2) 
    {   
        Cwarning;
        printf("Nenhum argumento adicionado ao programa\n");
        Cwarning;
        printf("Por default sera pego a imagem da camera com id (0)\n");

        if(!cap.isOpened()) 
        {
            Cerro;
            printf("Erro ao abrir a camera !\n");
            return -1;
        }
        sleep(1);
    }
    else
    {
        char *local_video;      
        local_video=argv[1];
        Cok;
        printf("Video ! %s ! escolhido pelo usuario\n",local_video);
        cap.open(local_video);
        if(!cap.isOpened())
        {
            Cerro;
            printf("Arquivo não encontrado !\n");
            return -1;
        }
        sleep(1);
    }
    /************************************************/ 

    pthread_t get_img;
    pthread_t show_img;

    pthread_create(&get_img, NULL, streaming , NULL); //pega imagem da camera ou do arquivo
    pthread_create(&show_img, NULL, image_show , NULL); //pega imagem da camera ou do arquivo

    pthread_join(get_img,NULL); 
    pthread_join(show_img,NULL); 


}


void *streaming( void *)        /*pega imagem da camera ou do arquivo*/
{
    while(1)
    {
        pthread_mutex_lock(&in_frame);
        cap >> frame;
        imshow("frame",frame);
        namedWindow("frame", CV_WINDOW_NORMAL);
        setMouseCallback("frame", CallBackFunc, NULL);
        waitKey(30);
        pthread_mutex_unlock(&in_frame);
        Caviso;  printf("Fps do streaming: "); end_fps();
    }
    Cerro; printf("Streaming Down !\n");
    return NULL;
}


void *image_show( void *)        /*analiza imagem*/
{
    Mat frameCopy;
    Mat frameAnalize;
    Mat result;
    mouseInfo.event=-1;
    while(1)
    {
        pthread_mutex_lock(&in_frame);
        frameCopy=frame;
        pthread_mutex_unlock(&in_frame);

        pthread_mutex_lock(&in_mouseInfo);
        if(mouseInfo.x > 100 && mouseInfo.y >100 && mouseInfo.event==EVENT_LBUTTONDOWN)
        {
            Cerro;
            printf("Change! \n");
            Rect myDim(mouseInfo.x-25,mouseInfo.y-25, 50, 50);
            frameAnalize = frameCopy(myDim).clone();     
            frameAnalize.copyTo(frameAnalize);
        }
        else if(mouseInfo.event == -1)
        {
            Rect myDim(100,100, 50, 50);
            frameAnalize = frameCopy(myDim);     
            frameAnalize.copyTo(frameAnalize);
            mouseInfo.event=-2;
        }
        pthread_mutex_unlock(&in_mouseInfo);

        /// Create the result matrix
        int result_cols =  frameCopy.cols - frameAnalize.cols + 1;
        int result_rows = frameCopy.rows - frameAnalize.rows + 1;
        result.create( result_cols, result_rows, CV_32FC1 );

        /// Do the Matching and Normalize
        int match_method=1; //1-5
        matchTemplate( frameCopy, frameAnalize, result, match_method );
        normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

        /// Localizing the best match with minMaxLoc
        double minVal; double maxVal; Point minLoc; Point maxLoc;
        Point matchLoc;
        minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

        /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
        if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
            { matchLoc = minLoc; }
        else
            { matchLoc = maxLoc; }
        
        /// Show me what you got
        rectangle( frameCopy, matchLoc, Point( matchLoc.x + frameAnalize.cols , matchLoc.y + frameAnalize.rows ), Scalar::all(0), 2, 8, 0 );
        rectangle( result, matchLoc, Point( matchLoc.x + frameAnalize.cols , matchLoc.y + frameAnalize.rows ), Scalar::all(0), 2, 8, 0 );

        /// make a dif with the original and the matched
        Rect myDim2(matchLoc.x,matchLoc.y,50 , 50);
        Mat frameAnalizado = frameCopy(myDim2).clone(); 
        Mat subt = frameAnalize - frameAnalizado;

        /// Make a simple text to debug
        char str[256];
        sprintf(str, "x:%d/y:%d", matchLoc.x, matchLoc.y);
        putText(frameCopy, str, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);

        sprintf(str, "maxVal:%.8f/minVal:%.8f", maxVal, minVal);
        putText(frameCopy, str, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(200,200,250), 1, CV_AA);

        /// Show de imgs
        imshow("image_show",frameCopy);
        namedWindow("image_show", CV_WINDOW_NORMAL); waitKey(30);
        imshow("analize",frameAnalize);
        namedWindow("analize", CV_WINDOW_NORMAL); waitKey(30);
        imshow("result",result);
        namedWindow("result", CV_WINDOW_NORMAL); waitKey(30);
        imshow("analizado",frameAnalizado);
        namedWindow("analizado", CV_WINDOW_NORMAL); waitKey(30);
        imshow("sub",subt);
        namedWindow("sub", CV_WINDOW_NORMAL); waitKey(30);

        usleep(10);
        
    }
    Cerro; printf("Image_show Down !\n");
    return NULL;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
     if(event==EVENT_LBUTTONDOWN || event==EVENT_MOUSEMOVE)
     {
         pthread_mutex_lock(&in_mouseInfo);
         mouseInfo.event=event;
         mouseInfo.x=x;
         mouseInfo.y=y;
         pthread_mutex_unlock(&in_mouseInfo);
     }
}