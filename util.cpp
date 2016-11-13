#include "include/util.h"

#include <list>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <stack>

using namespace std;
using namespace std::chrono;


namespace util {
	stack<high_resolution_clock::time_point> startTimeStack;
	string whitespaces (" \t\f\v\n\r");

	FilePaths::FilePaths(string inputDirectory, string indexDirectory, string resultDirectory) {
		this->inputDirectory = inputDirectory;
		stopwordsFile = inputDirectory + "/stopwords.txt";
		queryFile = inputDirectory + "/topics25.txt";

		docFile = indexDirectory + "/doc.dat";
		termFile = indexDirectory + "/term.dat";
		indexFile = indexDirectory + "/index.dat";
		tfFile = indexDirectory + "/tf.dat";

		resultFile = resultDirectory + "/result.txt";
		relevantFile = resultDirectory + "/relevant_document.txt";
	}

	FilePaths::FilePaths(char* argv[]) : FilePaths(string(argv[1]), string(argv[2]), string(argv[3])) {}

	string FilePaths::articleFile(string type, int year, int month, int day) {
		// make file name
		char buffer[100];
		if(type == "APW") {
			std::snprintf(buffer, sizeof(buffer), "%d%02d%02d_APW_ENG", year, month, day);
		} else if(type == "NYT") {
			std::snprintf(buffer, sizeof(buffer), "%d%02d%02d_NYT", year, month, day);
		} else {
			//TODO throw error
		}
		string fileName = string(buffer);

		// total path
		return inputDirectory + "/" + type + "/" + to_string(year) + "/" + fileName;
	}

	void startTimer() {
		startTimeStack.push(high_resolution_clock::now());
	}

	void endTimerAndPrint(string with) {
		auto duration = duration_cast<milliseconds>( high_resolution_clock::now() - startTimeStack.top() ).count();
		cout << with << endl;
		cout << durationToString(duration) << endl;
		startTimeStack.pop();
	}

	string durationToString(long duration) {
		string result = "";
		int deciSecond = (duration / 100) % 10;
		int second = (duration / 1000) % 60;
		int minute = (duration / (1000 * 60)) % 60;
		int hour = (duration / (1000 * 60 * 60)) % 24;

		if(hour > 0)
			result += to_string(hour) + "h ";
		if(minute > 0)
			result += to_string(minute) + "m ";
		result += to_string(second) + "." + to_string(deciSecond) + "s was taken.";

		return result;
	}

	void logList(list<string> strings) {
		list<string>::iterator iter = strings.begin();
		while( iter != strings.end()) {
			cout << *iter << endl;
			iter++;
		}
	}

	int endTimerAndGetMinute() {
		auto duration = duration_cast<milliseconds>( high_resolution_clock::now() - startTimeStack.top() ).count();
		startTimeStack.pop();

		return duration / (1000 * 60);
	}

	string getFileIntoString(string directory, string fileName) {
		string line = "", result = "";
		ifstream file (directory + fileName);
		if(file.is_open()) {
			result.assign( (istreambuf_iterator<char>(file) ), (istreambuf_iterator<char>()));
			file.close();
			cout << "Read complete - " << fileName << endl;
		}
		return result;
	}

	string getFileIntoString(string fileName) {
		string line = "", result = "";
		ifstream file (fileName);
		if(file.is_open()) {
			result.assign( (istreambuf_iterator<char>(file) ), (istreambuf_iterator<char>()));
			file.close();
			cout << "Read complete - " << fileName << endl;
		}
		return result;
	}
	
	void trim(string &str) {
		rightTrim(str);
		leftTrim(str);
	}

	// TODO clear 처리
	void rightTrim(string &str) {
	  size_t found = str.find_last_not_of(whitespaces);
		if (found!=string::npos) {
	    str.erase(found+1);
	  } else {
	    str.clear();
	  }
	}
	
	void leftTrim(string &str) {
		str.erase(0, str.find_first_not_of(whitespaces));
	}

}
