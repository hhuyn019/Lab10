#include <avr/io.h>
#include <avr/interrupt.h>
#define input (~PINA & 0x04)

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff() {
	TCCR1B = 0x00;
}
void TimerISR() {
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum ThreeLEDsSM_States {BEGIN, LED1, LED2, LED3} ThreeLEDsSM_State;
enum BlinkingLEDSM_States {BEGIN2, LED_ON, LED_OFF} BlinkingLEDSM_State;
enum CombineLEDsSM_States {BEGIN3, COMBINE} CombineLEDsSM_State;
enum ToggleSM_States {BEGIN4, WAIT, ON, OFF} ToggleSM_State;
	
unsigned char threeLEDs = 0x00;
unsigned char blinkingLED = 0x00;
unsigned char temp = 0x00;
unsigned char toggle = 0x00;
unsigned short ms = 0x00;
unsigned short ms2 = 0x00;
unsigned short ms3 = 0x00;

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	ThreeLEDsSM_State = BEGIN;
	BlinkingLEDSM_State = BEGIN;
	CombineLEDsSM_State = BEGIN;
	TimerSet(1);
	TimerOn();
	while(1)
	{
		ThreeLEDsSM();
		BlinkingLEDSM();
		ToggleSM();
		CombineLEDsSM();
		while(!TimerFlag);
		TimerFlag = 0;
	}
}

void ThreeLEDsSM() {
	switch(ThreeLEDsSM_State) {
		case BEGIN:
		ThreeLEDsSM_State = LED1;
		break;
		case LED1:
		if(ms < 300) {
			ThreeLEDsSM_State = LED1;
			ms = ms + 1;
			break;
			} else {
			ThreeLEDsSM_State = LED2;
			ms = 0;
			break;
		}
		case LED2:
		if(ms < 300) {
			ThreeLEDsSM_State = LED2;
			ms = ms + 1;
			break;
			} else {
			ThreeLEDsSM_State = LED3;
			ms = 0;
			break;
		}
		case LED3:
		if(ms < 300) {
			ThreeLEDsSM_State = LED3;
			ms = ms + 1;
			break;
			} else {
			ThreeLEDsSM_State = LED1;
			ms = 0;
			break;
		}
	}
	switch(ThreeLEDsSM_State) {
		case BEGIN:
		break;
		case LED1:
		threeLEDs = 0x01;
		break;
		case LED2:
		threeLEDs = 0x02;
		break;
		case LED3:
		threeLEDs = 0x04;
		break;
	}
}

void BlinkingLEDSM() {
	switch(BlinkingLEDSM_State) {
		case BEGIN2:
		BlinkingLEDSM_State = LED_ON;
		break;
		case LED_ON:
		if(ms2 < 1000) {
			BlinkingLEDSM_State = LED_ON;
			ms2 = ms2 + 1;
			break;
			} else {
			BlinkingLEDSM_State = LED_OFF;
			ms2 = 0;
			break;
		}
		case LED_OFF:
		if(ms2 < 1000) {
			BlinkingLEDSM_State = LED_OFF;
			ms2 = ms2 + 1;
			break;
			} else {
			BlinkingLEDSM_State = LED_ON;
			ms2 = 0;
			break;
		}
	}
	switch(BlinkingLEDSM_State) {
		case BEGIN2:
		break;
		case LED_ON:
		blinkingLED = 0x08;
		break;
		case LED_OFF:
		blinkingLED = 0x00;
		break;
	}
}

void ToggleSM() {
	switch(ToggleSM_State) {
		case BEGIN4:
			ToggleSM_State = WAIT;
			break;
		case WAIT:
			if (input == 0x04) {
				ToggleSM_State = ON;
				break;
			} else {
				ToggleSM_State = WAIT;
				break;
			}
			break;
		case ON:
			if ((input == 0x04) && (ms3 < 2)) {
				ToggleSM_State = ON;
				++ms3;
				break;
			} else if (input == 0x04) {
				ToggleSM_State = OFF;
				ms3 = 0;
				break;
			} else {
				ToggleSM_State = WAIT;
				break;
			}
		case OFF:
			if ((input == 0x04) && (ms3 < 2)) {
				ToggleSM_State = OFF;
				++ms3;
				break;
			} else if (input == 0x04) {
				ToggleSM_State = ON;
				ms3 = 0;
				break;
			} else {
				ToggleSM_State = WAIT;
				break;
			}
	}
	switch(ToggleSM_State) {
		case BEGIN4:
			break;
		case WAIT:
			break;
		case ON:
			toggle = 0x10;
			break;
		case OFF:
			toggle = 0x00;
			break;
	}
}

void CombineLEDsSM() {
	switch(CombineLEDsSM_State) {
		case BEGIN3:
		CombineLEDsSM_State = COMBINE;
		break;
		case COMBINE:
		break;
	}
	switch(CombineLEDsSM_State) {
		case BEGIN3:
		break;
		case COMBINE:
		PORTC = (threeLEDs | blinkingLED) | toggle;
		break;
	}
}
