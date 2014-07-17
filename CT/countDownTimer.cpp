#include "countDownTimer.h"

countDownTimer::countDownTimer()
{
	_ison=false;
	_lessThan10Seconds=false;
	//Serial.println("Constructor");
} //countDownTimer

// default destructor
countDownTimer::~countDownTimer()
{
} //~countDownTimer
void countDownTimer::startTimer(int two, int one,int three,int four)
{
	_ison=true;
	//	_prevtime = 0;
	_hours=0;
	_minutes=(four*10)+three;
	_seconds=(two*10)+one;
	_lessThan10Seconds=false;
	_wasstarted=false;
	if (_minutes==0 && _seconds<10)
	{
		_lessThan10Seconds=true;
	}
	//Serial.println("inStartTimer");
	//Serial.print("Minutes ");
	//Serial.println(_minutes);
	//Serial.print("Seconds ");
	//Serial.println(_seconds);
	_hasended=false;
}
void countDownTimer::startTimer(int six, int five, int four,int three , int two, int one)
{
	_ison=true;
	_running=false;
	_hours=(six*10)+five;
	_minutes=(four*10)+three;
	_seconds=(two*10)+one;
	_hasended=false;
	_lessThan10Seconds=false;
}
void countDownTimer::turnOff()
{
	_ison=false;
	_running=false;
}
void  countDownTimer::turnOnTimer()
{
	_running=true;
	_wasstarted=true;
	_hasended=false;
	//Serial.println("Turn on Timer");
}
boolean countDownTimer::isOn()
{
	return _ison;
}

uint8_t countDownTimer::getHours()
{
	uint8_t _value;
	//Serial.println("countdowntimer::gethours");
	//if (_hours==0)
	//_value = _minutes;
	//else
	_value = _hours;
	
	return _value;
}

uint8_t countDownTimer::getMinutes()
{
	uint8_t _value;
	//if ((_minutes==0)&&(_hours==0))
	//_value = _seconds;
	//else
	_value=_minutes;
	return _value;
}

uint8_t countDownTimer::getSeconds()
{
	return _seconds;
}

boolean countDownTimer::isRunning()
{
	return _running;
}

elapsedMillis countDownTimer::update(elapsedMillis _count)
{

	//Serial.print("Minute ");
	//Serial.println(_minutes);
	//Serial.print("Seconds ");
	//Serial.println(_seconds);
	//Serial.print("Running ");
	//Serial.println(_running);

	if (_running && _seconds < 10 && _minutes==0 && _hours==0)
	_lessThan10Seconds=true;
	
	
	if (_running && _count >= 1000)
	{
		_count -=1000;
		if (_seconds > 0)
		{
			_seconds--;
			} else {
			if (_minutes > 0)
			{
				_minutes--;
				_seconds = 59;
			} else
			{
				_running = false;
				_hasended = true; // trigger buzzer, relay, etc
			}
		}
		//	_prevtime = _count;
	}
	return _count;
}// int countDownTimer::update(int count)

void countDownTimer::startTimer()
{
	_running=true;
}

boolean countDownTimer::hasEnded()
{
	// had to been started (wasstarted) and ended to sound off buzzer.
	
	return _wasstarted && _hasended;
}

void countDownTimer::stopTimer()
{
	
}

void countDownTimer::init()
{
	
}

boolean countDownTimer::getlessThan10Seconds()
{

	return _lessThan10Seconds;
}
