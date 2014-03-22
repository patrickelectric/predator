#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

class Image 
{
  private:
    /// this function its only possivel with the codes of https://github.com/mlkimg
    static void mouseCallback(int event, int x, int y, int flags, void *param);
    void doMouseCallback(int event, int x, int y, int flags);

  public:
    struct Mouse{int x; int y; int event; int flag; bool mouse_on;};
    Mouse mouse;

    Mat img;
    char* window_name;
    int window_flag; //WINDOW_NORMAL - WINDOW_AUTOSIZE - WINDOW_OPENGL

    void SetData(char* window_name_sample); 
    void SetData(char* window_name_sample ,int window_flag_sample); 
    void SetData(Mat img_sample,char* window_name_sample ,int window_flag_sample);
    void ScaleImg(float scale);
    void Flip();
    void ChangeColour(int code); //CV_BGR2GRAY, CV_RGB2GRAY, CV_GRAY2BGR, CV_GRAY2RGB, CV_BGR2GRAY, CV_RGB2GRAY, CV_GRAY2BGR, CV_GRAY2RGB
    void PutPiece(Mat src, int x,int y,int cols, int rows);
    void PutPiece(Mat src, Point point, Size size);
    void GetPiece(Mat& piece, int x,int y,int cols, int rows);
    void GetPiece(Mat& piece, Point point, Size size);
    char Show();
    void Destroy();
};

Image* ins;

void Image::SetData(char* window_name_sample)
{
    ins=this;
    window_name=window_name_sample;
    window_flag=CV_WINDOW_NORMAL;
}

void Image::SetData(char* window_name_sample ,int window_flag_sample)
{
    
    ins=this;
    window_name=window_name_sample;
    window_flag=window_flag_sample;
}

void Image::SetData(Mat img_sample,char* window_name_sample ,int window_flag_sample)
{
    ins=this;
    img=img_sample;
    window_name=window_name_sample;
    window_flag=window_flag_sample;
}

void Image::ScaleImg(float scale)
{
    resize(img, img, Size((int)(img.cols*scale),(int)(img.rows*scale)));
}

void Image::Flip()
{
    flip(img,img,1);
}

void Image::ChangeColour(int code)
{
    cvtColor(img, img, code);
}

void Image::PutPiece(Mat src, int x,int y,int cols, int rows)
{
    Rect Dim(x,y, cols, rows);
    img = src(Dim).clone();     
}

void Image::PutPiece(Mat src, Point point, Size size)
{
    Rect Dim(point.x, point.y, size.width, size.height);
    img = src(Dim).clone();
}

void Image::GetPiece(Mat& piece, int x,int y,int cols, int rows)
{
    Rect Dim(x, y, cols, rows);
    img.copyTo(piece(Dim));
}

void Image::GetPiece(Mat& piece, Point point, Size size)
{
    Rect Dim(point.x, point.y, size.width, size.height);
    img.copyTo(piece(Dim));
}

char Image::Show()
{
    mouse.mouse_on=false;
    namedWindow(window_name,window_flag); 
    imshow(window_name,img);
    setMouseCallback(window_name, mouseCallback, this);
    return waitKey(1);
}

void Image::Destroy()
{
    destroyWindow("frame");
}

void Image::mouseCallback(int event, int x, int y, int flags, void *param)
{
    Image& self = *((Image*)param); 
    self.mouse.x=x;
    self.mouse.y=y;
    self.mouse.event=event;
    self.mouse.flag=flags;
    self.mouse.mouse_on=true;
}