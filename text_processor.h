#ifndef _TEXTPROCESSOR_
#define _TEXTPROCESSOR_

#include <string>
#include <list>

using namespace std;
namespace text_processor {
	void removePunctuation(string &str);
	void removeNumberWords(list<string> &words);
	bool isStopword(list<string> stopwords, string word);
	string concatStringList(list<string> words);
}

#endif
