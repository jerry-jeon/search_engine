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

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time

#include "document.h"
using namespace std;
using namespace std::chrono;


stack<high_resolution_clock::time_point> startTimeStack;
void startTimer();
void endTimerAndPrint(string with);
void logList(list<string> strings);
string durationToString(long duration);
void makeWordStatistics(list<string> words);
void makeStemStatistics(list<string> stems);
//these above functions and variables are for development.

string inputDirectory, outputDirectory, stopwordsFile;
int handleArguments(int argc, char* argv[]);
void initializeStopwords();

string makeFileName(string type, int year, int month, int day);
string getFileIntoString(string fileName);
void transformFilesInFolder(string type);
void transformFile(string type, int year, int month, int day);
int findDOCTagPosition(string fileString, int startPosition);

list<Document> parseToDocuments(string fileString);
Document parseToDocument(string file, int docTagStartPosition);
string extractContentInTag(string fileString, string tag, int docTagStartPosition);
void writeDocInfoFile(string fileName, list<Document> document);
void writeTermInfoFile(); 
void writeIndexFile();
void writeDocDataFile();


void startTimer() {
	startTimeStack.push(high_resolution_clock::now());
}

void endTimerAndPrint(string with) {
	auto duration = duration_cast<milliseconds>( high_resolution_clock::now() - startTimeStack.top() ).count();
	cout << with << endl;
	cout << durationToString(duration) << endl;
	startTimeStack.pop();
}

string durationToString(long duration) {
	string result = "";
	int deciSecond = (duration / 100) % 10;
	int second = (duration / 1000) % 60;
	int minute = (duration / (1000 * 60)) % 60;
	int hour = (duration / (1000 * 60 * 60)) % 24;

	if(hour > 0)
		result += to_string(hour) + "시간 ";
	if(minute > 0)
		result += to_string(minute) + "분 ";
	result += to_string(second) + "." + to_string(deciSecond) + "초 걸렸습니다.";

	return result;
}

void logList(list<string> strings) {
	list<string>::iterator iter = strings.begin();
	while( iter != strings.end()) {
		cout << *iter << endl;
		iter++;
	}
}
//these above functions are for development.

int main(int argc, char *argv[]) {
	if(handleArguments(argc, argv) == -1) {
		return -1;
	} else {
		startTimer();
		initializeStopwords();

		//transformFile("APW", 2000, 9, 30);
		//transformFile("NYT", 2000, 8, 30);
		transformFilesInFolder("APW");
		transformFilesInFolder("NYT");
		
		//writeTermInfoFile();
		writeDocDataFile();
		writeIndexFile();

		endTimerAndPrint("구동시간-------------------------------------");
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
	string fileString = getFileIntoString(inputDirectory + type + "/" + to_string(year) + "/" + fileName);

	if(fileString != "") {
		startTimer();
		list<Document> documentList = parseToDocuments(fileString);
		list<Document>::iterator iter = documentList.begin();
		while( iter != documentList.end()) {
			//iter->makeDocInfoFile();
			iter->writeTFFile();
			iter->addDf();
			iter++;
		}
		//writeDocInfoFile(fileName, documentList); // first homework
		endTimerAndPrint("refine complete " + fileName);
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


string getFileIntoString(string fileName) {
	string line, result = "";
	ifstream file (fileName);
	if(file.is_open()) {
		result.assign( (istreambuf_iterator<char>(file) ), (istreambuf_iterator<char>()));
		file.close();
		cout << "read complete " << fileName << endl;
	}
	return result;
}

list<Document> parseToDocuments(string fileString) {
	list<Document> documentList;
	int docTagStartPosition = findDOCTagPosition(fileString, 0);
	while(docTagStartPosition != string::npos) {
		Document document = parseToDocument(fileString, docTagStartPosition);
		document.transform();
		documentList.push_back(document);
		document.addDf();

		docTagStartPosition = findDOCTagPosition(fileString, docTagStartPosition);
	}

	return documentList;
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
	// 통계내기위한 코드
	//writeHighRankedTfIdfWords(documentList);
	//writeStemStatistics();
} 

void writeDocInfoFile(string fileName, list<Document> documentList) {
	ofstream outputFile (outputDirectory + fileName);
	list<Document>::iterator iter = documentList.begin();
	while( iter != documentList.end()) {
		outputFile << iter->to_string() << endl;
		iter++;
	}
	outputFile.close();
	// 통계내기위한 코드
	//writeHighRankedTfIdfWords(documentList);
	//writeStemStatistics();
}

void writeDocDataFile() {
	map<string, int> termFrequencies;
	ifstream file (outputDirectory + "/tf.dat");
	ofstream outputFile (outputDirectory + "/doc.dat");
	int id = 0;
	string docno;
	float denominator;

	string line; 

	// dirty code. should refactoring
	getline(file, line);
	vector<string> token;
	stringstream linestream(line);
	for(string item; getline(linestream, item, '\t');) {
		token.push_back(item);
	}
	termFrequencies[token[2]] =  atoi(token[3].c_str());
	docno = token[1];

	while(getline(file, line)) {
		vector<string> docToken;
		stringstream linestream(line);
		for(string item; getline(linestream, item, '\t');) {
			docToken.push_back(item);
		}
		if(docno != token[1]) { // different document
			map<string, int>::iterator iter = termFrequencies.begin();
			while( iter != termFrequencies.end()) {
				int documentFrequency = Document::documentFrequencies[iter->first];
				int documentNumber = Document::idMaker;
				float dValue =	log(documentNumber / documentFrequency);
				denominator += pow((log(iter->second) + 1.0f) * dValue, 2.0f);
				iter++;
			}
			denominator = sqrt(denominator);
			// calc denominator
			outputFile << to_string(++id) << "\t" << docno << "\t" << termFrequencies.size() << "\t" << denominator << endl;

			termFrequencies.clear();
			docno = token[1];
		}

		termFrequencies[token[2]] =  atoi(token[3].c_str());
	}  
}

void writeIndexFile() { // and term.dat

	int documentSize;
	ofstream indexFile (outputDirectory + "/index.dat");
	ofstream termFile (outputDirectory + "/term.dat");
	ifstream docFile (outputDirectory + "/doc.dat");
	map<string, int>::iterator iter = Document::collectionFrequencies.begin();
	int count = 0;
	string docno;
	int wordId = 1;
	int lineCount;


	while( iter != Document::collectionFrequencies.end()) {
		string word = iter->first;
		float documentFrequency = Document::documentFrequencies[word];
		float documentNumber = Document::idMaker;
		float dValue = log(documentNumber / documentFrequency);
		//iterate document file
		cout << word << endl;
		int termFrequency;
		ifstream tfFile (outputDirectory + "/tf.dat");
		for(string line; getline(tfFile, line);) {
			vector<string> token;
			stringstream linestream(line);
			for(string item; getline(linestream, item, '\t');) {
				token.push_back(item);
			}

			if(token[2] == word) {
				docno = token[1];
				termFrequency = atoi(token[3].c_str());
				string docLine;
				while(getline(docFile, docLine)) {
					string::size_type sz;
					vector<string> docToken;
					stringstream linestream(docLine);
					for(string item; getline(linestream, item, '\t');) {
						docToken.push_back(item);
					}
					if(docno == docToken[1]) {
						float numerator = (log((float)termFrequency) + 1.0f) * dValue;	//get doc.data and denominator
						float weight = numerator / stof(docToken[3], &sz); // is denominator
						// get figure of weight
						int temp = (int)weight;
						int count = 0;
						do {
							temp = int(temp / 10);
							count++;
						} while(temp > 0);

						indexFile << setfill('0') << setw(5) << wordId << setfill('0') << setw(6) << token[0] << setfill('0') << setw(3) << termFrequency << setfill('0') << setw(count) << fixed << setprecision(7 - count) << weight << endl;
						lineCount++;
					}
				}
				docFile.clear();
				docFile.seekg(0);
				termFile << wordId++ << '\t' << word << '\t' << Document::documentFrequencies[word] << '\t' << iter->second << '\t' << lineCount * 22 << endl;
			}
		}
		tfFile.close();


		iter++;
	}
	indexFile.close();
	docFile.close();
	termFile.close();
	// 통계내기위한 코드
	//writeHighRankedTfIdfWords(documentList);
	//writeStemStatistics();
	// get word.
	string word;
}

float calculateWeight(Document document, int tf, int df, int docSize) {
	float denominator;

	map<string, int>::iterator iter = document.termFrequencies.begin();
	while( iter != document.termFrequencies.end()) {
		float dValue = log(docSize / df);
		float weight = ((log(iter->second) + 1.0f) * dValue) /	document.denominator;

		iter++;
	}
}
