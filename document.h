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

struct Term {
	Term(string* tokens);

	int id;
	string word;
	int df;
	int cf;
	unsigned long long indexStart;
	bool operator==(const Term &other) const {
		return id == other.id;
	}
};

struct Index {
	Index(string indexFileLine);
	Term *term;
	int termId;
	int docId;
	int tf;
	float weight;
};


class Document {
public:
	Document(string _docno, string headline, string text); 
	Document(int _id, string _docno, float _denominator); 
	Document(string* tokens);

	static string outputDirectory;
	static map<string, int> wordIds;
	static vector<term*> wordList;
	static TextProcessor textProcessor;
	int id;
	int size;
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

	//TODO arrange variables
	list<Index*> indexes;

	set<Term*> s_words; // this is for language model

	bool operator==(const Document &other) {
		return id == other.id;
	}
private:
	static int documentNumber;
};

#endif
