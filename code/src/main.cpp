#include "lib.h"
#define PTHREAD_THREADS_MAX 1024    //define o max de threads


/**********************MUTEX*********************/ 
/*Frames*/
pthread_mutex_t in_window = PTHREAD_MUTEX_INITIALIZER;

/*Conditions and global variables */
pthread_mutex_t mutex_freq_image_show;
pthread_cond_t cond;

/*Threads*/
pthread_t show_img;
pthread_t thread_info; 
/************************************************/  

/********************FUNCTIONS*******************/
void *image_show (void *);              //thread of frame analize
void *thread_analize (void *);              //thread of frame analize
void CallBackFunc(int event, int x, int y, int flags, void* userdata);    
/********************FUNCTIONS*******************/

data_mouse mouseInfo;  
VideoCapture cap;      // frame capture from camera
float freq_to_analize;
bool mouse_on=false;
bool change_sample=false;


int main(int argc, char *argv[])
{
    start_fps();

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
    }
    /************************************************/ 

	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex_freq_image_show, NULL);

	pthread_create(&show_img, NULL, image_show , NULL); //take frame and analize
	pthread_create(&thread_info, NULL, thread_analize , NULL); //analize the threads

    pthread_join(show_img,NULL); 
    pthread_join(thread_info,NULL); 
}


void *thread_analize(void *)
{
    struct timeval  now;        //tv_sec tv_usec
    struct timespec timeout;    //tv_sec tv_nsec
	float freq = 0;
    while(1)
    {
        gettimeofday(&now, NULL);

        timeout.tv_sec = now.tv_sec + 1;
        timeout.tv_nsec = (now.tv_usec)*1000;

        if(pthread_cond_timedwait(&cond, &mutex_freq_image_show, &timeout))
            {Cerro; printf("TIMEOUT show\n");}
        else
            freq=freq_to_analize;

        Caviso;  printf("Freq de image_show: %.2f\n",freq); //end_fps();
    }
    Cerro; printf("Thread_analize Down !\n");
    return NULL;
}



bool live=false;
void *image_show( void *)        /*analiza imagem*/
{
	/////
	Size s;
    s.width = 640;
    s.height  = 480;
	Mat frame;
	/////

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
  		
		cap >> frame;
		
        if(frame.empty()){
			if (live){
				printf("END OF THE FILM !\n");
				if(pthread_kill(thread_info, 0) == 0)
			  		pthread_cancel(thread_info);
				break;
			}
  			continue;
		}
		
		live = true;
				
		flip(frame,frame,1);
        resize(frame, frame, s);
        cvtColor(frame, frame, CV_RGB2GRAY);
        detecCorners(frame,frame); //futura implementação por contornos

		if(mouseInfo.x[0]>25 && mouseInfo.y[0]>25 && mouseInfo.x[0]<frame.cols-25 && mouseInfo.y[0]<frame.rows-25 && mouseInfo.event==EVENT_LBUTTONDOWN)
        {
            change_sample=true;
            Cerro; printf("Change! \n");
            Rect myDim(mouseInfo.x[0]-25,mouseInfo.y[0]-25, 50, 50);
            frameAnalize = frame(myDim).clone();     
            frameAnalize.copyTo(frameAnalize);

            filterx.number[0]=filterx.number[1]=alvof.x=mouseInfo.x[0];
            filtery.number[0]=filtery.number[1]=alvof.y=mouseInfo.y[0];
        }
        else if(mouseInfo.event == -1)
        {
            Rect myDim(frame.cols/2,frame.rows/2, 50, 50);
            filterx.number[0]=filterx.number[1]=alvo.x=alvof.x=frame.cols/2;
            filtery.number[0]=filtery.number[1]=alvo.y=alvof.y=frame.rows/2;
            
            frameAnalize = frame(myDim);     
            frameAnalize.copyTo(frameAnalize);
            mouseInfo.event=-2;
        }
        
        /// Create the result matrix
        int result_cols =  frame.cols - frameAnalize.cols + 1;
        int result_rows = frame.rows - frameAnalize.rows + 1;
        result.create( result_cols, result_rows, CV_32FC1 );

        /// Do the Matching and Normalize
        int match_method=1; //1-5
        Point origem;
        origem.x=alvof.x-50;
        origem.y=alvof.y-50;
        if(origem.x<0)
            origem.x=alvo.x;
        if(origem.y<0)
            origem.y=alvo.y;

        // to solve some problems with image size
        if(origem.x+200>frame.cols) origem.x=frame.cols-200;
        if(origem.y+200>frame.rows) origem.y=frame.rows-200;

        Rect Dim(origem.x,origem.y,200,200);
        Mat  frameReduzido = frame(Dim).clone();
        
        matchTemplate( frameReduzido, frameAnalize, result, match_method );
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
        Rect myDim2(alvo.x-25,alvo.y-25,50 , 50);
        Mat frameAnalizado = frame(myDim2).clone();
        if((alvo.x-25>0 && alvo.y-25>0) && (alvo.x+25<frame.cols && alvo.y+25<frame.rows))
        { 
            Rect roi2( Point( frame.cols-frameAnalizado.cols, 50 ), frameAnalizado.size() );
            frameAnalizado.copyTo( frame( roi2 ) );
            
        }
        
        Rect myDim3(alvof.x-25,alvof.y-25,50 , 50);
        Mat frameAnalizadoFiltrado = frame(myDim3).clone(); 
        if ((alvof.x-25>0 && alvof.y-25>0) && (alvof.x+25<frame.cols && alvof.y+25<frame.rows))
        {
            Rect roi3( Point( frame.cols-frameAnalizadoFiltrado.cols, 100 ), frameAnalizadoFiltrado.size() );
            frameAnalizadoFiltrado.copyTo( frame( roi3 ) );
        }
        
        Rect roi1( Point( frame.cols-frameAnalize.cols, 0 ), frameAnalize.size() );
        frameAnalize.copyTo( frame( roi1 ) );
        
        Rect roi4( Point( frame.cols-frameReduzido.cols, frame.rows-frameReduzido.rows ), frameReduzido.size() );
        frameReduzido.copyTo( frame( roi4 ) );
    
        // Translate matchCoord to Point
        if(matchLoc.x+origem.x+25>0 && matchLoc.x+origem.x+25<frame.cols && matchLoc.y+origem.y+25>0 && matchLoc.y+origem.y+25<frame.rows)
        {
            alvo.x=matchLoc.x+origem.x+25;
            alvo.y=matchLoc.y+origem.y+25;
            alvof.x=(int)filterx.filter(alvo.x,timer_image_show.end()*4);
            alvof.y=(int)filtery.filter(alvo.y,timer_image_show.end()*4);
        }

        // math erro
        if(alvo.x<0 || alvo.y<0 || alvof.x<0 || alvof.y<0)
        {
            Cerro; printf("MATH ERROR (1)\n");
        }
        if(alvo.x>frame.cols || alvo.y>frame.rows || alvof.x>frame.cols || alvof.y>frame.rows)
        {
            Cerro; printf("MATH ERROR (2)\n");
        }

        #if 1
            printf("frame:  %d,%d\n",frame.cols,frame.rows);
            printf("origem: %d,%d\n",origem.x,origem.y);
            printf("alvo :  %d,%d\n",alvo.x,alvo.y);
            printf("alvof:  %d,%d\n",alvof.x,alvof.y);
            printf("diff :  %f\n",diffMat(frameAnalizado, frameAnalizadoFiltrado));
        #endif

        /// Make the image colorful again
        cvtColor(frame, frame, CV_GRAY2RGB);

        /// make a circle in alvof 
        circle(frame, alvof, 3, cvScalar(0,0,255), 1, 8, 0);
        /// Make a simple text to debug
        char str[256];
        sprintf(str, "x:%d/y:%d", alvof.x, alvof.y);
        putText(frame, str, cvPoint(alvof.x+30,alvof.y-30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0,0,255), 1, CV_AA);

        sprintf(str, "x:%d/y:%d", alvo.x, alvo.y);
        putText(frame, str, cvPoint(alvo.x+30,alvo.y+30), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(205,201,201), 1, CV_AA);

        sprintf(str, "maxVal:%.8f/minVal:%.8f", maxVal, minVal);
        putText(frame, str, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(0,100,0), 1, CV_AA);

        if(mouse_on)
        {
            sprintf(str, "MOUSE ON");
            putText(frame, str, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            mouse_on=false;
        }

        if(change_sample)
        {
            sprintf(str, "SAMPLE CHANGED"   );
            putText(frame, str, cvPoint(30,90), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            change_sample=false;
        }
        freq_to_analize=(float)(1/filter.filter(timer_image_show.b(),5*timer_image_show.b()));        

        sprintf(str, "FPS: %.2f",freq_to_analize);
        putText(frame, str, cvPoint(30,frame.rows-30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, red, 1, CV_AA);

        //draw lines     
        line(frame, Point (0,alvo.y), Point (frame.cols,alvo.y), cvScalar(205,201,201), 1, 8, 0);
        line(frame, Point (alvo.x,0), Point (alvo.x,frame.rows), cvScalar(205,201,201), 1, 8, 0);

        imshow("image_show",frame);
        namedWindow("image_show", CV_WINDOW_NORMAL); 
        setMouseCallback("image_show", CallBackFunc, NULL);
        // erro in some math loop ou analize
        if(freq_to_analize < 4.0)
        {
            Cerro; printf("ERROR DROP THE BASS\n");
        }
        waitKey(30);
        pthread_cond_signal(&cond);
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