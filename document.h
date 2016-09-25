#ifndef _DOCUMENT_
#define _DOCUMENT_

#include <string>
#include <list>
#include <map>

using namespace std;

void removePunctuation(string &str);
void removeNumberWords(list<string> &words);
bool isStopword(string word);
void stem(list<string> &stemList, list<string> words);

class Document {
public:
	static list<string> stopwords;
	string docno;
	list<string> headlineWords;
	list<string> textWords;
	map<string, int> frequencies;
	list<string> headlineStems;
	list<string> textStems;
	int maxFrequency = 0;

	Document(string _docno, string headline, string text) {
		docno = _docno;
		removePunctuation(headline);
		removePunctuation(text);
		headlineWords = tokenize(headline);
		textWords = tokenize(text);
	}

	list<string> tokenize(string str);
	float termFrequency(string term);
	float idf(string term, list<Document> documents);
	float tfidf(string term, list<Document> documents);
	bool contain(string term);
	void transform();

	string to_string();
};

#endif