#pragma once
#include<mutex>
#include<condition_variable>

class algCalc
{
public:
	algCalc();
	~algCalc();

	bool threadOver;
	long long int cycleTime;

	void overThread();
	virtual void runAlgorithm() = 0;

	static std::mutex calcThreadMtx;
	static std::condition_variable signalConVar;
	static void threadCalcRun(algCalc *aC);
protected:
	bool calcOver;
private:

};