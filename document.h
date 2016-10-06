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

struct WordInfo {
	static int lastId;
	int id;
	string word;
	int df;
	int cf;
};

class Document {
public:
	static int documentNumber;
	static int idMaker;
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
	int maxFrequency = 0;
	float denominator = 0;

	Document(string _docno, string headline, string text); 

	list<string> tokenize(string str);
	float termFrequency(string term);
	float idf(string term, list<Document> documents);
	float tfidf(string term, list<Document> documents);
	bool contain(string term);
	void transform();
	void addWord(string temp);
	void addDf();
	void stem(list<string> &stemList, list<string> words);
	void calculateDenominator(float dValue);
	void writeTFFile();

	string to_string();
	void makeDocInfoFile();
};

#endif
