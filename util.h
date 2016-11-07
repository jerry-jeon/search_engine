#ifndef _UTIL_
#define _UTIL_

#include <string>
#include <list>

using namespace std;

namespace util {
	struct FilePaths {
		FilePaths(string inputDirectory, string indexDirectory, string resultDirectory);
		FilePaths(char *argv[]);

		string inputDirectory;

		// inputDirectory
		string stopwordsFile;
		string queryFile;
		string articleFile(string type, int year, int month, int day);

		// indexDirectory
		string docFile;
		string termFile;
		string indexFile;

		//TODO These files should be deleted
		string preDocFile;
		string preTFFile;
		string preTermFile;
		string tfFile;
		string docyFile;
		
		// resultDirectory
		string resultVSMFile;
		string resultLMFile;
		// TODO remove above two lines and make user input to choose what model would be used
		string resultFile;
	};

	void startTimer();
	void endTimerAndPrint(string with);
	void logList(list<string> strings);
	string durationToString(long duration);
	int endTimerAndGetMinute();
	string getFileIntoString(string directory, string fileName);
	string getFileIntoString(string fileName);

}

#endif
