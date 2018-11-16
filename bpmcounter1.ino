/*
 * BPM counter
 * 
 * Small utility to display BPM on a 16x2 I2C LED display 
 * Written for DIN Sync devices like the TR-606.  
 * High accuracy version, counts time between pulses on external timer 
 * 
 * Beware, high-precision timing on Arduinos will depend on the system speed and clock 
 * settings. I set up the timer attached to the DIN sync, but other timers may affect 
 * resolution and speed too. You may have to tweak the prescaler (256) and factor (16) 
 * on anything other than a default configured Arduino/Genuino Uno. 
 *
 * Connect the (TTL level) clock signal to pin 8 (and ground to GND) - e.g. 
 * DIN Sync Pin 3 to Arduino Uno Pin 8, DIN Pin 2 to any GND terminal on the Uno 
 * 
 * needs Schmitt Trigger for unstable signals
 * 
 * Copyright 2018 Sevo Stille <mailto:sevo@radiox.de>
 * @license GPL-3.0+ <http://www.gnu.org/licenses/gpl.html>
 * 
*/


#include <LiquidCrystal_I2C.h> // For display

LiquidCrystal_I2C lcd(0x3F, 16, 2); // default 16/2 I2C display

unsigned int time;
  unsigned char doubleOverflowError=0;
float bpm, hz, ms;
const unsigned int ticks_per_beat =  24;

 typedef enum {
      CAPTURE_1,
      CAPTURE_2,
      WAIT
  } timer_state_t;

  volatile timer_state_t flag = WAIT;

  volatile unsigned int Capt1, Capt2, CaptOvr;

  volatile unsigned long T1Ovs;

  void InitTimer1(void)
  {
     //Set Initial Timer value
     // All measurements against TCNT are relative, so no need to reset
     // TCNT1=0;

     // Note we need to set up all the timer control bits because we do not know what state they are in
     // If, for example, the WGM bits are set to a PWM mode then the TCNT is going to be resetting out 
    // from under us rather than monotonically counting up to MAX

     TCCR1A = 0x00;

     //First capture on rising edge
     TCCR1B =(1<<ICES1);
     //Enable input capture and overflow interrupts
     TIMSK1|=(1<<ICIE1)|(1<<TOIE1);
  }

 // J: Note that it would be ok to start the timer when we assign TCCR1B in InitTimer since nothing will happen when the ISR is called until we set flag to CAPTURE1

  void StartTimer1(void)
  {
  //Start timer with 256 prescaler (CS12)

  //  Note that we know that the other CS bits already are 0 because of the Assignment in InitTimer
  TCCR1B |= (1<<CS12);  

  //Enable global interrutps
  // J: Interrupts are turned on by  Arduino platform startup code
  //  sei();
  }

  ISR(TIMER1_CAPT_vect) {

   switch(flag) {
   case CAPTURE_1:
       Capt1 = ICR1;

       // Reset the overflow to 0 each time we start a measurement
       T1Ovs=0;
       doubleOverflowError=0;
       flag = CAPTURE_2;
       break;

   case CAPTURE_2:
       Capt2 = ICR1;

       // Grab a snap shot of the overflow count since the timer will keep counting (and overflowing);
       CaptOvr = T1Ovs;    
       flag = WAIT;

       break;
      }
  }


  ISR(TIMER1_OVF_vect)
  {
    T1Ovs++;

    // check for overflow of the overflow, otherwise if it overflows we would get an incorrect answer.
    if (!T1Ovs) {
      doubleOverflowError=1;
    }
  }

  void setup()
  {

    Serial.begin(9600);

    InitTimer1();
    StartTimer1();
     lcd.init(); //Init display 
     lcd.backlight(); //Power up backlight  
     lcd.print("BPM Counter");           
     lcd.setCursor(0, 1);
     lcd.print("by sevo");          
     delay(2000);                     // pause
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("BPM counting");       // go
       
  }

  void loop() {
    
    flag = CAPTURE_1;

    while (flag != WAIT);

     if (doubleOverflowError) {
         Serial.println( "Double Overflow Error! Use a bigger prescaller!");
     } else {
         time = ( (unsigned long) (Capt2) + (CaptOvr * 0x10000UL) )-Capt1 ;
     ms = time*16.0 + 0.0001;  // Prescaled to 256/16 (at timer res 16), add something small to avoid div by zero
     hz = 1000000.0/ms;
     bpm = (hz/ticks_per_beat)*60; // for ppq

     lcd.setCursor(0, 0);
     lcd.print(ms/1000.0, 4);                        //  PRINT ms
     lcd.print(" ms ");                       
     lcd.print(CaptOvr, DEC);                  // DEBUG print overflow
     lcd.print(" OVR  ");     
     lcd.setCursor(0, 1);
     lcd.print(bpm,2);                        //  PRINT BPM
     lcd.print(" BPM ");
          lcd.print(hz,2);                        //  PRINT HZ
     lcd.print(" Hz ");
         
     } 
}
