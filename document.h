#ifndef _DOCUMENT_
#define _DOCUMENT_

#include "text_processor.h"
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

class Document {
public:
	Document(string _docno, string headline, string text); 
	Document(int _id, string _docno, float _denominator); 

	static string outputDirectory;
	static map<string, int> wordIds;
	static vector<term*> wordList;
	static TextProcessor textProcessor;
	int id;
	float weightSum;
	static int wordId;
	string docno;
	list<string> headlineWords;
	list<string> textWords;
	list<string> headlineStems;
	list<string> textStems;
	float denominator = 0;

	static int getDocumentNumber();
	void transform();
	void increaseDocumentFrequency();
	void stem(list<string> &stemList, list<string> words);
	void calculateDenominator(float dValue);

	void writeDocInfoFile();
	string toString();
	string preFileString();

	set<term*> words;
private:
	static int documentNumber;
};

#endif
