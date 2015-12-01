
int dummy(int x)

{
  int i,j;
  i = x;
  j =  i*i;
  //j++;
  return   j ;
}

int __attribute__((naked)) dummy_naked()

{
	int x1;int x2;int x3;int x4;int x5;int x6;int x7;int x8;int x9;int x10;int x11;int x12;int x13;int x14;int x15;int x16;
	x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=x13=x14=x15=x16 = 6;
	int s=  x1+x2+x3+x4+x5+x6+x7+x8+x9+x10+x11+x12+x13+x14+x15+x16;
	s= s;
  	int i,j;
  	i = x1;
  	j =  i*i;
  	//j++;
  	return  j  ;
}

int div(int 	dividend,   	int	divisor)
{

  int     result = 0;

  int    remainder = dividend;

  while
    (remainder >= divisor) {

    result++;

    remainder -= divisor;

  }

  return    result;

}


int compute_volume(	       int	       rad)
{

  int    rad3 = rad * rad * rad;

  return    div(4*355*rad3, 3*113);

}

int kmain_chapitre2_et_3(      void      )
{

	
	//__asm("mrs r2, SPSR");
	__asm("mrs r2, CPSR");


	//__asm("cps #0b10000"); // mode USER
	//__asm("cps #0b10011"); // mode SVC

	__asm("bl dummy");

	__asm("cps #0b10000"); // mode USER

	int    radius = 5;

	__asm("mov r2, %0" : :"r"(radius) : "r2");    // ecriture registre
	__asm("mov %0, r3" : "=r"(radius) : : "r3");  // lecture registre
  


	int    volume;

	dummy(8);

	volume = compute_volume(radius);

	return    volume;

}




#include "syscall.h"

int kmain_sys_nop(      void      )
{

	__asm("cps #0b10000"); // mode USER
	
	  
	do {
		sys_nop();
	}while(1);
	
	sys_reboot();

	return    0;

}

int kmain_chap_4(void)
{

	__asm("cps #0b10000"); // mode USER
	sys_settime(0xffffffff11111111);

	
	return 0;
}

int div_2(int x){ return x/2;} 

void kmain_i(void)
{

	dummy_naked(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
	dummy(12);

}



# include"util.h"
#include"syscall.h"
#include"sched.h"

pcb_s pcb1, pcb2, pcb3;
pcb_s *p1, *p2,*p3;


//void user_process_1 and _2 ...
void user_process_1()
{
	int v1=5;
	while(1)
	{
		v1++;
		sys_yieldto(p2);
	}
}


void user_process_2()
{
	int v2= -12;
	while(1)
	{
		v2 -= 2 ;
		sys_yieldto(p1);
	}
}

void user_process_3()
{
	int v2= -12;
	while(1)
	{
		v2 -= 2 ;
		sys_yieldto(p1);
	}
}



void kmain_5_1(void)
{
	sched_init();
	p1=&pcb1;
	p2=&pcb2;
	p3=&pcb3;
	// initialize p1 and p2
	pcb1.lr_usr = (int )user_process_1;
	pcb2.lr_usr = (int )user_process_2;
	pcb3.lr_usr = (int )user_process_3;

	/*pcb1.lr_svc = (int )user_process_1;
	pcb2.lr_svc = (int )user_process_2;
	pcb3.lr_svc = (int )user_process_3;*/
	

	__asm("cps 0x10");

	//	switch CPU to USER mode
	// **********************************************************************
	
	sys_yieldto(p1);// this is now unreachable

	PANIC();

}



void kmain(void)
{
	sched_init();
	p1=create_process((func_t*) &user_process_1);
	p2=create_process((func_t*) &user_process_2);
	

	__asm("cps 0x10");

	//	switch CPU to USER mode
	// **********************************************************************
	
	sys_yieldto(p1);// this is now unreachable

	PANIC();

}
















