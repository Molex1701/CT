//////////////////////////////////////////////////////////////////////////
// Project: ClockTimer
// Programmer: Richard Waterman
// Purpose: To display time and timers to a 7-segment LED display.
// Program will display time rotating through the different active timers at a
// set interval. 
// Things I've learned:
// Setting and reading a Real Time clock. Sending information to 7-segment display.
// At first I used OneButton library for its built-in debouncing but then
// later added hardware debouncing circuit.
// Also added a parallel to serial chip (CD4021B) to save on inputs.
// went from needing 8 digital inputs to #? Though this was done on a UNO
// that has that many, I might make it with different ATMEL chip later that doesn't
// have that many inputs.
//////////////////////////////////////////////////////////////////////////
#include <Wire.h>       // SPI needed for Real Time Clock and display.

#include <DS1307RTC.h>  // For Real Time Clock

#include <OneButton.h> // Allows program to detect button presses with built-in
// debouncing. Also has long press, press, and double click features.
// Uses built-in pull-up resistor to make button high for off state.

// Used with the 7-segment display bought from Adafruit.

#include <Adafruit_GFX.h>         // Support library needed with LEDbackpack.h
#include <Adafruit_LEDBackpack.h>

#include <elapsedMillis.h> // Helps with measuring time changes in millis

#include <Time.h>  


// Includes I wrote needed for project
// What I wanted to do is have all types of clock timers in one list so I could
// rotate through them. So I made a base class called Timer that the others will
// be inherited from. 
#include "Timer.h"  // Is base class
#include "Clock.h"  // Used for storing information about a normal clock.
#include "countUpTimer.h"	// Used to count down from a specified time. Will
							// Will set off a buzzer when zero is reached.
#include "countDownTimer.h"	// Will count up for zero. Will have a buzzer go off
							// at a certain interval to remind me of passage of time.

Adafruit_7segment sevenSegment = Adafruit_7segment();

Timer *timers[5];
#define SHOWTIMERFOR 10000
elapsedMillis Showtimer;


enum displayState {dsNONE, dsNORMAL, ds10SEC };	

displayState dState;	
int currentTimer=0;
int digit0=0;  //M
int digit1=0;  //M
    // digit2    :
int digit3=0;  //S
int digit4=0;  //S


enum runState { rsNONE, rsCLOCK, rsMENU, rsERROR42 };  
runState FMS; // is used to control what state the program is in.
// rsNONE is just a default state when theres nothing to do.
// rsCLOCK to display time
// rsMENU when in the menu mode.
// rsERROR42 to display an error when time isn't set on the RealTimeClock chip.


elapsedMillis timeElapsed;


#define BLINKDOTFOR 500
elapsedMillis count = 0;  //

Clock *myclock;

// Variables used with menu.
//
// Menu button used to activate menu system with a long press.
// Once in menu mode to setup timer double click will activate that timer.
// Double click will change to next digit on display to change.
// Long press used to escape out of menu system.

OneButton menuButton(A0, true); //Menu button used to activate menu system with a long press.

OneButton   upButton(A1, true); // Up button to increase value of digit.

OneButton downButton(A2, true); // Down to decrease value of digit.

// While displaying a timer, a double click stop said timer.
// While in menu mode, normal press will change to previous digit.
// Double click will cancel current timer.
//
OneButton stopButton(A3, true);  // Stop timer, cancel timer.

// so far not using this array. I plan to use it if I want to make timers 
// count more than 59 minutes.
int digits[6];   // HHMMSS


// To show what digit is currently being edited that digit will flash
// it's dot.
 //To blink digit that cursor is currently on
boolean	digit4Blink; // S M
boolean digit3Blink; // S M
//      digit2          :
boolean	digit1Blink; // M H
boolean	digit0Blink; // M H

int currentDigit=4; //  0 1 2 3 4. 2 is used for colon.
                    //  M M :
//
// ^^Variables used with menu.  (I did comment like this so to keep track of where I am
// in the code.  In case it's on a different screen.
//

//////////////////////////////////////////////////////////////////////////
//Variables related to buzzer.
//////////////////////////////////////////////////////////////////////////
#define BUZZERPIN 2   // I like defines to save memory
#define BUZZERLENGTH  1500

//used to switch between updating timer vs buzzing.
enum buzzerState { bzNONE, bzSHOWCLOCK, bzBUZZER };

elapsedMillis buzzer;
buzzerState clockState = bzNONE;
//
// ^^ Variables related to buzzer.
//


//used to setup RealTimeClock and variables of program.
void setup()
{
	myclock = new Clock();
	myclock->init(); // I had originally tried to see if the time was set inside the
	//Clock() constructor but I don't think that code worked in the constuctor.
	//So had to use an init() function to test it that way.
    
	sevenSegment.begin(0x70);  //activates the LED display	
	
	if (myclock->isRunning() == false) // Test to see if RTC is set properly.
		FMS=rsERROR42;  // if not it will print 42 to screen to let user know
		// something wrong.
		//Just error number I picked from hitch hiker guide to the galaxy.
	else
	{  // setup the variables used with the normal operations of the project.
		
		pinMode(BUZZERPIN,OUTPUT); //Sets the pin used to turn on buzzer.


		// I did this because, if I didn't I think program used garbage memory
		// and would think there was something running and there wasn't.
		// So to make sure I set to zero and to not on and not running.
		countDownTimer *myTimerDown1= new countDownTimer();
		myTimerDown1->startTimer(0,0,0,0,0,0);
		countDownTimer *myTimerDown2= new countDownTimer();
		myTimerDown2->startTimer(0,0,0,0,0,0);
		countDownTimer *myTimerDown3= new countDownTimer();
		myTimerDown3->startTimer(0,0,0,0,0,0);
		countDownTimer *myTimerDown4= new countDownTimer();
		myTimerDown4->startTimer(0,0,0,0,0,0);
		timers[0]=myclock;
		timers[1]=myTimerDown1;
		timers[2]=myTimerDown2;
		timers[3]=myTimerDown3;
		timers[4]=myTimerDown4;
		
		menuButton.attachDoubleClick(menuButtonDoubleClick);
		menuButton.attachPress(menubLongPress);
		menuButton.attachClick(menuButtonClick);
		upButton.attachClick(upButtonClick);
		downButton.attachClick(downButtonClick);
		stopButton.attachClick(stopButtonClick);
		stopButton.attachDoubleClick(stopButtonDoubleClick);
		FMS=rsCLOCK;
		dState=dsNORMAL;
		clockState=bzSHOWCLOCK;
		timeElapsed=0;
		Showtimer=0;
	} //if (timers.get(0)->isRunning() == false)
	
} //setup()

void loop()
{
	switch (FMS)
	{
		case rsNONE:
			// this will only get here if I totally failed in my coding.
			sevenSegment.print(0xBEEF, HEX);
		break;
		case rsERROR42:  // Could not find real time clock. Display an error.
			sevenSegment.print(42);
		break; //case ERROR42:
		case rsCLOCK: 

		switch (dState)
		{
			case dsNORMAL:
			/// In Normal mode it will keep looking at timers that are running till it finds ones with less than 10 seconds.
			/// Once it finds one. It will switch to display that timer.
			if (Showtimer<SHOWTIMERFOR)
			{
				 int y;
				 y=1;
				 boolean search;
				 search=true;
				 while((y<4)&&(search))
				 {
						if (timers[y]->isRunning())
						{
							//boolean temp;
							//temp=false;
							//temp=timers[y]->getlessThan10Seconds();
							if (timers[y]->getlessThan10Seconds()==true)
							{
								currentTimer=y;
								search=false;
								dState=ds10SEC;
							} //if (timers[y]->_lessThan10Seconds)
						} //if (timers[y]->isRunning())
						 // else currentTimer=0;
						y++;
				  } // while(y<4)
				
				} // if (Showtimer<SHOWTIMERFOR)
				else {
					if (currentTimer<3)
						{//need to search for next running timer to switch to
						// so can display it.
							
							int x=currentTimer+1;
							boolean search=true;
							boolean found=false;
							while ((x<4) && (search==true))
							{
							  if (timers[x]->isRunning())
							  {
							    search=false;
								currentTimer=x;
								 Showtimer=0;
								 found=true;
							  }

							  x++;
							} //while ((x<4) && (search==true))
							if (found==false)
							{
								currentTimer=0;
								Showtimer=0;
							}
						} //if (currentTimer<3
						else
						{
							currentTimer=0;
							Showtimer=0;
						}
						
				} //if (Showtimer<SHOWTIMERFOR)

					break; //case dsNORMAL:
					case ds10SEC:
					// Will stay on this time till its done.  At zero time left will make a buzzer sound.
						switch (clockState)
						{
							case bzSHOWCLOCK:
								if (timers[currentTimer]->hasEnded())
								{
									clockState=bzBUZZER;
								
									if (timers[1]->hasEnded()) digit4Blink = false;
									if (timers[2]->hasEnded()) digit3Blink = false;
									if (timers[3]->hasEnded()) digit1Blink = false;
									if (timers[4]->hasEnded()) digit0Blink = false;
									timers[currentTimer]->_wasstarted=false;
									timers[currentTimer]->_hasended=false;
									buzzer=0;
								} //if (timers[currentTimer]->hasEnded())
							break; //case CSSHOWCLOCK:
							case bzBUZZER:
								//make buzzer go off.
								if (buzzer < BUZZERLENGTH)
									digitalWrite(BUZZERPIN,HIGH);
								else
									{
										digitalWrite(BUZZERPIN,LOW);
										clockState=bzSHOWCLOCK;
										currentTimer=0;  // switch back to displaying the clock.
										//// Turn off digitblink

									} // else if (buzzer < BUZZERLENGTH)

							break; //case CSBUZZER:
						} //switch (clockState)
					
					break; //case ds10SEC:
			
		} //switch (dState)
			
			menuButton.tick();
			digitalClockDisplay();
		break; //case CLOCK:
		case rsMENU:
		// Serial.println("MENU");
			displayMenu();
			menuButton.tick();
			upButton.tick();
			downButton.tick();
			stopButton.tick();
		break; //case MENU:
	} // switch (FMS)

			for (int x=1;x<5;x++) //The Clock (Timer) gets updated by system.
			{
				if (timers[x]->isRunning());
					  count=timers[x]->update(count);
			} // for (int x=1;x<5;x++)
			
	sevenSegment.writeDisplay();  // update the 7 segment display
} // loop()

void digitalClockDisplay()
{ // displays the current timer to the 7-segment LED.
  // If a particular timer is running a dot will be blinked to designate that.

	if (timers[currentTimer]->isRunning())
		if (timers[currentTimer]->getHours()>0)
		{
			digit0=getDigit(timers[currentTimer]->getHours(),2);
			digit1=getDigit(timers[currentTimer]->getHours(),1);
		
			digit3=getDigit(timers[currentTimer]->getMinutes(),2);
			digit4=getDigit(timers[currentTimer]->getMinutes(),1);
		} //if (timers[currentTimer]->getHours()>0)
	else
		{
			digit0=getDigit(timers[currentTimer]->getMinutes(),2);
			digit1=getDigit(timers[currentTimer]->getMinutes(),1);
			
			digit3=getDigit(timers[currentTimer]->getSeconds(),2);
			digit4=getDigit(timers[currentTimer]->getSeconds(),1);
		
		} // else if (timers[currentTimer]->getHours()>0)
	
	if (timeElapsed > BLINKDOTFOR)
	{
		if (timers[1]->isRunning()) digit4Blink = !digit4Blink;
		if (timers[2]->isRunning()) digit3Blink = !digit3Blink;
		if (timers[3]->isRunning()) digit1Blink = !digit1Blink;
		if (timers[4]->isRunning()) digit0Blink = !digit0Blink;
		timeElapsed = 0;
	} //if (timeElapsed > BLINKDOTFOR)
	
	sevenSegment.writeDigitNum(0,digit0,digit0Blink); 
	sevenSegment.writeDigitNum(1,digit1,digit1Blink);
	sevenSegment.drawColon(true); 
	sevenSegment.writeDigitNum(3,digit3,digit3Blink);
	sevenSegment.writeDigitNum(4,digit4,digit4Blink);
		
	sevenSegment.writeDisplay();
	
} // void digitalClockDisplay()

void displayMenu()
{
	if (timeElapsed > BLINKDOTFOR)
	{
					
		if (currentDigit==0) digit0Blink = !digit0Blink;
		if (currentDigit==1) digit1Blink = !digit1Blink;
		if (currentDigit==3) digit3Blink = !digit3Blink;
		if (currentDigit==4) digit4Blink = !digit4Blink;
		timeElapsed = 0;
	}
	// if (currentTimer==1)
	//// {


	sevenSegment.writeDigitNum(0,digit0,digit0Blink);
	sevenSegment.writeDigitNum(1,digit1,digit1Blink);
	sevenSegment.drawColon(true);
	sevenSegment.writeDigitNum(3,digit3,digit3Blink);
	sevenSegment.writeDigitNum(4,digit4,digit4Blink);
	sevenSegment.writeDisplay();
} // void displayMenu()

/*
Menu Button functions
*/

void menubLongPress()
{

	if (FMS == rsCLOCK)
	{
		FMS = rsMENU;
		currentTimer=1;
		currentDigit=4;
		if (timers[currentTimer]->isOn())
		{
			// get time from timer
			digit4=getDigit(timers[currentTimer]->getMinutes(),2);
			digit3=getDigit(timers[currentTimer]->getMinutes(),1);
			digit1=getDigit(timers[currentTimer]->getSeconds(),2);
			digit0=getDigit(timers[currentTimer]->getSeconds(),1);
		} // if (timerOne.isOn())
		else
		{
			digit0=0;
			digit1=0;
			digit3=0;
			digit4=0;
		} // if (timers[currentTimer]->isOn())	
	} //if (FMS == rsCLOCK)
	else
	{
		FMS = rsCLOCK;
		//interval = rotateInterval+5000;
		//currentTimertoShow=0;
		//digitalClockDisplay();
	} //else if (FMS == rsCLOCK)
	
}// void menubLongPress()

// this function will be called when the button was pressed 2 times in a short timeframe.
void menuButtonDoubleClick()
{
	switch (FMS)
	{
		case rsCLOCK:
		break;
		case rsMENU:

		//countDownTimer *temp = new countDownTimer();
		
		//temp->startTimer(digit2,digit1,digit3,digit4);
		int x=1;
		boolean search=true;
		while (x<4&&search)
		//search for unused timer slot to place timer in.
		{
			boolean temp =timers[x]->isRunning();
			if(timers[x]->isRunning()==false)
			  search=false;
			else x++;
		}
		if (digit3!=0||digit4!=0||digit1!=0||digit0!=0)
		{  // If one digit is set then its a countDownTimer
		   // if all digits are zero then it is a countUpTimer
		    countDownTimer *temp = new countDownTimer();
			temp->startTimer(digit0,digit1,digit3,digit4);
			timers[1]=temp;
			timers[1]->turnOnTimer();
			 //countDownTimer *temp2 = new countDownTimer();
			 //temp2->startTimer(4,3,2,1);
			 //timers[2]=temp2;
			 //timers[2]->startTimer();
			 //boolean t=timers[2]->isRunning();
		}
		else
		{
			countUpTimer *temp = new countUpTimer();
			//temp->startTimer(digit2,digit1,digit3,digit4);
			//timers[x]=temp;
		    //timers[x]->turnOnTimer();
			//timers[x]->startTimer(digit2,digit1,digit3,digit4);
			//timers[x]->turnOnTimer();
		}
		digit0Blink = false;
		digit1Blink = false;
		digit3Blink = false;
		digit4Blink = false;
		count=0;
		currentTimer=0;
		FMS=rsCLOCK;
		dState=dsNORMAL;
		//}
		break;
	} // switch (FMS)
} // void menuButtonDoubleClick()

void upButtonClick()
{
	if (FMS == rsMENU)
	{
		if (currentDigit==0)
		{	
			if ((digit0<6)&&(digit1==0))
				digit0++;
			else if (digit0<5)
				digit0++;
		}
		if (currentDigit==1)
		{
			if (digit1<9)  // allow 60
				digit1++;
		}
		if (currentDigit==3)
		{
			if ((digit3<6)&&(digit4==0))
				digit3++;
			else if (digit3<5)
				digit3++;
		}
		if (currentDigit==4)
			if (digit4<9)  // allow 60
			   digit4++;
	} //if (FMS == MENU)
	
} // void upButtonClick()

void downButtonClick()
{
	// Serial.println("downButtonClick");
	switch(FMS)
	{
		case rsCLOCK:
		break; // case CLOCK:
		case rsMENU:
		if (currentDigit == 0)
			if (digit0 > 0)
				digit0--;
		if (currentDigit == 1)
			if (digit1 > 0)
				digit1--;
		if (currentDigit == 3)
			if (digit3 > 0)
				digit3--;
		if (currentDigit == 4)
			if (digit4 > 0)
				digit4--;
		break; // case MENU:
	} // switch(FMS)
} //void downButtonClick()


void menuButtonClick()
{
	switch (FMS)
	{
		case rsCLOCK:
		break;
		case rsMENU:
			if (currentDigit>0)
			 {
				   if (currentDigit==3)
		 			   currentDigit=1; // Need to skip placement used for colon
				   else
					currentDigit--;						
			 }
		        

		digit4Blink=false;
		digit3Blink=false;
		digit1Blink=false;
		digit0Blink=false;
		break;
	} // switch (FMS)
} // void menuButtonClick()

void stopButtonClick()
{
	switch (FMS)
	{
		case rsCLOCK:
		break;  //case CLOCK:
		case rsMENU:
			if (currentDigit<4)
			{
				if (currentDigit==1)
				   currentDigit=3;
				else
					currentDigit++;
			}
				digit4Blink=false;
				digit3Blink=false;
				digit1Blink=false;
				digit0Blink=false;
		break; //case MENU:
	} //switch (FMS)
} //void stopButtonClick()


void stopButtonDoubleClick()
{
	//Serial.println("stop doubleclick");
	switch (FMS)
	{
		case rsCLOCK:
		break; // case CLOCK:
		case rsMENU:
		//if (currentTimer==1)
		//Serial.print("Current timer is ");
		//Serial.println(currentTimer);
		timers[currentTimer]->stopTimer();
		//digit1=0;
		//digit2=0;
		//digit3=0;
		//digit4=0;
		 FMS=rsCLOCK;
		break; // case MENU:
	} // switch (FMS)
} //void stopButtonDoubleClick()

// Utility function to get specific digit
int getDigit(int number,  int digit) {
	int retVal = number % 10;
	while (--digit)
	{
		number /= 10;
		retVal = number % 10;
	}
	return retVal;
} // int getDigit()