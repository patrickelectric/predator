#include "lib.h"
#define PTHREAD_THREADS_MAX   1024  //define o max de threads

//sistema de tracking em testes
#define tracking_low_speed    0     //ativa funcao de tracking em testes //0-1
#define circle_radius         10    //define o valor de raio do circulo de analize
#define diff_percent          6     //define o valor de diff entre a sample e o detectado, caso maior pega novo sample

//debug
#define histogram             0     //ativa janelas de histograma         //0-1
#define print_image_data      1     //ativa print de image data          //0-1
#define print_mouse_data      1     //ativa print de mouse data          //0-1

#define sample_size_pixels    50    //declara o tamanho do sample de amostra (50 default)

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

VideoCapture cap;      // frame capture from camera
float freq_to_analize;

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
        
        for(int idCamera = 3; !cap.open(idCamera); idCamera--) 
        {
			Cerro;
            printf("Erro ao abrir a camera id %d!\n", idCamera);
            if(idCamera == 0)
                return -1;
        }
		
		Image frame;
		while (frame.img.empty()){
			cap >> frame.img;
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


void *image_show( void *)        /*analiza imagem*/
{
    int scale=1;            // mantem tamanho original
    int dbt=30;             // distance_between_text
    float dist_filter=10;   // filtro de area
    float veloc=1;
    float min_fps=4.0;      // fps minimo para aviso de queda de fps
    Size sample_size(sample_size_pixels,sample_size_pixels);    // size of sample
    Size aws(sample_size_pixels*4,sample_size_pixels*4);        // analysis_window_size 
    bool change_sample=true;
    bool mouse_on=false;

    Image frame;            // frame de captura de camera
    Image frameAnalize;     // frame de analize
    Image frameAnalizado;   // frame resultante
    Image result;
    
    Point alvo;             // target coord
    Point alvof;            // target coord  with filter
    Point last_alvo;       

    timer timer_image_show;

    filterOrder1 filter;
    filterOrder1 filterx;
    filterOrder1 filtery;

    while(frame.img.empty())
        cap >> frame.img;

    data_mouse mouseInfo; 
    mouseInfo.x[0]=320;
    mouseInfo.y[0]=320;
    mouseInfo.event=-1;

    while(1)
    {
        /// PARTE DE CAPTURA DA CAMERA E TRATAMENTO DE DADOS PARA EVITAR ERROS DE ANALIZES DO PROGRAMA
        /////////////////////////////////////////////////////////////////////////////////////
        timer_image_show.a();
  		
		cap >> frame.img;

        if(frame.img.empty())
        {
			printf("END OF THE FILM !\n");
			if(pthread_kill(thread_info, 0) == 0)
		  		pthread_cancel(thread_info);
			break;
		}
				
		frame.Flip();
        frame.ScaleImg((float)scale);
        frame.ChangeColour(CV_RGB2GRAY);
        detecCorners(frame.img,frame.img); //futura implementação por contornos

		if(mouseInfo.x[0]>sample_size.width/2 && mouseInfo.y[0]>sample_size.height/2 && mouseInfo.x[0]<frame.img.cols-sample_size.width/2 && mouseInfo.y[0]<frame.img.rows-sample_size.height/2 && mouseInfo.event==EVENT_LBUTTONDOWN)
        {
            change_sample=true;  
            Cerro; printf("Change! \n");
            frameAnalize.PutPiece(frame.img, Point(mouseInfo.x[0]-sample_size.width/2,mouseInfo.y[0]-sample_size.height/2), sample_size);

            filterx.number[0]=filterx.number[1]=alvof.x=mouseInfo.x[0];
            filtery.number[0]=filtery.number[1]=alvof.y=mouseInfo.y[0];
        }
        else if(mouseInfo.event == -1)
        {
            Rect myDim(Point(frame.img.cols/2,frame.img.rows/2),sample_size);
            frameAnalize.PutPiece(frame.img, Point(frame.img.cols/2,frame.img.rows/2), sample_size);  

            filterx.number[0]=filterx.number[1]=alvo.x=alvof.x=frame.img.cols/2;
            filtery.number[0]=filtery.number[1]=alvo.y=alvof.y=frame.img.rows/2;
        }
        #if tracking_low_speed
            else if (alvo.x>sample_size.width/2 && alvo.y>sample_size.height/2 && alvo.x<frame.img.cols-sample_size.width/2 && alvo.y<frame.img.rows-sample_size.height/2 && diffMat(frameAnalizado.img, frameAnalize.img)>diff_percent && DistTwoPoints(alvo,last_alvo)<dist_filter)
            {
                frameAnalize.PutPiece(frame.img, Point(alvo.x-sample_size.width/2, alvo.y-sample_size.height/2), sample_size);
            }
        #endif

        /// PARTE DE FILTROS, MATCHS E CAPTURA DE IMAGENS DO FRAME
        /////////////////////////////////////////////////////////////////////////////////////
        
        /// Create the result matrix
        int result_cols =  frame.img.cols - frameAnalize.img.cols;
        int result_rows =  frame.img.rows - frameAnalize.img.rows;
        result.img.create( result_cols, result_rows, CV_32FC1 );

        Point origem;
        Point matchLoc;
        origem.x=alvof.x-aws.width/2;
        origem.y=alvof.y-aws.height/2;
        
        if(origem.x<0)
            origem.x=0;
        if(origem.y<0)
            origem.y=0;
        

        // to solve some problems with image size
        if(origem.x+aws.width>frame.img.cols) origem.x=frame.img.cols-aws.width;
        if(origem.y+aws.height>frame.img.rows) origem.y=frame.img.rows-aws.height;

        Image  frameReduzido;
        frameReduzido.PutPiece(frame.img, Point(origem.x, origem.y),aws);
        
        Match Match;

        matchLoc = Match.SimpleMatch(frameReduzido.img, frameAnalize.img);

        /// to solve some bugs
        if((alvo.x-sample_size.width/2>0 && alvo.y-sample_size.height/2>0) && (alvo.x+sample_size.width/2<frame.img.cols && alvo.y+sample_size.height/2<frame.img.rows))
        { 
            frameAnalizado.PutPiece(frame.img, Point(alvo.x-sample_size.width/2,alvo.y-sample_size.height/2), sample_size);
            frameAnalizado.GetPiece(frame.img, Point(frame.img.cols-frameAnalizado.img.cols, sample_size.height*1), sample_size);            
        }
        
        Image frameAnalizadoFiltrado;
        if ((alvof.x-sample_size.width/2>0 && alvof.y-sample_size.height/2>0) && (alvof.x+sample_size.width/2<frame.img.cols && alvof.y+sample_size.height/2<frame.img.rows))
        {
            frameAnalizadoFiltrado.PutPiece(frame.img, Point(alvof.x-sample_size.width/2,alvof.y-sample_size.height/2), sample_size);
            frameAnalizadoFiltrado.GetPiece(frame.img, Point(frame.img.cols-frameAnalizadoFiltrado.img.cols, sample_size.height*2), sample_size);
        }

        frameAnalize.GetPiece(frame.img, Point(frame.img.cols-frameAnalize.img.cols, sample_size.height*0), sample_size);
        frameReduzido.GetPiece(frame.img, Point(frame.img.cols-frameReduzido.img.cols, frame.img.rows-frameReduzido.img.rows), Size(frameReduzido.img.cols, frameReduzido.img.rows));
    
        // Translate matchCoord to Point
        if(matchLoc.x+origem.x+sample_size.width/2>0 && matchLoc.x+origem.x+sample_size.width/2<frame.img.cols && matchLoc.y+origem.y+sample_size.height/2>0 && matchLoc.y+origem.y+sample_size.height/2<frame.img.rows)
        {

            if(mouseInfo.event != -1) // primeiro evento
                last_alvo=alvo;

            alvo.x=matchLoc.x+origem.x+sample_size.width/2;
            alvo.y=matchLoc.y+origem.y+sample_size.height/2;

            // filter of distance
            #if tracking_speed
                if(DistTwoPoints(alvo, last_alvo) < dist_filter || mouseInfo.event == 1 ) // se estiver dentro da area ou sample change
                {
                    alvof.x=(int)filterx.filter(alvo.x,timer_image_show.end()*10);
                    alvof.y=(int)filtery.filter(alvo.y,timer_image_show.end()*10);
                }
                else
                {
                    alvo=last_alvo;

                    alvof.x=(int)filterx.filter(last_alvo.x,timer_image_show.end()*10);
                    alvof.y=(int)filtery.filter(last_alvo.y,timer_image_show.end()*10);
                }
            #else
                alvof.x=(int)filterx.filter(alvo.x,timer_image_show.end()*10);
                alvof.y=(int)filtery.filter(alvo.y,timer_image_show.end()*10);
            #endif
        }

        /// PARTE DE DEBUG
        /////////////////////////////////////////////////////////////////////////////////////

        // math erro
        if(alvo.x<0 || alvo.y<0 || alvof.x<0 || alvof.y<0)
        {
            Cerro; printf("MATH ERROR (1)\n");
        }
        if(alvo.x>frame.img.cols || alvo.y>frame.img.rows || alvof.x>frame.img.cols || alvof.y>frame.img.rows)
        {
            Cerro; printf("MATH ERROR (2)\n");
        }

        char str[256];

        Image debug;
        debug.img.create( 500, 400, CV_8UC3);
        debug.img = Scalar(0,0,0);
        
        #if (print_image_data || print_mouse_data) 
            int multi=1;
        #endif
            
        #if print_image_data
            float text_size=1.0;
            sprintf(str, "frame :  %d,%d (x,y)",frame.img.cols,frame.img.rows);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "origem: %d,%d (x,y)",origem.x,origem.y);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "alvo  :  %d,%d (x,y)",alvo.x,alvo.y);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "alvof :  %d,%d (x,y)",alvof.x,alvof.y);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "veloc :  %.2f pixels/sec",veloc);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "timer :  %.4f s",timer_image_show.b());
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "dist_filter :  %.2f pixels",dist_filter);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);

            #if 0
                printf("frame:  %d,%d (x,y)\n",frame.img.cols,frame.img.rows);
                printf("origem: %d,%d (x,y)\n",origem.x,origem.y);
                printf("alvo :  %d,%d (x,y)\n",alvo.x,alvo.y);
                printf("alvof:  %d,%d (x,y)\n",alvof.x,alvof.y);
                printf("veloc:  %.2f pixels/sec\n",veloc);
                printf("timer:  %.4f s\n",timer_image_show.b());
                printf("dist_filter :  %.2f pixels\n",dist_filter);
            #endif

            if(!frameAnalizado.img.empty() || !frameAnalizadoFiltrado.img.empty())
            {
                sprintf(str, "diffF:  %f %%",diffMat(frameAnalizado.img, frameAnalizadoFiltrado.img));
                putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
                
                //printf("diffF:  %f %%\n",diffMat(frameAnalizado.img, frameAnalizadoFiltrado.img));
            }
            if(!frameAnalizado.img.empty() || !frameAnalizadoFiltrado.img.empty())
            {
                sprintf(str, "diff :  %f %%",diffMat(frameAnalizado.img, frameAnalize.img));
                putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);

                //printf("diff :  %f %%\n",diffMat(frameAnalizado.img, frameAnalize.img));
            }
        #endif

        #if print_mouse_data
            sprintf(str, "mouse_click :  %d,%d (x,y)",mouseInfo.x[0],mouseInfo.y[0]);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "mouse       :  %d,%d (x,y)",mouseInfo.x[1],mouseInfo.y[1]);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);
            sprintf(str, "event        :  %d",mouseInfo.event);
            putText(debug.img, str, cvPoint(dbt,dbt*multi++), FONT_HERSHEY_COMPLEX_SMALL, text_size, cvScalar(0,0,255), 1, CV_AA);

            #if 0
                printf("mouse_click :  %d,%d (x,y)\n",mouseInfo.x[0],mouseInfo.y[0]);
                printf("mouse       :  %d,%d (x,y)\n",mouseInfo.x[1],mouseInfo.y[1]);
                printf("event       :  %d\n",mouseInfo.event);
            #endif
        #endif    

        #if (print_image_data || print_mouse_data)    
            debug.SetData(debug.img, "debug", CV_WINDOW_NORMAL);
            debug.Show();
        #endif

        /// Make the image colorful again
        frame.ChangeColour(CV_GRAY2RGB);
        /// make a circle in alvof 
        circle(frame.img, alvof, 3, cvScalar(0,0,255), 1, 8, 0);
        /// make a circle of R = dist of alvof and alvo 
        dist_filter = DistTwoPoints(alvof,alvo) + circle_radius;
        veloc = abs((dist_filter - circle_radius)/timer_image_show.b());
        circle(frame.img, alvo, dist_filter, cvScalar(0,0,255), 1, 8, 0);

        /// Make a simple text to debug
        sprintf(str, "x:%d/y:%d", alvof.x, alvof.y);
        putText(frame.img, str, cvPoint(alvof.x+dbt,alvof.y-dbt), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(0,0,255), 1, CV_AA);

        sprintf(str, "x:%d/y:%d", alvo.x, alvo.y);
        putText(frame.img, str, cvPoint(alvo.x+dbt,alvo.y+dbt), FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(205,201,201), 1, CV_AA);

        if(mouse_on)
        {
            sprintf(str, "MOUSE ON");
            putText(frame.img, str, cvPoint(dbt,dbt*2), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            mouse_on=false;
        }

        if(change_sample)
        {
            sprintf(str, "SAMPLE CHANGED" );
            putText(frame.img, str, cvPoint(dbt,dbt*3), FONT_HERSHEY_COMPLEX_SMALL, 0.6, red, 1, CV_AA);
            change_sample=false;
        }

        freq_to_analize=(float)(1/filter.filter(timer_image_show.b(),5*timer_image_show.b()));        

        sprintf(str, "FPS: %.2f",freq_to_analize);
        putText(frame.img, str, cvPoint(dbt,frame.img.rows-dbt), FONT_HERSHEY_COMPLEX_SMALL, 0.8, red, 1, CV_AA);

        //draw lines     
        line(frame.img, Point (0,alvo.y), Point (frame.img.cols,alvo.y), cvScalar(205,201,201), 1, 8, 0);
        line(frame.img, Point (alvo.x,0), Point (alvo.x,frame.img.rows), cvScalar(205,201,201), 1, 8, 0);

        // modo de histograma, ainda com possivel bugs
        #if histogram
            if(!frameReduzido.img.empty())
            {
                Image histImage;
                frameReduzido.ChangeColour(CV_GRAY2RGB);
                MatHistogram(frameReduzido.img,histImage.img);
                histImage.SetData(histImage.img, "histograma frameReduzido", CV_WINDOW_NORMAL);
                histImage.Show();
                frameReduzido.ChangeColour(CV_RGB2GRAY);
            }

            if(!frameAnalizado.img.empty())
            {
                Image histImage2;
                frameAnalizado.ChangeColour(CV_GRAY2RGB);
                MatHistogram(frameAnalizado.img,histImage2.img);
                histImage2.SetData(histImage2.img, "histograma frameAnalizado", CV_WINDOW_NORMAL);
                histImage2.Show();
                frameAnalizado.ChangeColour(CV_RGB2GRAY);
            }
            
            if(!frameAnalize.img.empty())
            {
                Image histImage3;
                frameAnalize.ChangeColour(CV_GRAY2RGB);
                MatHistogram(frameAnalize.img,histImage3.img);
                histImage3.SetData(histImage3.img, "histograma frameAnalize", CV_WINDOW_NORMAL);
                histImage3.Show();
                frameAnalize.ChangeColour(CV_RGB2GRAY);
            }
        #endif

        frame.SetData(frame.img, "image_show", CV_WINDOW_NORMAL);
        frame.Show();
        mouse_on=frame.mouse.mouse_on;
        mouseInfo.event=frame.mouse.event;

        mouseInfo.x[1]=frame.mouse.x;
        mouseInfo.y[1]=frame.mouse.y;

        if(mouseInfo.event==CV_EVENT_LBUTTONDOWN)
        {
            mouseInfo.x[0]=frame.mouse.x;
            mouseInfo.y[0]=frame.mouse.y;
        }

        // erro in some math loop ou analize
        if(freq_to_analize < min_fps)
        {
            Cerro; printf("ERROR DROP THE BASS\n");
        }
        pthread_cond_signal(&cond);
    }
    Cerro; printf("Image_show Down !\n");
    return NULL;
}