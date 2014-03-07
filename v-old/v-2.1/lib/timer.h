#include <time.h>
#include <ctime>
#include <stdio.h>
#include <iostream>
#include <string.h>
using namespace std;

class timer
{
	private:
		bool  started,ended;
		struct timespec time1,time2;
		double timeResut;
		void clear();
		
	public:
		//void init(char *Name);
		timer() : started(false),ended(false) {};
		void a();
		double b();
		double end();
};

void timer::a()
{
	clear();
	clock_gettime(CLOCK_MONOTONIC ,&time1);
	started = true;
}

double timer::b()
{
	ended=true;
	if(started==true)
	{
	
	clock_gettime(CLOCK_MONOTONIC ,&time2);

	double sec  = (double)(time2.tv_sec  - time1.tv_sec);
	double nano = (double)(time2.tv_nsec - time1.tv_nsec)*1E-9;
	
	timeResut = sec + nano;
	
	return timeResut;

	}
	else
	{
		printf("timer not initialized\n");
		return -1;
	}
}

double timer::end()
{
	if(ended==true)
		return timeResut;
	if(ended==false)
		b();
}
void timer::clear()
{
	time1.tv_sec=time1.tv_nsec=time2.tv_sec=time2.tv_nsec=0;
}