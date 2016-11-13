#include "include/document.h"
#include "include/util.h"
#include "include/text_processor.h"
#include "include/index_maker.h"

#include <stack>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cmath>
#include <iomanip>

using namespace util;
using namespace std;
using namespace std::chrono;

void transformFilesInFolder(FilePaths *filePaths, string type, vector<Document> &documentVector) {
	for(int year = 1998; year <= 2000; year++) {
		for(int month = 1; month <= 12; month++) {
			for(int day = 1; day <= 31; day++) {
				transformFile(filePaths->articleFile(type, year, month, day), documentVector);
			}
		}
	}
}

void transformFile(string filePath, vector<Document> &documentVector) {
	string fileString = getFileIntoString(filePath);

	if(fileString != "") {
		startTimer();
		parseToDocuments(fileString, documentVector);

		endTimerAndPrint("Refine complete...");
	}
}

void parseToDocuments(string fileString, vector<Document> &documentVector) {
	int docTagStartPosition = findDOCTagPosition(fileString, 0);
	while(docTagStartPosition != string::npos) {
		Document document = parseToDocument(fileString, docTagStartPosition);
		document.transform();
		document.increaseDocumentFrequency();
		documentVector.push_back(document);

		docTagStartPosition = findDOCTagPosition(fileString, docTagStartPosition);
	}
}

// return npos if doc doens't exist
int findDOCTagPosition(string fileString, int startPosition) {
	string startTag = "<DOC>";
	int result = fileString.find(startTag, startPosition);
	return result != string::npos ? result + startTag.length() : string::npos;
}

Document parseToDocument(string file, int docTagStartPosition) {
	string docno = extractContentInTag(file, "DOCNO", docTagStartPosition);
	string headline = extractContentInTag(file, "HEADLINE", docTagStartPosition);
	string text = extractContentInTag(file, "TEXT", docTagStartPosition);
	return Document (docno, headline, text);
}

string extractContentInTag(string fileString, string tag, int docTagStartPosition) {
	string startTag = "<" + tag + ">";
	string endTag = "</" + tag + ">";
	string result;
	int startPosition = fileString.find(startTag, docTagStartPosition) + startTag.size();
	int length = fileString.find(endTag, docTagStartPosition) - startPosition;
	result = fileString.substr(startPosition, length);
	return result;
}

void writeDocDataFile(FilePaths *filePaths, vector<Document> &documentVector) {
	ofstream documentFile (filePaths->docFile);
	vector<Document>::iterator documentIterator = documentVector.begin();
	while( documentIterator != documentVector.end()) {
		float denominator = 0.0f;
		set<Term*>::iterator wordIterator = documentIterator->words.begin();
		while(wordIterator != documentIterator->words.end()) {
			float dValue =	log((float)Document::getDocumentNumber() / (float)(*wordIterator)->df);

			denominator += pow((log((*wordIterator)->tf[documentIterator->id]) + 1.0f) * dValue, 2.0f);

			wordIterator++;
		}
		documentIterator->denominator = sqrt(denominator);
		documentFile << documentIterator->toString() << endl;
		documentIterator++;
	}
	documentFile.close();
}


void writeIndexFile(FilePaths *filePaths, vector<Document> documentVector) { // and term.dat
	cout << "Document size :  " << documentVector.size() << endl;
	ofstream tfFile (filePaths->tfFile);
	ofstream termFile (filePaths->termFile);
	ofstream indexFile (filePaths->indexFile);
	int lineCount = 0;
	int size = Document::wordList.size();
	unsigned long long pre_df = 0;
	unsigned long long index_line = 26;

	for(int i = 0; i < Document::wordList.size(); i++) {
		if(i % 500 == 0)
			startTimer();
		Term temp = *Document::wordList[i];
		
		int documentFrequency = temp.df;
		float dValue =	log((float)Document::getDocumentNumber() / (float)documentFrequency);
		if(dValue == 0)
			continue;
		map<int, int>::iterator tfIterator = temp.tf.begin();

		while(tfIterator != temp.tf.end()) {
	
			int termFrequency = tfIterator->second;
			float numerator = (log((float)termFrequency) + 1.0f) * dValue;	//get doc.data and denominator
			float weight = numerator / documentVector[tfIterator->first - 1].denominator; // is denominator
			if(documentVector[tfIterator->first - 1].denominator == 0) {
				cout << "denominator is zero" << endl;
			}
					// get figure of weight
			int temp_i = (int)weight;
			int count = 0;
			do {
				temp_i = int(temp_i / 10);
				count++;
			} while(temp_i > 0);

			tfFile << temp.id << '\t' << temp.str << '\t' << temp.df << '\t' << temp.cf << endl;
			indexFile << setfill('0') << setw(6) << temp.id << setfill('0') << setw(6) << to_string(documentVector[tfIterator->first - 1].id) << setfill('0') << setw(5) << termFrequency << setfill('0') << setw(count) << fixed << setprecision(7 - count) << weight << endl;
			documentVector[tfIterator->first - 1].weightSum += pow(weight, 2);
			tfIterator++;
		}

		termFile << temp.id << '\t' << temp.str << '\t' << temp.df << '\t' << temp.cf << '\t' << pre_df * index_line << endl;;
		pre_df += temp.df;
		if(i % 500 == 0) {
			cout << temp.id << " / " << size << "   " << (float)((float)i / (float)size) * 100 << "% proceeding" << endl;
			if(endTimerAndGetMinute() > 0) {
				cout << "Word / Minute speed : " << (int)(i / endTimerAndGetMinute()) << endl;
			}
		}
	}
	indexFile.close();
	termFile.close();
	tfFile.close();

	ofstream documentFile (filePaths->docFile);
	vector<Document>::iterator documentIterator = documentVector.begin();
	while( documentIterator != documentVector.end()) {
		documentFile << documentIterator->toString() << "\t" << sqrt(documentIterator->weightSum) << endl;
		documentIterator++;
	}
	documentFile.close();
}
