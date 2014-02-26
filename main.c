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

#define Version "0.2.0" //firmware version

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
// About 2 ms of calculations per step (orderExponential=15).
  // 5 means there is a ratio of ~1:100 between the minimum value and the maximum value
  // 7 means there is a ratio of 1:1 000 between the minimum value and the maximum value
  // 9 means there is a ratio of ~1:10 000 between the minimum value and the maximum value
#define totalTime 30 // total time of a complete sequence (minutes)

MAKE_OUTPUT(LIGHT, B, 1, 1) // PWM (actual light)

MAKE_INPUT(SELECT, C, 3, 1) // select button
MAKE_INPUT(ENTER, C, 2, 1) // enter button

#define delay_blink 250

// Current time
TimeVal curTime;
// Alarm time
TimeVal alarmTime1;
TimeVal alarmTime2;

char bufferLCD[6]; 

// Get the seconds value at the last iteration
uint8_t lastSec;

unsigned short mode=0;
unsigned short key;

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

  INIT_SELECT();
  INIT_ENTER();
  //PULLUP_SELECT();
  //PULLUP_ENTER();
  PORTC |= 1 << PC2 | 1 << PC3; //temporary replacement for PULLUP_...
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


void printTime(TimeVal* time, int level){
  int hour = time->hour;
  int min = time->min;
  int sec = time->sec;

  if (level>=0)
    {
      itoa(hour,bufferLCD,10); 
      if (hour < 10)				
	{
	  lcd_puts(" ");
	}
      lcd_puts(bufferLCD);
      lcd_puts(":");
      itoa(min,bufferLCD,10); 
      if (min < 10)
	{
	  lcd_puts("0");
	}
      lcd_puts(bufferLCD);
    }

  if (level>=2)
    {
      lcd_puts(":");
      itoa(sec,bufferLCD,10); 
      if (sec < 10)
	{
	  lcd_puts("0");
	}
      lcd_puts(bufferLCD);
    }

  lcd_puts(" ");

  if (level>=1)
    {
      int date = time->date;
      //int month = time->month;
      int year = time->year;
      itoa(date,bufferLCD,10);
      lcd_puts(bufferLCD);
      lcd_puts(" ");
      lcd_puts(getMonthStr(time));
      lcd_puts(" ");
      itoa(year,bufferLCD,10);
      lcd_puts(bufferLCD);
      lcd_puts(" ");
    }

}

unsigned short read_keys()
{
  unsigned short key = 0;

  if (SELECT() == 0)
    {
      key = 1;
    }
  else if (ENTER() == 0)
    {
      key = 2;
    }

  return key;
}

int main(void)
{
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
  blinkLIGHT(2);

  setIO();

  short alarmActive = 0;

  // wait until the alarm time
  while ( waitAlarm(&alarmTime1) & waitAlarm(&alarmTime2) )
    {
      
      // Get the seconds value at the last iteration
      lastSec = curTime.sec;
      
      // Read the current time
      getTime(&curTime);
      
      // Update the seconds count
      if (curTime.sec != lastSec)
	{
	  getTime(&curTime);
	  lcd_gotoxy(0,0);
	  printTime(&curTime, 1);
	  
	  lcd_gotoxy(0,1);
	  printTime(&alarmTime1, 0);
	  printTime(&alarmTime2, 0);
	}
      
      key = read_keys();
      // If a key was pressed
      if (key)
	{
	  OCR1A = 50;
	  _delay_ms(50);
	  OCR1A = 0;
	  _delay_ms(250);
	  // Add a debouncing delay
	}
      
      /* lcd_gotoxy(0,1); */
      /* if (key==0) */
       /* 	 { */
       /* 	   lcd_puts("00"); */
       /* 	 } */
       /* else if (key==1) */
       /* 	 { */
       /* 	   lcd_puts("10"); */
       /* 	 } */
       /* else if (key==2) */
       /* 	 { */
       /* 	   lcd_puts("01"); */
       /* 	 } */
       /* else if (key==3) */
       /* 	 { */
       /* 	   lcd_puts("11"); */
       /* 	 } */

       /* lcd_puts(" "); */
       /* itoa(key,bufferLCD,10); */
       /* lcd_puts(bufferLCD); */
       /* lcd_puts(" "); */

       /* lcd_gotoxy(0,1); */
       /* itoa(SELECT(),bufferLCD,10); */
       /* lcd_puts(bufferLCD); */
       /* lcd_puts(" "); */
       
       /* itoa(ENTER(),bufferLCD,10); */
       /* lcd_puts(bufferLCD); */
       /* lcd_puts(" "); */
       
    }

  alarmActive = 1;
  
  OCR1A = 50;
  _delay_ms(50);

  unsigned int difference=0;

  float maxx;
  float minx;
  maxx = 11;
  minx = 6.4;

  float x;

  while (alarmActive)
    {
      // Get the seconds value at the last iteration
      lastSec = curTime.sec;
      
      // Read the current time
      getTime(&curTime);
      
      // Update the seconds count
      if (curTime.sec != lastSec)
	{
	  difference++;
	  
	  lcd_gotoxy(0,0);
	  printTime(&curTime, 1);
	  
	  lcd_gotoxy(0,1);
	  printTime(&alarmTime1, 0);
	  printTime(&alarmTime2, 0);

	  x = minx + (maxx-minx)/(totalTime*60)*(float)difference;
	  intensity = expo(x)/expo(maxx); // calculate the relative intensity
	  
	  /* lcd_gotoxy(0,0); */
	  /* bufferLCD[0] = ' '; */
	  /* bufferLCD[1] = ' '; */
	  /* bufferLCD[2] = ' '; */
	  /* bufferLCD[3] = ' '; */
	  /* bufferLCD[4] = ' '; */
	  /* bufferLCD[5] = '\0'; */
	  itoa(difference,bufferLCD,10);
	  lcd_puts(bufferLCD);
	  lcd_puts(" ");
	  
	  /* lcd_gotoxy(0,1); */
	  /* bufferLCD[0] = ' '; */
	  /* bufferLCD[1] = ' '; */
	  /* bufferLCD[2] = ' '; */
	  /* bufferLCD[3] = ' '; */
	  /* bufferLCD[4] = ' '; */
	  /* bufferLCD[5] = '\0'; */
	  /* itoa(calculateOCR1Apercent(intensity),bufferLCD,10); */
	  /* lcd_puts(bufferLCD); */
	  /* lcd_puts(" "); */
	  
	  OCR1A = calculateOCR1Apercent(intensity);
	  // set PWM at intensity (relative intensity) @ 10bit
	}

      if (difference==(totalTime*60))
	{
	  alarmActive = 0;
	}
    }

  while(1)
    {
      //blink
      _delay_ms(200);
      //      OCR1A = 0;
      _delay_ms(200);
      //      OCR1A = 1023;

      // Read the current time
      getTime(&curTime);
      lcd_gotoxy(0,0);
      printTime(&curTime, 1);
    }
}
