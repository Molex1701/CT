#include <Adafruit_LEDBackpack.h>


#include <Wire.h>       // SPI needed for Real Time Clock and display.

#include <DS1307RTC.h>  // For Real Time Clock

#include <OneButton.h> // Allows program to detect button presses with built-in
// debouncing. Also has long press, press, and
// doubleclick features.

// Used with the 7-segment display bought
// from Adafruit.
#include <Adafruit_GFX.h>         // Support library needed with LEDbackpack.h

#include <elapsedMillis.h> // Helps with measuring time changes in millis

//#include <LinkedList.h> // Used to store the timers instead of a fixed array.
// Actually I think I am going back to fixed array. Will just have flag for when
// Timer is not in use. I think I can use the ison flag, I already have.
#include <Time.h>
// Includes I wrote needed for project
#include <Timer.h>
#include <Clock.h>
#include <countUpTimer.h>
#include "countDownTimer.h"

// Variables used for debugging.
// Can be used when you want to only display a variable once with Serial.
// output.
//#define _DEBUG ON		// Used to debug program. Will send messages to Serial out
// if defined. Comment out if don't need the messages.
//#define _DEBUG2 ON
//
//boolean printonce=false;  //false means wasn't printed, true and it was.
//boolean printonce2=false; //false means wasn't printed, true and it was.
////
// ^^Variables used for debugging.
//


Adafruit_7segment matrix = Adafruit_7segment();
//LinkedList<Timer*> timers;	// This will store Clock as Timer 0 and rest will be
// either countDown or countUp timers.
Timer *timers[5];
#define SHOWTIMERFOR 10000
elapsedMillis Showtimer;

enum runState { rsNONE, rsCLOCK, rsMENU, rsERROR42 };
enum displayState {dsNONE, dsNORMAL, ds10SEC };	

displayState dState;	
int currentTimer=0;
int digit1=0;  //
int digit2=0;  //
int digit3=0;  //
int digit4=0;  //
runState FMS;

elapsedMillis timeElapsed;


#define BLINKDOTFOR 500
elapsedMillis count = 0;

Clock *myclock;

// countdown or count up timers.
//unsigned int rotateInterval = 5000; // show an active timer for 5 seconds

//
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

int digits[6];   // HHMMSS
// To show what digit is currently being edited that digit will flash
// it's dot.
boolean	digit1Blink; //To blink digit that cursor is currently on
boolean digit2Blink;
boolean	digit3Blink;
boolean	digit4Blink;

boolean *digitsBlink[4];
int currentDigit=1; // 6 5 4 3 1 0. 2 is used for colon.
//
// ^^Variables used with menu.
//

//
//Variables related to buzzer.
//
// Change BUZZERPIN from 13 (built in LED), pin 2 (buzzer).
#define BUZZERPIN 2   // I like defines to save memory
#define BUZZERLENGTH  1500

//used to switch between updating timer vs buzzing.
enum buzzerState { bzNONE, bzSHOWCLOCK, bzBUZZER };

elapsedMillis buzzer;
buzzerState clockState = bzNONE;
//
// ^^ Variables related to buzzer.
//

void setup()
{
	/* Initialize serial interface */
	//Serial.begin(115200);

	matrix.begin(0x70);
	pinMode(BUZZERPIN,OUTPUT);
	/* Show build information via serial interface */
	//#if defined(ENABLE_TEXT_MESSAGE)
	//ShowBuildInfo();
	//#endif

	//timers = LinkedList<Timer*>();

	myclock = new Clock();
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

	//myTimerDown1->startTimer(0,0,0,5);
	myclock->init();
	//timers.add(myclock);
	// timers.add(myTimerDown1);
	
	if (timers[0]->isRunning() == false) // Test to see if RTC is set properly.
		FMS=rsERROR42;  // if not it will print 42 to screen to let user know
	// something wrong.
	else
	{
		menuButton.attachDoubleClick(menuButtonDoubleClick);
		menuButton.attachPress(menubLongPress);
		menuButton.attachClick(menuButtonClick);
		upButton.attachClick(upButtonClick);
		downButton.attachClick(downButtonClick);
		stopButton.attachClick(stopButtonClick);
		stopButton.attachDoubleClick(stopButtonDoubleClick);
		FMS=rsCLOCK;
		dState=dsNORMAL;
		//		Serial.print("displayState ");
		//		Serial.println(dState);
		clockState=bzSHOWCLOCK;
		timeElapsed=0;
		Showtimer=0;
	} //if (timers.get(0)->isRunning() == false)

	// timers.get(1)->turnOnTimer(); // Force timer on for testing.
	
} //setup()

void loop()
{
	switch (FMS)
	{
		case rsNONE:
			matrix.print(0xBEEF, HEX);
		break;
		case rsERROR42:  // Could not find real time clock. Display an error.
			matrix.print(42);
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
							  boolean temp;
							  temp =timers[x]->isRunning();
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
									//// Turn off digitblink
									if (timers[1]->hasEnded()) digit1Blink = false;
									if (timers[2]->hasEnded()) digit2Blink = false;
									if (timers[3]->hasEnded()) digit3Blink = false;
									if (timers[4]->hasEnded()) digit4Blink = false;
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
										//if (timers[1]->hasEnded()) digit1Blink = !digit1Blink;
										//if (timers[2]->hasEnded()) digit2Blink = !digit2Blink;
										//if (timers[3]->hasEnded()) digit3Blink = !digit3Blink;
										//if (timers[4]->hasEnded()) digit4Blink = !digit4Blink;
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
				if (timers[x]->isRunning())
				{
					//Serial.print("isrunning ");
					//Serial.println(timers[x]->isRunning());
					  count=timers[x]->update(count);
				}//if (timers[x]->isRunning())
			} // for (int x=1;x<5;x++)
			
	matrix.writeDisplay();  // update the 7 segment display
} // loop()

void digitalClockDisplay()
{

	if (timers[currentTimer]->isRunning())
	if (timers[currentTimer]->getHours()>0)
	{
		digit4=getDigit(timers[currentTimer]->getHours(),2);
		digit3=getDigit(timers[currentTimer]->getHours(),1);
		digit2=getDigit(timers[currentTimer]->getMinutes(),2);
		digit1=getDigit(timers[currentTimer]->getMinutes(),1);
	}
	else
	{
		digit4=getDigit(timers[currentTimer]->getMinutes(),2);
		digit3=getDigit(timers[currentTimer]->getMinutes(),1);
		digit2=getDigit(timers[currentTimer]->getSeconds(),2);
		digit1=getDigit(timers[currentTimer]->getSeconds(),1);
	}
	if (timeElapsed > BLINKDOTFOR)
		{
			if (timers[1]->isRunning()) digit1Blink = !digit1Blink;
			boolean t=timers[2]->isRunning();
			if (timers[2]->isRunning()) digit2Blink = !digit2Blink;
			if (timers[3]->isRunning()) digit3Blink = !digit3Blink;
			if (timers[4]->isRunning()) digit4Blink = !digit4Blink;
			timeElapsed = 0;
		}
	
	//
	matrix.writeDigitNum(0,digit4,digit4Blink);//,digit4Blink);
	matrix.writeDigitNum(1,digit3,digit3Blink);//,digit3Blink);
	
	matrix.drawColon(true);
	
	matrix.writeDigitNum(3,digit2,digit2Blink);//,digit2Blink);
	matrix.writeDigitNum(4,digit1,digit1Blink);
	matrix.writeDisplay();
	
	
} // void digitalClockDisplay()

void displayMenu()
{
	//Serial.println("displayMenu");
	if (timeElapsed > BLINKDOTFOR)
	{
					
		if (currentDigit==1) digit2Blink = !digit1Blink;
		if (currentDigit==2) digit2Blink = !digit2Blink;
		if (currentDigit==3) digit3Blink = !digit3Blink;
		if (currentDigit==4) digit4Blink = !digit4Blink;
		timeElapsed = 0;
	}
	// if (currentTimer==1)
	// {
	//if (timers.get(currentTimer)->isOn())
	//{
	//digit4=getDigit(timers2[currentTimer]->getHours(),2);
	//digit3=getDigit(timers2[currentTimer]->getHours(),1);
	//digit2=getDigit(timers2[currentTimer]->getMinutes(),2);
	//digit1=getDigit(timers2[currentTimer]->getMinutes(),1);
	//}
	//else
	//{
	//  digit4=0;
	//   digit3=0;
	//   digit2=0;
	//   digit1=0;
	//}

	//  }
	
	matrix.writeDigitNum(0,digit4,digit4Blink);
	matrix.writeDigitNum(1,digit3,digit3Blink);
	matrix.drawColon(true);
	
	// Serial.println(minute());
	matrix.writeDigitNum(3,digit2,digit2Blink);
	matrix.writeDigitNum(4,digit1,digit1Blink);
	matrix.writeDisplay();
} // void displayMenu()

/*
Menu Button functions
*/
void menuButtonClick()
{
	switch (FMS)
	{
		case rsCLOCK:
		break;
		case rsMENU:
		if (currentDigit<4)
		// if (currentDigit==1)
		//		currentDigit=2; // Need to ski
		currentDigit++;

		digit1Blink=false;
		digit2Blink=false;
		digit3Blink=false;
		digit4Blink=false;
		break;
	} // switch (FMS)
} // void menuButtonClick()

void menubLongPress()
{

	if (FMS == rsCLOCK)
	{
		FMS = rsMENU;
		currentTimer=1;
		currentDigit=1;
		if (timers[currentTimer]->isOn())
		{
			//Serial.println("currentTimer isOn()");
			// get time from timer
			digit4=getDigit(timers[currentTimer]->getMinutes(),2);
			digit3=getDigit(timers[currentTimer]->getMinutes(),1);
			digit2=getDigit(timers[currentTimer]->getSeconds(),2);
			digit1=getDigit(timers[currentTimer]->getSeconds(),1);
			
			
			digit1Blink= timers[currentTimer]->isOn();

		} // if (timerOne.isOn())
		else
		{
			digit4=0;
			digit3=0;
			digit2=0;
			digit1=0;
		} // if (timers[currentTimer]->isOn())
		
	}
	else
	{
		FMS = rsCLOCK;
		//interval = rotateInterval+5000;
		//currentTimertoShow=0;
		digitalClockDisplay();
	}
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
		if (digit2!=0||digit1!=0||digit3!=0||digit4!=0)
		{  // If one digit is set then its a countDownTimer
		   // if all digits are zero then it is a countUpTimer
		    countDownTimer *temp = new countDownTimer();
			temp->startTimer(digit2,digit1,digit3,digit4);
			timers[1]=temp;
			timers[1]->turnOnTimer();
			 countDownTimer *temp2 = new countDownTimer();
			 temp2->startTimer(4,3,2,1);
			 timers[2]=temp2;
			 timers[2]->startTimer();
			 boolean t=timers[2]->isRunning();
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
	// Serial.println("upButtonClick");
	if (FMS == rsMENU)
	{
		#ifdef _DEBUG1
		Serial.println("FMS == MENU");
		#endif // _DEBUG1

		if (currentDigit==1)
		{
			//Serial.println("currentDigit==1");
			
			if (digit1<9)
			{
				digit1++;
				//Serial.println()
			}
		}
		if (currentDigit==2)
		{
			if ((digit2<6)&&(digit1==0)) // allow 60
			digit2++;
			else if (digit2<5)  // 5#
			digit2++;
		}
		if (currentDigit==3)
		{
			if (digit3<9)
			digit3++;
		}
		if (currentDigit==4)
		{
			if ((digit4<6)&&(digit3==0))  // allow 60
			digit4++;
			else if (digit4<5)  // 5#
			digit4++;
		}
	} //if (FMS == MENU)
	
	//Serial.print("currentDigit ");
	//Serial.println(currentDigit);
	//Serial.print("Digit 1 ");
	//Serial.println(digit1);
	//
} // void upButtonClick()

void downButtonClick()
{
	// Serial.println("downButtonClick");
	switch(FMS)
	{
		case rsCLOCK:
		break; // case CLOCK:
		case rsMENU:
		if (currentDigit == 1)
		if (digit1 > 0)
		digit1--;
		if (currentDigit == 2)
		if (digit2 > 0)
		digit2--;
		if (currentDigit == 3)
		if (digit3 > 0)
		digit3--;
		if (currentDigit == 4)
		if (digit4 > 0)
		digit4--;
		break; // case MENU:
	} // switch(FMS)
} //void downButtonClick()

void stopButtonClick()
{
	switch (FMS)
	{
		case rsCLOCK:
		break;  //case CLOCK:
		case rsMENU:
			if (currentDigit>0)
				currentDigit--;
				digit1Blink=false;
				digit2Blink=false;
				digit3Blink=false;
				digit4Blink=false;
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