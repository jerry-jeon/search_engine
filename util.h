#ifndef _UTIL_
#define _UTIL_

#include <string>
#include <list>

using namespace std;

namespace util {
	void startTimer();
	void endTimerAndPrint(string with);
	void logList(list<string> strings);
	string durationToString(long duration);
	int endTimerAndGetMinute();
	string getFileIntoString(string directory, string fileName);
	string getFileIntoString(string fileName);
}

#endif
