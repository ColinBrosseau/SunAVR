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

#define Version "0.1.2" //firmware version

// this code sets up counter1 for an 4kHz, 10bit, Phase Corrected PWM 
// @ 16Mhz Clock

#include <avr/io.h>
#include <util/delay.h>
#include "utility.h" // macro for simplification of pin manipulations
#include "i2cmaster.h" // for the ds1307 clock
#include "clock.h" // for the ds1307 clock
#include "lcd.h" // for LCD 

#define orderExponential 15 //nomber of terms to calculate en exponential (-1)
#define maxXexponential 3 // max x-axis value for calculation of en exponential
  // 5 means there is a ratio of ~1:100 between the minimum value and the maximum value
  // 7 means there is a ratio of 1:1 000 between the minimum value and the maximum value
  // 9 means there is a ratio of ~1:10 000 between the minimum value and the maximum value
#define numberSteps 10000 // Number of steps for the complete sequence
#define totalTime 30 // total time of a complete sequence (minutes)
#define waitTime 178 // == totalTime*60*1000/numberSteps -2 (in ms) : 
//         30-2->5minutes, 
//         180-2->30 minutes
// With 10000 steps and a waiting time of 6 ms it takes about 80 seconds (supposed to be 60 without calculations) to cycle.
// So about 2 ms of calculations per step (orderExponential=15).

MAKE_OUTPUT(LED, B, 5, 1) // LED indicator
MAKE_OUTPUT(LIGHT, B, 1, 1) // PWM (actual light)

MAKE_OUTPUT(GND_CLOCK, C, 2, 1) // power of the clock
MAKE_OUTPUT(VCC_CLOCK, C, 3, 1) // power of the clock

#define delay_blink 250

// Current time
TimeVal curTime;
// Alarm time
TimeVal alarmTime1;
TimeVal alarmTime2;

char bufferLCD[3]; 

// Calculate OCR1A value given a duty cycle (percent) 
int calculateOCR1Apercent(float intensity)
{
  return (int)(intensity*1023);
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

  INIT_LED();
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


void blink(int n)
{
  for (int j=0; j<n; j++)
    {
      // now turn ON the LED
      LED(1);
      _delay_ms(delay_blink); // wait
      // now turn off the LED
      LED(0);
      _delay_ms(delay_blink); // wait
    }
}

void blinkLIGHT(int n)
{
  for (int j=0; j<n; j++)
    {
      // now turn ON the LIGHT
      LIGHT(1);
      _delay_ms(delay_blink); // wait
      // now turn off the LIGHT
      LIGHT(0);
      _delay_ms(delay_blink); // wait
    }
}

void waitMinutes(int toWait)
{
  for (int j=0; j<toWait; j++)
    {
      for (int j=0; j<60; j++)
	{
	  _delay_ms(1000);
	}
    }
}

void waitHours(int toWait)
{
  for (int j=0; j<toWait; j++)
    {
      waitMinutes(60);
    }
}

int waitAlarm(TimeVal* ptr_alarmTime)
//return 1 if alarmTime = curTime (we have to wait)
//       0 if alarmTime != curTime
{
  return cmpTimeHM(&curTime, ptr_alarmTime) != 0;
}

//initialise LCD display
void initLCD(void) {
  //LCD
  lcd_init(LCD_DISP_ON); /* initialize display, cursor off */
  lcd_clrscr(); /* clear display and home cursor */
  lcd_puts("SunAVR ");
}

int main(void)
{
  // set clock alimentation (thru uC pins)
  INIT_GND_CLOCK();
  GND_CLOCK(0);
  INIT_VCC_CLOCK();
  VCC_CLOCK(1);
  // Initialize I2C library
  i2c_init();

  // Initialize the LCD
  initLCD();

  float intensity;

  //SET TIME
  /* curTime.year=2014; */
  /* curTime.month=2; */
  /* curTime.date=12; */
  /* curTime.hour=12; */
  /* curTime.min=54; */
  /* curTime.sec=01; */
  /* setTime(&curTime); */

  //alarm time #1
  alarmTime1.hour = 5;
  alarmTime1.min = 0;
  //alarm time #2
  alarmTime2.hour = 8;
  alarmTime2.min = 0;

  getTime(&curTime);
  
  //Check point (it will not shine if the clock is not available)
  blinkLIGHT(curTime.hour);
  blink(5);
  blinkLIGHT(alarmTime1.hour);
  blink(5);
  blinkLIGHT(alarmTime2.hour);
  blink(5);

  setIO();

  // wait until the alarm time
  while ( waitAlarm(&alarmTime1) & waitAlarm(&alarmTime2) )
    {
       _delay_ms(1000);
       getTime(&curTime);
       lcd_gotoxy(0,0);
       itoa(curTime.hour,bufferLCD,10); 
       lcd_puts(bufferLCD);
       lcd_puts(":");
       itoa(curTime.min,bufferLCD,10); 
       lcd_puts(bufferLCd);
       lcd_puts(":");
       itoa(curTime.sec,bufferLCD,10); 
       lcd_puts(bufferLCD);
    }
  
  OCR1A = 1000;
       _delay_ms(50);
  blink(5);

  float maxx;
  float minx;
  maxx = 11;
  minx = 6.4;
  for (float x=minx; x<maxx; x += (maxx-minx)/(float)numberSteps)
    {
      wait(waitTime); // wait
      intensity = expo(x)/expo(maxx); // calculate the relative intensity
      //	    intensity = expo(x)/expo(maxXexponential); // calculate the relative intensity
      OCR1A = calculateOCR1Apercent(intensity);
      // set PWM at intensity (relative intensity) @ 10bit
    }
  
  while (1)
    {
    }
}
