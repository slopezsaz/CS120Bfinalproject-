/*
 * CS/EE 120B Final Project 
 * 
 * Suzette Lopez Saz
 * 
 */ 


#include <avr/io.h>

#include "timer.h"
#include "scheduler.h"
#include "lcd.c"




void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

#define msensor (~PINA & 0x02) //PA0
#define SlowLow 140  //bottom range for slow speed
#define SlowHighMediumLow 170 // top range for slow speed and top range for med speed
#define MediumHighFastLow 176 // top rand for med speed and low for fast speed


//===============================MOTION SM=========================================


unsigned char motion;       //ONLY SHARED VARIABLE

enum states {motioninit, motioncheck, motionwait};
int motionSM (int state) {
	
	static unsigned long i;
	
	switch(state) {
		
		case motioninit:
		state = motioncheck;
		motion = 0;
		break;
		
		case motioncheck:
		if (msensor) {
			state = motionwait;
			i = 0;
			motion = 1;
		}
		break;
		
		case motionwait:
		if ((++i >= 4000) && (!msensor) ) {
			state = motioncheck;
			motion = 0;
		}
		break;
		default:
		state = motioninit;
		break;
	}
	
	switch (state) {
		
		case motioninit: break;
		case motioncheck: break;
		case motionwait:
		if ((i >= 4000) && msensor) {
			i = 0;
		}
		break;
	}
	
	return state;
	
}

//====================================End Motion SM=====================================

//====================================Nokia Screen Display SM============================

enum Display_states {nokia_init, nokia_OFF, MotionDetected, slow, medium, fast} ;

int Display_SM (int state) {
	
	unsigned char temperature = ADC;
	//unsigned char motion = PINB & 0x01;
	
	switch(state) {
		
		case nokia_init:
		state = nokia_OFF;
		break;
		
		case nokia_OFF:
		
		if (motion) {
			state = MotionDetected;
		}
		else{
			state = nokia_OFF;
		}
		break;
		
		case MotionDetected:
		
		
		if ((SlowLow <= temperature) && (temperature < SlowHighMediumLow)) {   //slow speed
			state = slow;
		}
		
		if ((SlowHighMediumLow <= temperature) && ( temperature< MediumHighFastLow)) { //medium speed
			state = medium;
		}
		
		if (MediumHighFastLow <= temperature) {  // fast speed
			state = fast;
		}
		
		break;
		
		
		case slow:
		if (temperature < SlowLow) {
			state = nokia_OFF;
		}
		
		if ((SlowHighMediumLow <= temperature) && ( temperature < MediumHighFastLow)) { //medium speed
			state = medium;
		}
		
		if ((SlowLow <= temperature) && (temperature < SlowHighMediumLow)) {   //slow speed
			state = slow;
		}
		
		if (!motion) {
			state = nokia_OFF;
		}
		
		break;
		
		case medium:
		
		if (temperature < SlowHighMediumLow); {   //slow speed
			state = slow;
		}
		
		if (MediumHighFastLow <= temperature) {  // fast speed
			state = fast;
		}
		
		if ((SlowHighMediumLow <= temperature) && ( temperature < MediumHighFastLow)) { //medium speed
			state = medium;
		}
		
		if (!motion) {
			state = nokia_OFF;
		}
		
		
		break;
		
		case fast:
		
		if ((SlowHighMediumLow <= temperature) && ( temperature < MediumHighFastLow)) { //medium speed
			state = medium;
		}
		
		if (MediumHighFastLow <= temperature) {  // fast speed
			state = fast;
		}
		
		if (!motion) {
			state = nokia_OFF;
		}
		
		break;
		
		default:
		state = nokia_init;
		break;
	}
	
	switch (state) {
		
		case nokia_init:
		break;
		
		case nokia_OFF:
		
		LCD_Clear();
		gotoXY(0,2);
		LCD_String( "Fan OFF");
		
		break;
		
		case MotionDetected:
		LCD_Clear();
		gotoXY(0,2);
		LCD_String("Motion");
		
		break;

		case slow:
		LCD_Clear();
		gotoXY(0,2);
		LCD_String("Speed: Low");
		
		break;
		
		case medium:
		LCD_Clear();
		gotoXY(0,2);
		LCD_String("Speed: Med");
		
		break;
		
		case fast:
		LCD_Clear();
		gotoXY(0,2);
		LCD_String("Speed: Fast");
		
		break;
	}
	
	return state;
}



unsigned char motor_out = 0x00;

//=====================================End Nokia Display SM==================================================



//=====================================Stepper motor SM========================================================

enum Stepper_states {stepper_init, pulse1, pulse2, pulse3, pulse4};

int Steppermotor_SM (int state) {
	
	unsigned char temperature = ADC;
	
	static unsigned char i;
	static unsigned char wait;   // wait changes period
	
	if ((SlowLow <= temperature) && (temperature < SlowHighMediumLow)) {   //slow speed
		wait = 6;
	}
	
	if ((SlowHighMediumLow <= temperature) && (temperature < MediumHighFastLow)) { //medium speed
		wait = 4;
	}
	
	if (MediumHighFastLow <= temperature) {  // fast speed
		wait = 2;
	}
	
	switch(state) {
		
		case stepper_init:
		state = pulse1;
		
		break;
		
		case pulse1:
		
		if (i < wait) {
			state = pulse1;
		}
		
		if (i >= wait && motion) {
			state = pulse2;
			i = 0;
		}
		break;
		
		case pulse2:
		
		if (i < wait) {
			state = pulse2;
		}

		if (i >= wait && motion) {
			state = pulse3;
			i = 0;
		}
		break;
		
		case pulse3:
		
		if (i < wait) {
			state = pulse3;
		}
		
		if (i >= wait && motion) {
			state = pulse4;
			i = 0;
		}
		break;
		
		case pulse4:
		
		if (i < wait) {
			state = pulse4;
		}
		
		if (i >= wait && motion) {
			state = pulse1;
			i = 0;
		}
		
		break;
		
		default:
		state = stepper_init;
		break;
	}
	switch (state) {
		
		case pulse1:
		motor_out = 0x80;
		++i;
		break;
		
		case pulse2:
		motor_out = 0x40;
		++i;
		break;
		
		case pulse3:
		motor_out = 0x20;
		++i;
		break;
		
		case pulse4:
		motor_out = 0x10;
		++i;
		break;
	}
	PORTC = motor_out;
	return state;
}


//======================================End Stepper Motor SM=========================================================

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	unsigned long DisplayPeriod = 150;
	unsigned long MotorPeriod = 1;
	unsigned long motionPeriod = 1;
	
	
	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = DisplayPeriod;//Task Period.
	task1.elapsedTime = DisplayPeriod;//Task current elapsed time.
	task1.TickFct = &Display_SM;//Function pointer for the tick.

	// Task 2
	task2.state = -1;//Task initial state.
	task2.period = motionPeriod;//Task Period.
	task2.elapsedTime = motionPeriod;//Task current elapsed time.
	task2.TickFct = &motionSM;//Function pointer for the tick.

	// Task 3
	task3.state = -1;//Task initial state.
	task3.period = MotorPeriod;//Task Period.
	task3.elapsedTime = MotorPeriod; // Task current elapsed time.
	task3.TickFct = &Steppermotor_SM; // Function pointer for the tick
	
	
	ADC_init();
	LCD_Init();
	
	TimerSet(1);
	TimerOn();
	

	unsigned short i;
    while(1)
    {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

 
}