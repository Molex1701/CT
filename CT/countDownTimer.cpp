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
void countDownTimer::startTimer(int zero, int one,int three,int four)
{
	_ison=true;
	_hours=0;
	_minutes=(zero*10)+one;
	_seconds=(three*10)+four;
	_lessThan10Seconds=false;
	_wasstarted=false;
	if (_minutes==0 && _seconds<10)
		_lessThan10Seconds=true;

	_hasended=false;
}
void countDownTimer::startTimer(int six, int five, int four,int three , int one, int zero)
{
	_ison=true;
	_running=false;
	_hours=(six*10)+five;
	_minutes=(three*10)+four;
	_seconds=(zero*10)+one;
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
}
boolean countDownTimer::isOn()
{
	return _ison;
}

uint8_t countDownTimer::getHours()
{
	uint8_t _value;

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
