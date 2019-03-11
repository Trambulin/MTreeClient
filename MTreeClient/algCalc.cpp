#include"algCalc.h"

std::mutex algCalc::calcThreadMtx;
std::condition_variable algCalc::signalConVar;

algCalc::algCalc()
{
	threadOver = false; calcOver = false;
	cycleTime = 0;
}

algCalc::~algCalc()
{
}

void algCalc::overThread()
{
	calcOver = true; threadOver = true;
}

void algCalc::threadCalcRun(algCalc *aC)
{
	while (1) {
		std::unique_lock<std::mutex> lock(algCalc::calcThreadMtx);
		//algCalc::signalConVar.wait(lock);
		aC->runAlgorithm();
		if (aC->threadOver) {
			break;
		}
		//aC->runScrypt(aC->firstVal, aC->lastVal);
	}
}