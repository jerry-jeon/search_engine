#include "document.h"
#include "util.h"

#include <stack>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <map>
#include <cmath>
#include <iomanip>

using namespace util;
using namespace std;
using namespace std::chrono;

string inputDirectory, outputDirectory, stopwordsFile;
int handleArguments(int argc, char* argv[]);
void initializeStopwords();

string makeFileName(string type, int year, int month, int day);
string getFileIntoString(string directory, string fileName);
void transformFilesInFolder(string type);
void transformFile(string type, int year, int month, int day);
int findDOCTagPosition(string fileString, int startPosition);

void parseToDocuments(string fileString);
Document parseToDocument(string file, int docTagStartPosition);
string extractContentInTag(string fileString, string tag, int docTagStartPosition);
list<Document> documentList;
void writeTermInfoFile(); 
void writeIndexFile();
void writeDocDataFile();


int main(int argc, char *argv[]) {
	if(handleArguments(argc, argv) == -1) {
		return -1;
	} else {
		startTimer();
		initializeStopwords();

		transformFile("APW", 2000, 9, 30);
		//transformFile("NYT", 2000, 8, 30);
		//transformFilesInFolder("APW");
		//transformFilesInFolder("NYT");
		
		//writeTermInfoFile();
		cout << "Start cacluate denimonator and write document file..." << endl;
		startTimer();
		writeDocDataFile();
		endTimerAndPrint("Writing document file -------------------------------------");
		startTimer();
		cout << "Start write index and term file..." << endl;
		writeIndexFile();
		endTimerAndPrint("Writing index file -------------------------------------");

		endTimerAndPrint("All time -------------------------------------");
		return 0;
	}

}

// return -1 if argument is not valid
int handleArguments(int argc, char* argv[]) {
	if(argc < 4) {
		cout << "다음의 형태로 사용해주세요" << endl;
		cout << argv[0] << " input폴더 output폴더 stopword파일" << endl;
		cout << "ex) " << argv[0] << " input/ output/ stopwords.txt" << endl;
		return -1;
	} else {
		inputDirectory = argv[1];
		outputDirectory = argv[2];
		Document::outputDirectory = argv[2];
		stopwordsFile = argv[3];
		return 0;
	}
	return 1;
}

void initializeStopwords() {
	list<string> stopwords;
	string line;
	ifstream file (stopwordsFile);
	if(file.is_open()) {
		while(getline(file, line)) {
			if(!line.empty())
				stopwords.push_back(line);
		}
		file.close();
	}
	Document::stopwords = stopwords;
}


void transformFilesInFolder(string type) {
	for(int year = 1998; year <= 2000; year++) {
		for(int month = 1; month <= 12; month++) {
			for(int day = 1; day <= 31; day++) {
				transformFile(type, year, month, day);
			}
		}
	}
}

void transformFile(string type, int year, int month, int day) {
	string fileName = makeFileName(type, year, month, day);
	string fileString = getFileIntoString(inputDirectory + type + "/" + to_string(year) + "/", fileName);

	if(fileString != "") {
		startTimer();
		parseToDocuments(fileString);

		endTimerAndPrint("Refine complete - " + fileName);
	}
}

string makeFileName(string type, int year, int month, int day) {
	char buffer[100];
	if(type == "APW") {
		std::snprintf(buffer, sizeof(buffer), "%d%02d%02d_APW_ENG", year, month, day);
	} else if(type == "NYT") {
		std::snprintf(buffer, sizeof(buffer), "%d%02d%02d_NYT", year, month, day);
	} else {
		//TODO throw error
	}
	return string(buffer);
}


string getFileIntoString(string directory, string fileName) {
	string line, result = "";
	ifstream file (directory + fileName);
	if(file.is_open()) {
		result.assign( (istreambuf_iterator<char>(file) ), (istreambuf_iterator<char>()));
		file.close();
		cout << "Read complete - " << fileName << endl;
	}
	return result;
}

void parseToDocuments(string fileString) {
	int docTagStartPosition = findDOCTagPosition(fileString, 0);
	while(docTagStartPosition != string::npos) {
		Document document = parseToDocument(fileString, docTagStartPosition);
		document.transform();
		document.increaseDocumentFrequency();
		documentList.push_back(document);

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

void writeTermInfoFile() {
	ofstream outputFile (outputDirectory + "term.dat");
	map<string, int>::iterator iter = Document::collectionFrequencies.begin();
	int count = 0;
	while( iter != Document::collectionFrequencies.end()) {
		outputFile << ++count << "\t" << iter->first << "\t" << Document::documentFrequencies[iter->first] << "\t" << iter->second << endl;
		iter++;
	}
	outputFile.close();
} 

void writeDocDataFile() {
	ofstream documentFile (outputDirectory + "/doc.dat");
	list<Document>::iterator documentIterator = documentList.begin();
	while( documentIterator != documentList.end()) {
		float denominator = 0.0f;
		map<string, int>::iterator tfIterator = documentIterator->termFrequencies.begin();
		while( tfIterator != documentIterator->termFrequencies.end()) {
			int documentFrequency = Document::documentFrequencies[tfIterator->first];
			float dValue =	log(Document::getDocumentNumber() / documentFrequency);

			denominator += pow((log(tfIterator->second) + 1.0f) * dValue, 2.0f);
			tfIterator++;
		}
		documentFile << documentIterator->toString() << endl;
		documentIterator->denominator = sqrt(denominator);
		documentIterator++;
	}
	documentFile.close();
}

void writeIndexFile() { // and term.dat
	ofstream indexFile (outputDirectory + "/index.dat");
	ofstream termFile (outputDirectory + "/term.dat");
	int wordId = 1;
	int lineCount = 0;

	map<string, int>::iterator wordIterator = Document::collectionFrequencies.begin();
	while( wordIterator != Document::collectionFrequencies.end()) {
		string word = wordIterator->first;
		
		int documentFrequency = Document::documentFrequencies[word]; // should change
		float dValue =	log(Document::getDocumentNumber() / documentFrequency);

		list<Document>::iterator documentIterator = documentList.begin();
		while( documentIterator != documentList.end()) {
			if(documentIterator->contain(word)) {
				int termFrequency = documentIterator->termFrequencies[word];
				float numerator = (log((float)termFrequency) + 1.0f) * dValue;	//get doc.data and denominator
				float weight = numerator / documentIterator->denominator; // is denominator
				// get figure of weight
				int temp = (int)weight;
				int count = 0;
				do {
					temp = int(temp / 10);
					count++;
				} while(temp > 0);

				indexFile << setfill('0') << setw(5) << wordId << setfill('0') << setw(6) << documentIterator->id << setfill('0') << setw(3) << termFrequency << setfill('0') << setw(count) << fixed << setprecision(7 - count) << weight << endl;
				lineCount++;
			}
			documentIterator++;
		}
		termFile << wordId++ << '\t' << word << '\t' << Document::documentFrequencies[word] << '\t' << wordIterator->second << '\t' << lineCount * 22 << endl;
		wordIterator++;
	}
	indexFile.close();
}
