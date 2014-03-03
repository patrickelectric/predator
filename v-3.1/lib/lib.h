#ifdef UNIX
  //#include <time.h>
  #include <unistd.h>   /* UNIX standard function definitions */
  #include <sys/time.h>
  #include <termios.h>  /* POSIX terminal control definitions */
  #include <sys/ioctl.h>
  #include <getopt.h>
#else // WIN32
  #include "pthread.h"
  #include <windows.h>
  #include "gettime.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "patrickh.h"
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */

#include "filter.h"

using namespace cv;
using namespace std;
using std::vector;

struct data_mouse {
    int event;
    int x[2];
    int y[2];   
};
