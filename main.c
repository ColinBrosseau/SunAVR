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

// Calculate OCR1A value given a duty cycle (percent) 
int calculateOCR1Apercent(int percent)
{
  return percent*1023/100;
}

int main(void)
{
    DDRB |= (1 << DDB1);
    // PB1 is now an output

    OCR1A = calculateOCR1Apercent(1);
    // set PWM for 1% duty cycle @ 10bit

    TCCR1A |= (1 << COM1A1);
    // set none-inverting mode

    TCCR1A |= (1 << WGM11) | (1 << WGM10);
    // set 10bit phase corrected PWM Mode

    TCCR1B |= (1 << CS11);
    // set prescaler to 8 and starts PWM


    while (1)
    {
        // we have a working Fast PWM
    }
}
