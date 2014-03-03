#include <time.h>
#include <ctime>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "pthread_clock.h"
using namespace std;

class timer
{
	private:
		bool  started,ended;
		struct timespec time_a,time_b;
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
	clock_gettime(CLOCK_MONOTONIC ,&time_a);
	started = true;
}

double timer::b()
{
	ended=true;
	if(started==true)
	{
	
	clock_gettime(CLOCK_MONOTONIC ,&time_b);

	double sec  = (double)(time_b.tv_sec  - time_a.tv_sec);
	double nano = (double)(time_b.tv_nsec - time_a.tv_nsec)*1E-9;
	
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
	if(ended)
		return timeResut;
	else
		return b();
}
void timer::clear()
{
	time_a.tv_sec=time_b.tv_sec=0;
	time_a.tv_nsec=time_b.tv_nsec=0;
}