//SunAVR
//
// ATmega8 @ 16 MHz (Arduino NG)
//
// A dawn simulator
//
// Originaly inspired from 
//   https://sites.google.com/site/qeewiki/books/avr-guide/pwm-atmega8
// 
//Possible improvements:
//Bugs:
//
//-----
//Done
//
//Solved bugs:
//Done improvements

#define Version "0.0.2" //firmware version

// this code sets up counter1 for an 4kHz, 10bit, Phase Corrected PWM 
// @ 16Mhz Clock

#include <avr/io.h>
#include <util/delay.h>

// Calculate OCR1A value given a duty cycle (percent) 
int calculateOCR1Apercent(int percent)
{
  return percent*10;
}

void setIO(void)
{
  DDRB |= (1 << DDB1);
  // PB1 is now an output
  
  TCCR1A |= (1 << COM1A1);
  // set none-inverting mode
  
  TCCR1A |= (1 << WGM11) | (1 << WGM10);
  // set 10bit phase corrected PWM Mode
  
  TCCR1B |= (1 << CS11);
  // set prescaler to 8 and starts PWM
}

int main(void)
{
  setIO();
  
  while (1)
    {
      for(int i=0;i<100;i++)
	//loop over 0-100 % in 10 seconds
	{
	  _delay_ms(100);
	  // wait (ms)
	  OCR1A = calculateOCR1Apercent(i);
	  // set PWM for i (%) duty cycle @ 10bit
	}
    }
}
