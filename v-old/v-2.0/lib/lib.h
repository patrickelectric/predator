#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "patrickh.h"
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <getopt.h>
#include "timer.h"

using namespace cv;
using namespace std;
using std::vector;

struct data_mouse {
    int event;
    int x;
    int y;   
};
