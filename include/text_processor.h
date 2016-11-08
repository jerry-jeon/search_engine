#ifndef _TEXTPROCESSOR_
#define _TEXTPROCESSOR_

#include <string>
#include <list>
#include <map>

using namespace std;
class TextProcessor {
	public:
		TextProcessor();
		TextProcessor(string stopwordsFile);

		list<string> stopwords;

		void initializeStopwords(string stopwordsFile);
		void removePunctuation(string &str);
		void removeNumberWords(list<string> &words);
		bool isStopword(string word);
		string concatStringList(list<string> words);
		list<string> tokenize(string str);
		map<string, int> stem(list<string> words);
		map<string, int> stringToRefinedStems(string str);

};

#endif
