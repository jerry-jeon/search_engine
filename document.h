#ifndef _DOCUMENT_
#define _DOCUMENT_

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

using namespace std;

struct term {
	int id;
	int cf = 0;
	int df = 0;
	map<int, int> tf;
	string str;
};

void removePunctuation(string &str);
void removeNumberWords(list<string> &words);
bool isStopword(string word);
string concatStringList(list<string> words);

class Document {
public:
	Document(string _docno, string headline, string text); 

	static string outputDirectory;
	static map<string, int> wordIds;
	static vector<term*> wordList;
	int id;
	static int wordId;
	static list<string> stopwords;
	string docno;
	list<string> headlineWords;
	list<string> textWords;
	list<string> headlineStems;
	list<string> textStems;
	float denominator = 0;

	static int getDocumentNumber();
	list<string> tokenize(string str);
	void transform();
	void increaseDocumentFrequency();
	void stem(list<string> &stemList, list<string> words);
	void calculateDenominator(float dValue);

	void writeDocInfoFile();
	string toString();

	set<term*> words;
private:
	static int documentNumber;
};

#endif
