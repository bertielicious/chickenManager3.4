/*
 * File:   main.c
 * Author: Phil Glazzard
 *
 * Created on 19 November 2019, 18:49
 */
/* Software Construction
 1. Problem definition
 * Control of a motorised door to the chicken coup where the door will be:
 a)  Open if the light level is daylight or the clock timer has been 
 * triggered to open the door, or if the manual open button has been pressed.
 b) Closed if the light level is night level or the clock timer has been 
 * triggered to close the door or the manual close button has been 
 * pressed
 2. Display on LCD the date, time, door open time, door close time, timer ON/OFF
 * light sensor ON/ OFF, manual OPEN/ CLOSE ON/ OFF and battery charge level on 
 * an LCD with three button keypad to control all of above. Confirm key presses
 *  via buzzer.
 3. Via blu-tooth connection, transmit all LCD display info to phone app, and 
 * also allow control of all door functions from the app.
 4. Microcontroller to be in sleep mode except when:
 * a) door needs to move from closed to open or open to closed due to timer or 
 * light sensor inputs or manual close/ open button.
 * b) someone presses a keypad button (time out back to sleep mode after 
 * 2 minutes without button activity)
 * (c) blue tooth transmit and receive
 2. Sub systems
 * a) LCD
 * b) uC sleep
 * c) blue tooth
 * d) door
 * e) timer/ clock
 * f) light sensor
 * g) keypad
 * h) solar psu
 * i) motor
 * 
 3. Structs
 * a) LCD
 *  date(day, month, year)
 *  time (hour, minutes, seconds)
 *  door (open time, close time)
 *  timer(on, off)
 *  set time(door open time, door close time)
 *  light sensor (on, off)
 *  light sensor (adjust up time, adjust down time)
 *  manual door button (open, close)
 *  battery charge level display (o% - 100%)
 *  keypad with three buttons (up, down, enter)
 *  confirm key press with buzzer (key pressed, key not pressed)
 * 
 * b) uC sleep
 * if (blue tooth active or button pressed or door needs to open or close (light sensor or timer))
 *    wake up
 *    run required code
 *    else
 *    sleep
 * 
 * c) blu-tooth
 *    transmit
 *    receive
 *    run necessary code
 * 
 * d) door
 *    open 
 *    close
 *    open or close door
 * 
 * e) clock/ timer
 *      check open
 *      check close
 *      open or close door
 * 
 * f) light sensor
 *      check light
 *      check dark
 *      open or close door
 * 
 * g) keypad
 *    check button press
 *       which button pressed
 *       setup screen or exit to main loop
 *       take action or open / close door
 *  
 * 
 * h) solar psu
 *      charge LiPo battery
 *      display battery charge level
 * 
 * i) motor
 *    motor moves clockwise = open door
 *    motor moves anti-clockwise = close door
 * 
 * 
 *                  16f1459
 *                  ---------
 *   +5 Volts    1 |Vdd      | 20 0 Volts
        LCD D6   2 |RA5   RA0| 19   - PUSH BUTTON
 *    motor ACW  3 |RA4   RA1| 18   + PUSH BUTTON
       MCLR      4 |RA3      | 17  MOTOR DIRECTION
 *  ENT PBUTTON  5 |RC5   RC0| 16  LIGHT SENSOR (analog)
 *    RS         6 |RC4   RC1| 15  RTC INPUT
 *    EN         7 |RC3   RC2| 14  TOP LIMIT SWITCH
 *    LCD D4     8 |RC6   RB4| 13  SDA
 *    LCD D5     9 |RC7   RB5| 12  LCD D7
 *    TX        10 |RB7   RB6| 11  SCL
 *                  ---------
 motor CW and BOTTOM LIMIT SWITCH need to be allocated  uC pin each
 */

#include "config.h"
#include "configOsc.h"
#include "configPorts.h"
#include "configUsart.h"
#include "putch.h"
#include <stdio.h>
#include "configLCD.h"
#include "pulse.h"
#include "nibToBin.h"
#include "byteToBin.h"
#include "configI2c.h"
#include "i2cStart.h"
#include "i2cWrite.h"
#include "i2cRestart.h"
#include "PCF8583Read.h"
#include "PCF8583Write.h"
#include "setupTime.h"
#include "dateInput.h"
#include "timeInput.h"
#include "clearRow.h"
#include "setupDate.h"
void main(void)
{
    uchar numSec, secMsb, secLsb = 0;
    uchar numHour, hourMsb, hourLsb = 0;
    uchar numMin, minMsb, minLsb = 0;
   
    
    
    uchar colon = 0x3a;     // : symbol
    uchar backSlash = 0x2f; // / symbol
    uchar static result;
    configOsc();
    configPorts();
    configUsart();
    
    configLCD();
   
    configI2c();
    //PCF8583Write(0xa0,0x00,0x08);// mask bit set in control reg allowing direct reading of day and date
    
    while(On == 0)
    {
        timeInput();
        setupTime();
        numSec = PCF8583Read(0xa0, 0x02);   // read seconds
        secLsb = numSec%10;
        secMsb = numSec/10;
        byteToBin(0,0x92);
        byteToBin(1, secMsb + 0x30);
        byteToBin(1, secLsb + 0x30);
        
        byteToBin(0,0x91);
        byteToBin(1, colon);
        
        numMin = PCF8583Read(0xa0, 0x03);   // read mins
       
        minLsb = numMin%10;
        minMsb = numMin/10;
        byteToBin(0,0x8f);
        byteToBin(1, minMsb + 0x30);
        byteToBin(1, minLsb + 0x30);
        
        byteToBin(0,0x8e);
        byteToBin(1, colon);
        
        numHour = PCF8583Read(0xa0, 0x04);   // read hours
       
        hourLsb = numHour%10;
        hourMsb = numHour/10;
        byteToBin(0,0x8c);
        byteToBin(1, hourMsb + 0x30);
        byteToBin(1, hourLsb + 0x30);
    }
    dateInput();
    __delay_ms(1500);
    while(state < 22)
    {
        
        setupDate();
        
        numDay = PCF8583Read(0xa0, 0x05);   // read day
        numDay = numDay & 0x3f;
       // printf("numDay %d\n", numDay);
        dayLsb = numDay%10;
        dayMsb = numDay/10;
        byteToBin(0,0x80);
        byteToBin(1, dayMsb + 0x30);
        byteToBin(1, dayLsb + 0x30);
        
        byteToBin(0,0x82);
        byteToBin(1, backSlash);
        
        numMth = PCF8583Read(0xa0, 0x06);   // read month
        numMth = numMth & 0x1f;
        //printf("numMth %d\n", numMth);
        mthLsb = numMth%10;
        mthMsb = numMth/10;
        byteToBin(0,0x83);
        byteToBin(1, mthMsb + 0x30);
        byteToBin(1, mthLsb + 0x30);
        
        byteToBin(0,0x85);
        byteToBin(1, backSlash);
        
        byteToBin(1, 0x02 + 0x30);      //2
        byteToBin(1, 0x00 + 0x30);      //0
        
        numYr = PCF8583Read(0xa0, 0x05);   // read year
        
        printf("numYr in Main %d\n", numYr);
        if(numYr == 0)
        {
            numYr = 19;
        }
        else if(numYr == 1)
        {
            numYr = 20;
        }
        else if(numYr == 2)
        {
            numYr = 21;
        }
        else if(numYr == 3)
        {
            numYr = 22;
        }
        yrLsb = (numYr)%10;
        yrMsb = (numYr)/10;
        
        byteToBin(0,0x88);
        byteToBin(1, yrMsb + 0x30);
        byteToBin(1, yrLsb + 0x30);
        
    }
    
    while(1)
    {
       numSec = PCF8583Read(0xa0, 0x02);   // read seconds
        secLsb = numSec%10;
        secMsb = numSec/10;
        byteToBin(0,0x92);
        byteToBin(1, secMsb + 0x30);
        byteToBin(1, secLsb + 0x30);
        
        byteToBin(0,0x91);
        byteToBin(1, colon);
        
        numMin = PCF8583Read(0xa0, 0x03);   // read mins
       
        minLsb = numMin%10;
        minMsb = numMin/10;
        byteToBin(0,0x8f);
        byteToBin(1, minMsb + 0x30);
        byteToBin(1, minLsb + 0x30);
        
        byteToBin(0,0x8e);
        byteToBin(1, colon);
        
        numHour = PCF8583Read(0xa0, 0x04);   // read hours
       
        hourLsb = numHour%10;
        hourMsb = numHour/10;
        byteToBin(0,0x8c);
        byteToBin(1, hourMsb + 0x30);
        byteToBin(1, hourLsb + 0x30);
    }
}    

