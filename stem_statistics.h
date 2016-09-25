#ifndef _STEM_STATISTICS_
#define _STEM_STATISTICS_

#include <string>
#include <list>

using namespace std;

class StemStatistics {
public:
	string stem;
	list<string> words;
	int count = 0;

	StemStatistics(string _stem);
	void addCount(string word);

	string to_string();
	bool operator==(StemStatistics stemStatistics);
};

string concatStringList(list<string> words);
void addStemStatistics(string stem, string word);
bool compareStemStatistics(StemStatistics s1, StemStatistics s2);
void writeStemStatistics();

#endif

