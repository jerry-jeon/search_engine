#include "text_processor.h"
#include <regex>

using namespace std;

namespace text_processor {
	void removePunctuation( string &str ) {
		char* charsToRemove = "%#!?;*$\\+@="; // <>
		for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
			str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
		}
	}

	void removeNumberWords( list<string> &words ) {
		regex reg(".*[0-9]+.*");
		list<string>::iterator iter = words.begin();
		while( iter != words.end()) {
			if(regex_match(*iter, reg)) {
				iter = words.erase(iter);
			} else
				iter++;
		}
	}

	bool isStopword(list<string> stopwords, string word) {
		list<string>::iterator iter = stopwords.begin();
		while( iter != stopwords.end()) {
			if(word == *iter)
				return true;
			iter++;
		}
		return false;
	}

	string concatStringList(list<string> words) {
		string result;
		list<string>::iterator iter = words.begin();
		result = *iter++;
		while( iter != words.end()) {
			result += " " + *iter;
			iter++;
		}
		return result;
	}
}
