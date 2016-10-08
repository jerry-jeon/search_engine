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
vector<Document> documentList;
void writeIndexFile();
void writeDocDataFile();
void writeTFFile();


int main(int argc, char *argv[]) {
	if(handleArguments(argc, argv) == -1) {
		return -1;
	} else {
		startTimer();
		initializeStopwords();

		startTimer();
		//transformFile("APW", 2000, 9, 30);
		//transformFile("NYT", 2000, 8, 30);
		//transformFile("NYT", 2000, 8, 29);
		//transformFile("NYT", 2000, 8, 28);
		//transformFile("NYT", 2000, 8, 27);
		transformFilesInFolder("APW");
		transformFilesInFolder("NYT");
		endTimerAndPrint("Reading input file -------------------------------------");
		
		cout << "Start cacluate denimonator and write document file..." << endl;
		startTimer();
		writeDocDataFile();
		endTimerAndPrint("Writing document file -------------------------------------");
		
		startTimer();
		cout << "Start write index and term file..." << endl;
		writeIndexFile();
		endTimerAndPrint("Writing index file -------------------------------------");
	
		//writeTFFile();

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

		endTimerAndPrint("Refine complete...");
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


void writeTFFile() {
	ofstream preDocFile (outputDirectory + "/pre_doc.dat");
	ofstream preTermFile (outputDirectory + "/pre_term.dat");
	ofstream preTFFile (outputDirectory + "/pre_tf.dat");
	int df_total = 0;
	for(int i = 0; i < Document::wordList.size(); i++) {
		term temp = *Document::wordList[i];
		preTermFile << i << '\t' << temp.str << '\t' << temp.df << '\t' << temp.cf << '\t' << df_total * 23 << endl;
		df_total += temp.df;
	}

	for(int i = 0; i < Document::wordList.size(); i++) {
		term temp = *Document::wordList[i];
		map<int, int>::iterator iter = temp.tf.begin();
		while(iter != temp.tf.end()) {
			
			preTFFile << i << '\t' << temp.str << '\t' << iter->first << '\t' << iter->second << endl;;
			iter++;
		}
	}

	preDocFile.close();
	preTermFile.close();
	preTFFile.close();
}


void writeDocDataFile() {
	ofstream documentFile (outputDirectory + "/doc.dat");
	vector<Document>::iterator documentIterator = documentList.begin();
	while( documentIterator != documentList.end()) {
		float denominator = 0.0f;
		set<term*>::iterator wordIterator = documentIterator->words.begin();
		while(wordIterator != documentIterator->words.end()) {
			float dValue =	log(Document::getDocumentNumber() / (*wordIterator)->df);

			denominator += pow((log((*wordIterator)->tf[documentIterator->id]) + 1.0f) * dValue, 2.0f);

			wordIterator++;
		}
		documentIterator->denominator = sqrt(denominator);
		documentFile << documentIterator->toString() << endl;
		documentIterator++;
	}
	documentFile.close();
}


void writeIndexFile() { // and term.dat
	ofstream indexFile (outputDirectory + "/index.dat");
	int lineCount = 0;
	int size = Document::wordList.size();

	for(int i = 0; i < Document::wordList.size(); i++) {
		if(i % 2000 == 0)
			startTimer();
		term temp = *Document::wordList[i];
		
		int documentFrequency = temp.df;
		float dValue =	log(Document::getDocumentNumber() / documentFrequency);
		map<int, int>::iterator tfIterator = temp.tf.begin();

		while(tfIterator != temp.tf.end()) {
	
			int termFrequency = tfIterator->second;
			float numerator = (log((float)termFrequency) + 1.0f) * dValue;	//get doc.data and denominator
			float weight = numerator / documentList[tfIterator->first - 1].denominator; // is denominator
			// get figure of weight
			int temp_i = (int)weight;
			int count = 0;
			do {
				temp_i = int(temp_i / 10);
				count++;
			} while(temp_i > 0);

			indexFile << setfill('0') << setw(6) << temp.id << setfill('0') << setw(6) << to_string(documentList[tfIterator->first - 1].id) << setfill('0') << setw(3) << termFrequency << setfill('0') << setw(count) << fixed << setprecision(7 - count) << weight << endl;
			tfIterator++;
		}

		if(i % 2000 == 0) {
			cout << temp.id << " / " << size << "   " << i / size * 100 << "% 진행중" << endl;
			cout << "??" << endl;
			if(endTimerAndGetMinute() > 0) {
				cout << "Word / Minute speed : " << (int)(i / endTimerAndGetMinute()) << endl;
			}
			cout << "???" << endl;
		}
	}
	indexFile.close();
}
