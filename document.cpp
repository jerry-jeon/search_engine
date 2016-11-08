#include "document.h"
#include "text_processor.h"
#include "porter2_stemmer.h"
#include <stack>
#include <algorithm>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cmath>
#include <set>

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time

TextProcessor Document::textProcessor;
string Document::outputDirectory = "";
int Document::documentNumber = 0;
map<string, int> Document::wordIds = {{"asd", 0}};
vector<term*> Document::wordList;
int Document::wordId = 0;
term* getWord(string word);

Document::Document(string _docno, string headline, string text) {
	id = ++documentNumber;
	docno = _docno;
	textProcessor.removePunctuation(headline);
	textProcessor.removePunctuation(text);
	headlineWords = textProcessor.tokenize(headline);
	textWords = textProcessor.tokenize(text);
}

Document::Document(int _id, string _docno, float _denominator) {
	id = _id;
	docno = _docno;
	denominator = _denominator;
}

int Document::getDocumentNumber() {
	return documentNumber;
}



void Document::increaseDocumentFrequency() {
	set<term*>::iterator iter = words.begin();
	while( iter != words.end()) {
		(*iter)->df++;
		iter++;
	}
}


void Document::transform() {
	textProcessor.removeNumberWords(headlineWords);
	textProcessor.removeNumberWords(textWords);
	stem(headlineStems, headlineWords);
	stem(textStems, textWords);

	//headlineWords.unique();
	//textStems.unique();
}

void Document::stem(list<string> &stemList, list<string> words) {
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		string word = *iter;
		Porter2Stemmer::trim(word);
		Porter2Stemmer::stem(word);

		if(!word.empty()) {
			stemList.push_back(word);
			term *temp = getWord(word);
			temp->cf++;;
			temp->tf[id]++;

			this->words.insert(temp);
		}

		iter++;
	}
}

term* getWord(string word) {
	if(Document::wordIds[word] == 0) {
		term *temp = new term;
		temp->id = ++Document::wordId;
		Document::wordIds[word] = Document::wordId;
		temp->str = word;
		Document::wordList.push_back(temp);
		return temp;
	} else {
		return Document::wordList[Document::wordIds[word] - 1];
	}
}

void Document::writeDocInfoFile() {
	ofstream outputFile (outputDirectory + "/doc.dat", ios_base::app);
	outputFile.close();
}

string Document::preFileString() {
	return to_string(id) + "\t" + docno + "\t" + to_string(words.size()) + "\t" + to_string(denominator);
}

string Document::toString() {
	return to_string(id) + "\t" + docno + "\t" + to_string(words.size());
}

void Document::calculateDenominator(float dValue) { // tf idf formula's denominator
	set<term*>::iterator iter = words.begin();
	while( iter != words.end()) {
		denominator += pow((log((*iter)->tf[id]) + 1.0f) * dValue, 2.0f);
		iter++;
	}
	denominator = sqrt(denominator);
}
