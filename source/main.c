/*	Author: gyama009
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #10 Exercise #2
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

unsigned char threeLEDs;
unsigned char blinkingLED;

enum BL_States {BL_SMStart, LEDOff, LEDOn} BL_State;

void TickFct_BlinkLed(){
	switch(BL_State){ // Initial transition
		case BL_SMStart:
			blinkingLED = 0;
			BL_State = LEDOff;
			break;
		case LEDOff:
			BL_State = LEDOn;
			break;
		case LEDOn:
			BL_State = LEDOff;
			break;
		default:
			BL_State = BL_SMStart;
			break;
	} // Transitions
	switch(BL_State){ // State actions
		case LEDOff:
			blinkingLED = 0x08;
			break;
		case LEDOn:
			blinkingLED = 0x00;
			break;
		default:
			break;
	} // State actions
}

enum TL_States {TL_SMStart, T0, T1, T2} TL_State;

void TickFct_ThreeLeds(){
	switch(TL_State){ // Initial transition
		case TL_SMStart:
			threeLEDs = 0;
			TL_State = T0;
			break;
		case T0:
			TL_State = T1;
			break;
		case T1:
			TL_State = T2;
			break;
		case T2:
			TL_State = T0;
			break;
		default:
			TL_State = TL_SMStart;
			break;
	} // Transitions
	switch(TL_State){ // State actions
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
}

enum CL_States {CL_SMStart, Output} CL_State;

void TickFct_CombineLeds(){
	switch(CL_State){ // Initial transition
		case CL_SMStart:
			CL_State = Output;
			break;
		case Output:
			CL_State = Output;
			break;
		default:
			CL_State = CL_SMStart;
			break;
	} // Transitions
	switch(CL_State){ // State actions
		case Output:
			PORTB = (threeLEDs | blinkingLED);
			break;
		default:
			break;
	} // State actions
}

int main(void) {
	unsigned long BL_elapsedTime = 0;
	unsigned long TL_elapsedTime = 0;
	const unsigned long timerPeriod = 100;

	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as output
	TimerSet(100);
	TimerOn();
	BL_State = BL_SMStart;
	TL_State = TL_SMStart;
	CL_State = CL_SMStart;
	while (1) {
		if(BL_elapsedTime >= 1000){	// 1000 ms period
			TickFct_BlinkLed();	// Execute one tick of the BlinkLed synchSM
			BL_elapsedTime = 0;
		}
		if(TL_elapsedTime >= 300){	// 300 ms period
			TickFct_ThreeLeds();	// Execute one tick of the ThreeLeds synchSM
			TL_elapsedTime = 0;
		}
		TickFct_CombineLeds();	// Tick the CombineLeds synchSM
		while(!TimerFlag){}	// Wait for timer period
		TimerFlag = 0;		// Lower flag raised by timer
		BL_elapsedTime += timerPeriod;
		TL_elapsedTime += timerPeriod;
	}
}
