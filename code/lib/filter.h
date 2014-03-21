#include "timer.h"
// adicione o caminho para a biblioteca filter, para o funcionamento da biblioteca

class filterOrder1
{
	private:
		timer timer1;
		bool  started;
		
	public:
		double number[2];
		filterOrder1() : started(false){};
		double filter(double,double);
};

double filterOrder1::filter(double x,double filterTime)
{
	if(started==false) 
	{
		timer1.a();
		number[0]=x; number[1]=0;
		started=true;
		return number[0];
	}
	else
		number[0]=x;
	timer1.b();
	//printf("timer end: %f\n",timer1.end() );
	number[1]=number[1]*( 1-timer1.end()/(filterTime+timer1.end()))+(timer1.end()/(filterTime+timer1.end()))*number[0];
	timer1.a();
	return number[1];
}