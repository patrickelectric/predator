#include "../lib/lib.h"
#define PTHREAD_THREADS_MAX 1024    //define o max de threads


/**********************MUTEX*********************/ 
pthread_mutex_t in_frame = PTHREAD_MUTEX_INITIALIZER;    
/************************************************/  

/********************FUNCTIONS*******************/
void *streaming(void *);                //thread que faz a captura de imagens da camera deixando tudo atualizado o quanto poder
void *image_show (void *);                 
/********************FUNCTIONS*******************/

VideoCapture cap(0);

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
        waitKey(30);
        pthread_mutex_unlock(&in_frame);
        Caviso;  printf("Fps do streaming: "); end_fps();
    }
    Cerro; printf("Streaming Down !\n");
    return NULL;
}

void *image_show( void *)        /*analiza imagem*/
{
    while(1)
    {
        Mat frameCopy;
        Mat frameAnalize;
        pthread_mutex_lock(&in_frame);
        frameCopy=frame;
        Rect myDim(100, 100, 200, 200);
        frameAnalize = frameCopy(myDim);     
        pthread_mutex_unlock(&in_frame);

        
        imshow("image_show",frameAnalize);
        namedWindow("image_show", CV_WINDOW_NORMAL); waitKey(30);

        usleep(10);
        
    }
    Cerro; printf("Image_show Down !\n");
    return NULL;
}