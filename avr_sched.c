/*
 * avr_sched.c
 * -->Premptive Multitasking Scheduler
 *
 * Created: 27-06-2016 09:29:09 PM
 * Author : Akash Kollipara
 */ 

#define F_CPU 16000000UL

#define maxTask 3

#define StackPointer SP

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void timer_init();
void tasks_init();


//tasks
void blink1();
void blink2();
void blink3();

uint8_t R=0, currentTaskId=0, nextTaskId=0;

/*
 * -->to define the task's state these variables are used
 * -->t_pause and t_quit will be used when priority is concerned, also when dynamic allocation is required
 */

typedef enum status{t_ready, t_busy, t_wait, t_quit} status;

typedef struct TaskControlBlock		//defines and controls a task for this scheduler
{
	uint8_t task_id;
	enum status state;
	uint8_t priority;
	unsigned int stack_pointer_begin;
	unsigned int stack_pointer_end;
	void (*fnctpt)(void);
} tcb;
tcb task[maxTask];

int main(void)
{
    	cli();
	//initialization 
	
	timer_init();			//initializes timer for scheduler
	tasks_init();			//initializes tasks
	
	//setting up first task parameters
	StackPointer=task[0].stack_pointer_begin;
	currentTaskId=task[0].task_id;
	if(task[0].state==t_ready)
		task[0].state=t_busy;
	sei();
	//running first task
	task[0].fnctpt();
	
    	while (1);
}

ISR(TIMER0_COMPA_vect)
{
	uint8_t q=0;
	cli();
	//getting the termination pointer of individual stacks
	task[currentTaskId].stack_pointer_end=StackPointer;
	for(q=0; q<maxTask; q++)
	{
		if(task[q].state==t_ready)
		{
			//setting up task parameters 
			task[q].state=t_busy;
			currentTaskId=task[q].task_id;		//sets task_id for saving context in corresponding task's stack
			StackPointer=task[q].stack_pointer_begin;
			sei();
			task[q].fnctpt();		//execute task if not priorly executed
		}
		R=q;
	}
	if(R==maxTask-1)	//checks if all the tasks are in queue
	{
		if(nextTaskId==maxTask) nextTaskId=0;		//resets the the task_id if it exceeds maxTask
		if(task[nextTaskId].state==t_busy)
		{
			//pointing stack to start context restoring
			StackPointer=task[nextTaskId].stack_pointer_end;
			currentTaskId=nextTaskId;				//when next interrupt occurs, this var is used as the task_id for getting stack end pointer
			nextTaskId=task[nextTaskId].task_id+1;	//increments the task_id for next call of scheduler
		}
	}
}

void blink1()
{
	DDRB|= (1<< PORTB4);
	while(1)
	{
		PORTB |= (1<<PORTB4);
		_delay_ms(50);
		PORTB &= ~(1<<PORTB4);
		_delay_ms(50);
	}
}

void blink2()
{
	DDRB|= (1<<PORTB3);
	while(1)
	{
		PORTB |= (1<<PORTB3);
		_delay_ms(100);
		PORTB &= ~(1<<PORTB3);
		_delay_ms(100);
	}
}

void blink3()
{
	DDRB |= (1<<PORTB2);
	while(1)
	{
		PORTB |= (1<<PORTB2);
		_delay_ms(300);
		PORTB &= ~(1<<PORTB2);
		_delay_ms(300);
	}
}

void timer_init()
{
	TCCR0A=(1<<WGM01);	//enable CTC-mode
	TCCR0B=(1<<CS02);	//pre-scalar to /256
	TIMSK0=(1<<OCIE0A);		//timer0A interrupt enable
	/*
	 * pre-scalar is set to /256
	 * 1.024ms for each task execution per submission
	 * can be increased to 32.64ms by assigning OCR0A=254 (MAX) for prescalar of 1024
	 */
	OCR0A =31;
}

void tasks_init()
{
	//assigning task_id
	task[0].task_id=0;
	task[1].task_id=1;
	task[2].task_id=2;
	
	//setting up PC for tasks
	task[0].fnctpt=&blink1;
	task[1].fnctpt=&blink2;
	task[2].fnctpt=&blink3;
	
	//setting up stack
	task[0].stack_pointer_begin=RAMEND-100;
	task[1].stack_pointer_begin=RAMEND-140;
	task[2].stack_pointer_begin=RAMEND-180;
	
	//assigning priority
	task[0].priority=0;
	task[1].priority=0;
	task[2].priority=0;
	
	//setting up states
	task[0].state=t_ready;
	task[1].state=t_ready;
	task[2].state=t_ready;
}
