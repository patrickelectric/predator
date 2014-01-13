#include "../lib/lib.h"
#define PTHREAD_THREADS_MAX 1024    //define o max de threads


/**********************MUTEX*********************/ 
pthread_mutex_t in_frame = PTHREAD_MUTEX_INITIALIZER;    
pthread_mutex_t in_window = PTHREAD_MUTEX_INITIALIZER;    
/************************************************/  

/********************FUNCTIONS*******************/
void *streaming(void *);                //thread of frame capture
void *image_show (void *);              //thread of frame analize
void *thread_analize (void *);              //thread of frame analize
void CallBackFunc(int event, int x, int y, int flags, void* userdata);    
/********************FUNCTIONS*******************/

VideoCapture cap;
data_mouse mouseInfo;
Mat frame;
bool state=false;

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
        printf("Por default o programa ira selecionar o maior ID de camera\n");

        int idCamera=3;
        
        cap.open(idCamera);
        while(!cap.isOpened()) 
        {

            Cerro;
            printf("Erro ao abrir a camera id %d!\n",idCamera);
            idCamera--;
            if(idCamera==-1)
                return -1;
            cap.release();
            cap.open(idCamera);
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
            printf("Arquivo nao encontrado !\n");
            return -1;
        }
        sleep(1);
    }
    /************************************************/ 

    pthread_t get_img;
    pthread_t show_img;
    pthread_t thread_info;

    pthread_create(&get_img, NULL, streaming , NULL);   //take image from camera or file
    pthread_create(&show_img, NULL, image_show , NULL); //take frame and analize
    pthread_create(&thread_info, NULL, thread_analize , NULL); //analize the threads

    pthread_join(get_img,NULL); 
    pthread_join(show_img,NULL); 
    pthread_join(thread_info,NULL); 


}


void *thread_analize(void *)
{
    //wait signal 1 for 0.1s else print and analize other
    //signal 2
    //signal 3
}



void *streaming( void *)        /*pega imagem da camera ou do arquivo*/
{
    cv::Size s;
    s.width = 640;
    s.height  = 480;
    
    while(1)
    {
        pthread_mutex_lock(&in_frame);
        cap >> frame;

        resize(frame, frame, s);
        cvtColor(frame, frame, CV_RGB2GRAY);
        waitKey(30);
        pthread_mutex_unlock(&in_frame);
        usleep(30);
    }
    Cerro; printf("Streaming Down !\n");
    return NULL;
}


void *image_show( void *)        /*analiza imagem*/
{
    Mat frameCopy;
    Mat frameAnalize;
    Mat result;
    Point alvo;             // target coord
    Point alvof;            // target coord  with filter
    timer timer_image_show;
    filterOrder1 filter;
    filterOrder1 filterx;
    filterOrder1 filtery;
    mouseInfo.event=-1;
    sleep(1);
    while(1)
    {

        timer_image_show.a();
        pthread_mutex_lock(&in_frame);
        frameCopy=frame;
        pthread_mutex_unlock(&in_frame);

        
        if(mouseInfo.x[0] > 26 && mouseInfo.y[0] >26 && mouseInfo.event==EVENT_LBUTTONDOWN)
        {
            Cerro;
            printf("Change! \n");
            Rect myDim(mouseInfo.x[0]-25,mouseInfo.y[0]-25, 50, 50);
            frameAnalize = frameCopy(myDim).clone();     
            frameAnalize.copyTo(frameAnalize);
        }
        else if(mouseInfo.event == -1)
        {
            Rect myDim(frameCopy.cols/2,frameCopy.rows/2, 50, 50);
            alvo.x=alvof.x=frameCopy.cols/2;
            alvo.y=alvof.y=frameCopy.rows/2;
            
            frameAnalize = frameCopy(myDim);     
            frameAnalize.copyTo(frameAnalize);
            mouseInfo.event=-2;
        }
        
        /// Create the result matrix
        int result_cols =  frameCopy.cols - frameAnalize.cols + 1;
        int result_rows = frameCopy.rows - frameAnalize.rows + 1;
        result.create( result_cols, result_rows, CV_32FC1 );

        /// Do the Matching and Normalize
        int match_method=1; //1-5
        Point origem;
        Point origemAbs;
        origem.x=alvof.x-100;
        origem.y=alvof.y-100;
        if(origem.x<0)
        {
            origemAbs.x=abs(origem.x);
            origem.x=0;
        }
        if(origem.y<0)
        {
            origemAbs.y=abs(origem.y);
            origem.y=0;
        }
        Rect Dim(origem.x,origem.y,200+origemAbs.x ,200+origemAbs.y);
        Mat  frameCopyReduzido = frameCopy(Dim).clone();

        matchTemplate( frameCopyReduzido, frameAnalize, result, match_method );
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
        
        /// make a dif with the original and the matched
        Rect myDim2(alvo.x-25,alvo.y-25,50 , 50);
        Mat frameAnalizado = frameCopy(myDim2).clone(); 
        Rect myDim3(alvof.x-25,alvof.y-25,50 , 50);
        Mat frameAnalizadoFiltrado = frameCopy(myDim3).clone(); 

        /// cut the image to make something more.... cool
        Rect roi1( Point( frameCopy.cols-frameAnalize.cols, 0 ), frameAnalize.size() );
        frameAnalize.copyTo( frameCopy( roi1 ) );
        Rect roi2( Point( frameCopy.cols-frameAnalizado.cols, 50 ), frameAnalizado.size() );
        frameAnalizado.copyTo( frameCopy( roi2 ) );
        Rect roi3( Point( frameCopy.cols-frameAnalizadoFiltrado.cols, 100 ), frameAnalizadoFiltrado.size() );
        frameAnalizadoFiltrado.copyTo( frameCopy( roi3 ) );
        Rect roi4( Point( frameCopy.cols-frameCopyReduzido.cols, frameCopy.rows-frameCopyReduzido.rows ), frameCopyReduzido.size() );
        frameCopyReduzido.copyTo( frameCopy( roi4 ) );

        // Translate matchCoord to Point
        alvo.x=matchLoc.x+origem.x+25;
        alvo.y=matchLoc.y+origem.y+25;
        alvof.x=filterx.filter(alvo.x,timer_image_show.end()*4);
        alvof.y=filtery.filter(alvo.y,timer_image_show.end()*4);

        // math erro
        if(alvo.x<0 || alvo.y<0 || alvof.x<0 || alvof.y<0)
        {
            Cerro; printf("MATH ERROR (1)\n");
        }
        if(alvo.x>frameCopy.cols || alvo.y>frameCopy.rows || alvof.x>frameCopy.cols || alvof.y>frameCopy.rows)
        {
            Cerro; printf("MATH ERROR (2)\n");
        }

        /// Make the image colorful again
        cvtColor(frameCopy, frameCopy, CV_GRAY2RGB);

        /// make retangles or circles
        #if 0
            rectangle( frameCopy, matchLoc, Point( matchLoc.x + frameAnalize.cols , matchLoc.y + frameAnalize.rows ), Scalar::all(0), 2, 8, 0 );
            rectangle( result, matchLoc, Point( matchLoc.x + frameAnalize.cols , matchLoc.y + frameAnalize.rows ), Scalar::all(0), 2, 8, 0 );
        #else
            circle(frameCopy, alvof, 3, cvScalar(0,0,255), 1, 8, 0);
        #endif
        
        /// Make a simple text to debug
        char str[256];
        sprintf(str, "x:%d/y:%d", alvof.x, alvof.y);
        putText(frameCopy, str, cvPoint(alvof.x+30,alvof.y-30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0,0,255), 1, CV_AA);

        sprintf(str, "x:%d/y:%d", alvo.x, alvo.y);
        putText(frameCopy, str, cvPoint(alvo.x+30,alvo.y+30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(205,201,201), 1, CV_AA);

        sprintf(str, "maxVal:%.8f/minVal:%.8f", maxVal, minVal);
        putText(frameCopy, str, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(0,100,0), 1, CV_AA);

        //draw lines     
        line(frameCopy, Point (0,alvo.y), Point (frameCopy.cols,alvo.y), cvScalar(205,201,201), 1, 8, 0);
        line(frameCopy, Point (alvo.x,0), Point (alvo.x,frameCopy.rows), cvScalar(205,201,201), 1, 8, 0);

        imshow("image_show",frameCopy);
        namedWindow("image_show", CV_WINDOW_NORMAL); 
        setMouseCallback("image_show", CallBackFunc, NULL);
        
        Caviso;  printf("Fps do image_show: %.2f\n",1/filter.filter(timer_image_show.b(),5*timer_image_show.b())); //end_fps();
        Caviso;  printf("tempo de image_show: %f s \n",timer_image_show.b());

        // erro in some math loop ou analize
        if(1/filter.filter(timer_image_show.b(),5*timer_image_show.b())<4)
        {
            Cerro; printf("ERROR DROP THE BASS\n");
        }
        waitKey(30);
        //pthread_mutex_unlock(&in_window);
        
    }
    Cerro; printf("Image_show Down !\n");
    return NULL;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    printf("callback\n");
    mouseInfo.event=event;
    switch( event )
    {
        case CV_EVENT_MOUSEMOVE:
            if(state==true)
            {
                mouseInfo.x[1]=x;
                mouseInfo.x[1]=y;
            }
        break;

        case CV_EVENT_LBUTTONDOWN:
            mouseInfo.x[0]=x;
            mouseInfo.y[0]=y;
            state=true;
        break;

        case CV_EVENT_LBUTTONUP:
            mouseInfo.x[1]=x;
            mouseInfo.y[1]=y;
            state=false;
        break;
    }
}