#include "Timer.h"

class countDownTimer : public Timer
{

	//functions
	public:
	countDownTimer();
	~countDownTimer();
	virtual	void startTimer();
	virtual	void startTimer(int four, int three , int two, int one);
	virtual	void startTimer(int six, int five, int four,int three , int two, int one);
	virtual	void init();
	virtual	void turnOnTimer();
	virtual	void turnOff();
	virtual	void stopTimer();
	virtual	elapsedMillis update(elapsedMillis _count);
	virtual boolean getlessThan10Seconds();
	virtual	boolean isRunning();
	virtual	boolean isOn();
	virtual	boolean hasEnded();
	
	virtual	uint8_t getHours();
	virtual	uint8_t getMinutes();
	virtual	uint8_t getSeconds();
	protected:
	private:
	countDownTimer( const countDownTimer &c );
	countDownTimer& operator=( const countDownTimer &c );

}; //countDownTimer
