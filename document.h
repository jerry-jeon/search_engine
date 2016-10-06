#ifndef _DOCUMENT_
#define _DOCUMENT_

#include <string>
#include <list>
#include <map>

using namespace std;

void removePunctuation(string &str);
void removeNumberWords(list<string> &words);
bool isStopword(string word);
string concatStringList(list<string> words);

class Document {
public:
	Document(string _docno, string headline, string text); 

	static string outputDirectory;
	static map<string, int> documentFrequencies;
	static map<string, int> collectionFrequencies;
	int id;
	static list<string> stopwords;
	string docno;
	list<string> headlineWords;
	list<string> textWords;
	map<string, int> termFrequencies;
	list<string> headlineStems;
	list<string> textStems;
	float denominator = 0;

	static int getDocumentNumber();
	list<string> tokenize(string str);
	bool contain(string term);
	void transform();
	void increaseDocumentFrequency();
	void stem(list<string> &stemList, list<string> words);
	void calculateDenominator(float dValue);

	void writeDocInfoFile();
	string toString();
private:
	static int documentNumber;
};

#endif
