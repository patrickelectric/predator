#include "../lib/lib.h"
#define PTHREAD_THREADS_MAX 1024    //define o max de threads


/**********************MUTEX*********************/ 
/*Frames*/
pthread_mutex_t in_frame = PTHREAD_MUTEX_INITIALIZER;    
pthread_mutex_t in_window = PTHREAD_MUTEX_INITIALIZER;

/*Conditions and global variables */
pthread_mutex_t mutex_freq_streaming = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_freq_image_show = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond[2] = PTHREAD_COND_INITIALIZER;   
/************************************************/  

/********************FUNCTIONS*******************/
void *streaming(void *);                //thread of frame capture
void *image_show (void *);              //thread of frame analize
void *thread_analize (void *);              //thread of frame analize
void CallBackFunc(int event, int x, int y, int flags, void* userdata);    
/********************FUNCTIONS*******************/

data_mouse mouseInfo;  
VideoCapture cap;      // frame capture from camera
Mat frame;             // the global image of camera
float freq_to_analize[2];
bool mouse_on=false;
bool change_sample=false;

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
    struct timeval  now;        //tv_sec tv_usec
    struct timespec timeout;    //tv_sec tv_nsec
    float freq[2];
    while(1)
    {
        gettimeofday(&now, NULL);

        timeout.tv_sec = now.tv_sec + 1;
        timeout.tv_nsec = (now.tv_usec)*1E3;

        if(pthread_cond_timedwait(&cond[0], &mutex_freq_streaming, &timeout))
            printf("TIMEOUT streaming\n");
        else
            freq[0]=freq_to_analize[0];

        timeout.tv_sec = timeout.tv_sec + 1;
        if(pthread_cond_timedwait(&cond[1], &mutex_freq_image_show, &timeout))
            printf("TIMEOUT show\n");
        else
            freq[1]=freq_to_analize[1];

        Caviso;  printf("Freq de streaming: %.2f\n",freq[0]); //end_fps();
        Caviso;  printf("Freq de image_show: %.2f\n",freq[1]); //end_fps();
    }
}



void *streaming( void *)        /*pega imagem da camera ou do arquivo*/
{
    Size s;
    s.width = 640;
    s.height  = 480;
    timer timer_streaming;
    filterOrder1 filter;
    while(1)
    {
        timer_streaming.a();
        pthread_mutex_lock(&in_frame);
        cap >> frame;
        if(frame.empty())
            printf("END OF THE FILM !\n");
        resize(frame, frame, s);
        cvtColor(frame, frame, CV_RGB2GRAY);
        pthread_mutex_unlock(&in_frame);
        usleep(1000);
        freq_to_analize[0] = 1/filter.filter(timer_streaming.b(),5*timer_streaming.b());
        pthread_cond_signal(&cond[0]);

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
    while(1)
    {
        timer_image_show.a();
        pthread_mutex_lock(&in_frame);
        frameCopy=frame;
        pthread_mutex_unlock(&in_frame);
        
        if(mouseInfo.x[0]>25 && mouseInfo.y[0]>25 && mouseInfo.x[0]<frameCopy.cols-25 && mouseInfo.y[0]<frameCopy.rows-25 && mouseInfo.event==EVENT_LBUTTONDOWN)
        {
            change_sample=true;
            Cerro; printf("Change! \n");
            Rect myDim(mouseInfo.x[0]-25,mouseInfo.y[0]-25, 50, 50);
            frameAnalize = frameCopy(myDim).clone();     
            frameAnalize.copyTo(frameAnalize);

            filterx.number[0]=filterx.number[1]=alvof.x=mouseInfo.x[0];
            filtery.number[0]=filtery.number[1]=alvof.y=mouseInfo.y[0];
        }
        else if(mouseInfo.event == -1)
        {
            Rect myDim(frameCopy.cols/2,frameCopy.rows/2, 50, 50);
            filterx.number[0]=filterx.number[1]=alvo.x=alvof.x=frameCopy.cols/2;
            filtery.number[0]=filtery.number[1]=alvo.y=alvof.y=frameCopy.rows/2;
            
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

        // to solve some problems with image size
        if(origem.x+200>frameCopy.cols) origem.x=frameCopy.cols-200;
        if(origem.y+200>frameCopy.rows) origem.y=frameCopy.rows-200;
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
        
        /// to solve some bugs
        if((alvo.x-25>0 && alvo.y-25>0) && (alvo.x+25<frameCopy.cols && alvo.y+25<frameCopy.rows))
        {
            Rect myDim2(alvo.x-25,alvo.y-25,50 , 50);
            Mat frameAnalizado = frameCopy(myDim2).clone(); 
            Rect roi2( Point( frameCopy.cols-frameAnalizado.cols, 50 ), frameAnalizado.size() );
            frameAnalizado.copyTo( frameCopy( roi2 ) );
            
        }
        
        if ((alvof.x-25>0 && alvof.y-25>0) && (alvof.x+25<frameCopy.cols && alvof.y+25<frameCopy.rows))
        {
            Rect myDim3(alvof.x-25,alvof.y-25,50 , 50);
            Mat frameAnalizadoFiltrado = frameCopy(myDim3).clone(); 
            Rect roi3( Point( frameCopy.cols-frameAnalizadoFiltrado.cols, 100 ), frameAnalizadoFiltrado.size() );
            frameAnalizadoFiltrado.copyTo( frameCopy( roi3 ) );
        }
        
        Rect roi1( Point( frameCopy.cols-frameAnalize.cols, 0 ), frameAnalize.size() );
        frameAnalize.copyTo( frameCopy( roi1 ) );
        
        Rect roi4( Point( frameCopy.cols-frameCopyReduzido.cols, frameCopy.rows-frameCopyReduzido.rows ), frameCopyReduzido.size() );
        frameCopyReduzido.copyTo( frameCopy( roi4 ) );
    
        // Translate matchCoord to Point
        if(matchLoc.x+origem.x+25>0 && matchLoc.x+origem.x+25<frameCopy.cols && matchLoc.y+origem.y+25>0 && matchLoc.y+origem.y+25<frameCopy.rows)
        {
            alvo.x=matchLoc.x+origem.x+25;
            alvo.y=matchLoc.y+origem.y+25;
            alvof.x=filterx.filter(alvo.x,timer_image_show.end()*4);
            alvof.y=filtery.filter(alvo.y,timer_image_show.end()*4);
        }

        // math erro
        if(alvo.x<0 || alvo.y<0 || alvof.x<0 || alvof.y<0)
        {
            Cerro; printf("MATH ERROR (1)\n");
        }
        if(alvo.x>frameCopy.cols || alvo.y>frameCopy.rows || alvof.x>frameCopy.cols || alvof.y>frameCopy.rows)
        {
            Cerro; printf("MATH ERROR (2)\n");
        }

        #if 0
            printf("framecopy: %d,%d\n",frameCopy.cols,frameCopy.rows);
            printf("origem: %d,%d\n",origem.x,origem.y);
            printf("alvo: %d,%d\n",alvo.x,alvo.y);
            printf("alvof: %d,%d\n",alvof.x,alvof.y);
            printf("origemABS: %d,%d\n",origemAbs.x,origemAbs.y);
        #endif

        /// Make the image colorful again
        cvtColor(frameCopy, frameCopy, CV_GRAY2RGB);

        /// make a circle in alvof 
        circle(frameCopy, alvof, 3, cvScalar(0,0,255), 1, 8, 0);
        /// Make a simple text to debug
        char str[256];
        sprintf(str, "x:%d/y:%d", alvof.x, alvof.y);
        putText(frameCopy, str, cvPoint(alvof.x+30,alvof.y-30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0,0,255), 1, CV_AA);

        sprintf(str, "x:%d/y:%d", alvo.x, alvo.y);
        putText(frameCopy, str, cvPoint(alvo.x+30,alvo.y+30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(205,201,201), 1, CV_AA);

        sprintf(str, "maxVal:%.8f/minVal:%.8f", maxVal, minVal);
        putText(frameCopy, str, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(0,100,0), 1, CV_AA);

        if(mouse_on)
        {
            sprintf(str, "MOUSE ON");
            putText(frameCopy, str, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            mouse_on=false;
        }

        if(change_sample)
        {
            sprintf(str, "SAMPLE CHANGED"   );
            putText(frameCopy, str, cvPoint(30,90), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            change_sample=false;
        }
        freq_to_analize[1]=1/filter.filter(timer_image_show.b(),5*timer_image_show.b());        

        sprintf(str, "FPS: %.2f",freq_to_analize[1]);
        putText(frameCopy, str, cvPoint(30,frameCopy.rows-30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, red, 1, CV_AA);

        //draw lines     
        line(frameCopy, Point (0,alvo.y), Point (frameCopy.cols,alvo.y), cvScalar(205,201,201), 1, 8, 0);
        line(frameCopy, Point (alvo.x,0), Point (alvo.x,frameCopy.rows), cvScalar(205,201,201), 1, 8, 0);

        imshow("image_show",frameCopy);
        namedWindow("image_show", CV_WINDOW_NORMAL); 
        setMouseCallback("image_show", CallBackFunc, NULL);
        // erro in some math loop ou analize
        if(freq_to_analize[1] < 4.0)
        {
            Cerro; printf("ERROR DROP THE BASS\n");
        }
        waitKey(30);
        pthread_cond_signal(&cond[1]);
    }
    Cerro; printf("Image_show Down !\n");
    return NULL;
}

bool state=false;
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    mouse_on=true;
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