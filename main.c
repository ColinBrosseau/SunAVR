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

#define Version "0.1.0" //firmware version

// this code sets up counter1 for an 4kHz, 10bit, Phase Corrected PWM 
// @ 16Mhz Clock

#include <avr/io.h>
#include <util/delay.h>

#define orderExponential 15 //nomber of terms to calculate en exponential (-1)
#define maxXexponential 7 // max x-axis value for calculation of en exponential
  // 7 means there is a ratio of 1:1 000 between the minimum value and the maximum value
  // 9 means there is a ratio of ~1:10 000 between the minimum value and the maximum value
#define numberSteps 1000 // Number of steps for the complete sequence
#define totalTime 20 // total time of a complete sequence (seconds)
#define waitTime 20 // == totalTime*1000/numberSteps (in ms)

// Calculate OCR1A value given a duty cycle (percent) 
int calculateOCR1Apercent(float intensity)
{
  int out = intensity*1023;
  return out;
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

float expo(float x)
// calculate an approximation of the exponential of x
{
  float out = 1; // order 0
  int nfact = 1; //factorial of n == n!
  float powx = 1; //x to the power of n == x^n
  for(int i=1; i<orderExponential; i++)
    {
      nfact *= i; 
      powx *= x;
      out += powx/nfact; // order i 
    }
  return out; 
}

void wait(int ms)
// wait for specified number ms
//it's a bit of a hack because _delay_ms cannot be more that ~6000 and must be defined af compile time
{
  if (ms<6000)
    {
      _delay_ms(waitTime);
    }
  else
    {
      _delay_ms(6000);
    }
}

int main(void)
{
  setIO();
  
  float intensity;

  while (1)
    {
      for (float x=0; x<maxXexponential; x += (float)maxXexponential/(float)numberSteps)
	{
	  wait(waitTime); // wait
	  intensity = expo(x)/expo(maxXexponential); // calculate the relative intensity
	  //intensity = .1;
	  OCR1A = calculateOCR1Apercent(intensity);
	  // set PWM at intensity (relative intensity) @ 10bit
	}
    }
}
