/*	Author: gyama009
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #10 Exercise #4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

task tasks[5];

const unsigned char tasksNum = 5;
const unsigned long tasksPeriodGCD = 1;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 300;
const unsigned char periodSpeaker = 1;
const unsigned char periodFrequency = 1;
const unsigned long periodCombineLEDs = 1;

unsigned char Frequency;
unsigned char Speaker;
unsigned char threeLEDs;
unsigned char blinkingLED;

void TimerISR(){
	unsigned char i;
	for(i = 0; i < tasksNum; ++i){
		if (tasks[i].elapsedTime >= tasks[i].period){
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

enum Frequency_States {Frequency_SMStart, F_Wait, F_Dec, F_Inc};

int TickFct_Frequency(int state){
	switch(state){ // Initial transition
		case Frequency_SMStart:
			Frequency = 2;
			state = F_Wait;
			break;
		case F_Wait:
			if(((~PINA & 0xFF) == 0x00) || ((~PINA & 0xFF) == 0x03)){
				state = F_Wait;
			}
			else if((~PINA & 0xFF) == 0x01){
				if(Frequency > 0){
				Frequency--;
				}
				state = F_Dec;
			}
			else if((~PINA & 0xFF) == 0x02){
				Frequency++;
				state = F_Inc;
			}
			break;
		case F_Dec:
			if((~PINA & 0xFF) == 0x01){
				state = F_Dec;
			}
			else{
				state = F_Wait;
			}
			break;
		case F_Inc:
			if((~PINA & 0xFF) == 0x02){
				state = F_Inc;
			}
			else{
				state = F_Wait;
			}
			break;
		default:
			state = Frequency_SMStart;
			break;
	} // Transitions
	switch(state){ // State actions
		case F_Wait:
			break;
		case F_Dec:
			break;
		case F_Inc:
			break;
		default:
			break;
	} // State actions
	return state;
}

enum Speaker_States {Speaker_SMStart, S_OFF, Toggle1, Toggle2};

int TickFct_Speaker(int state){
	static unsigned char cnt;
	switch(state){ // Initial transition
		case Speaker_SMStart:
			Speaker = 0;
			state = Toggle1;
			break;
		case S_OFF:
			if((~PINA & 0xFF) == 0x00){
				state = S_OFF;
			}
			else if((~PINA & 0xFF) == 0x04){
				cnt = 0;
				state = Toggle1;
			}
			break;
		case Toggle1:
			if(((~PINA & 0xFF) == 0x04) && (cnt < Frequency)){
				state = Toggle1;
			}
			else if(((~PINA & 0xFF) == 0x04) && (cnt >= Frequency)){
				cnt = 0;
				state = Toggle2;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = S_OFF;
			}
			break;
		case Toggle2:
			if(((~PINA & 0xFF) == 0x04) && (cnt < Frequency)){
                                state = Toggle2;
                        }
                        else if(((~PINA & 0xFF) == 0x04) && (cnt >= Frequency)){
                                cnt = 0;
                                state = Toggle1;
                        }
                        else if((~PINA & 0xFF) == 0x00){
                                state = S_OFF;
                        }
			break;
		default:
			state = Speaker_SMStart;
			break;
	} // Transitions
	switch(state){ // State actions
		case S_OFF:
			Speaker = 0;
			break;
		case Toggle1:
			cnt++;
			Speaker = 0x10;
			break;
		case Toggle2:
			cnt++;
			Speaker = 0x00;
			break;
		default:
			break;
	} // State actions
	return state;
}


enum BL_States {BL_SMStart, LEDOff, LEDOn};

int TickFct_BlinkLED(int state){
	switch(state){ // Initial transition
		case BL_SMStart:
			blinkingLED = 0;
			state = LEDOff;
			break;
		case LEDOff:
			state = LEDOn;
			break;
		case LEDOn:
			state = LEDOff;
			break;
		default:
			state = BL_SMStart;
			break;
	} // Transitions
	switch(state){ // State actions
		case LEDOff:
			blinkingLED = 0x08;
			break;
		case LEDOn:
			blinkingLED = 0x00;
			break;
		default:
			break;
	} // State actions
	return state;
}

enum TL_States {TL_SMStart, T0, T1, T2};

int TickFct_ThreeLEDs(int state){
	switch(state){ // Initial transition
		case TL_SMStart:
			threeLEDs = 0;
			state = T0;
			break;
		case T0:
			state = T1;
			break;
		case T1:
			state = T2;
			break;
		case T2:
			state = T0;
			break;
		default:
			state = TL_SMStart;
			break;
	} // Transitions
	switch(state){ // State actions
		case T0:
			threeLEDs = 0x01;
			break;
		case T1:
			threeLEDs = 0x02;
			break;
		case T2:
			threeLEDs = 0x04;
			break;
		default:
			break;
	} // State actions
	return state;
}

enum CL_States {CL_SMStart, Output};

int TickFct_CombineLEDs(int state){
	switch(state){ // Initial transition
		case CL_SMStart:
			state = Output;
			break;
		case Output:
			state = Output;
			break;
		default:
			state = CL_SMStart;
			break;
	} // Transitions
	switch(state){ // State actions
		case Output:
			PORTB = (threeLEDs | blinkingLED | Speaker);
			break;
		default:
			break;
	} // State actions
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as input
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as output
	unsigned char i = 0;
	tasks[i].state = BL_SMStart;
	tasks[i].period = periodBlinkLED;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlinkLED;
	++i;
	tasks[i].state = TL_SMStart;
	tasks[i].period = periodThreeLEDs;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ThreeLEDs;
	++i;
	tasks[i].state = CL_SMStart;
	tasks[i].period = periodCombineLEDs;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_CombineLEDs;
	++i;
        tasks[i].state = Speaker_SMStart;
        tasks[i].period = periodSpeaker;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &TickFct_Speaker;
	++i;
        tasks[i].state = Frequency_SMStart;
        tasks[i].period = periodFrequency;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &TickFct_Frequency;

	TimerSet(tasksPeriodGCD);
	TimerOn();
	while (1) {
		continue;
	}
	return 0;
}
