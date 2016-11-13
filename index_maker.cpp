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


void writeTFFile(FilePaths *filePaths, vector<Document> documentVector) {
	ofstream preDocFile (filePaths->preDocFile);
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
		preDocFile << documentIterator->toString() << endl;
		cout << documentIterator->id << " / " << documentVector.size() << endl;
		documentVector.erase(documentIterator);
	}

	cout << "complete preDocFile" << endl;

	ofstream preTFFile (filePaths->preTFFile);
	int size = Document::wordList.size();
	for(int i = 0; i < Document::wordList.size(); i++) {
		Term temp = *Document::wordList[i];
		map<int, int>::iterator iter = temp.tf.begin();
		while(iter != temp.tf.end()) {
			preTFFile << i << '\t' << temp.str << '\t' << iter->first << '\t' << iter->second << endl;;
			iter++;
		}
		cout << i << " / " << size << endl;
	}
	preTFFile.close();
	cout << "complete preTFFile" << endl;

	preDocFile.close();
	cout << "hmm..." << endl;
	int j = 0;
	ofstream preTermFile (filePaths->preTermFile);
	vector<Term*>::iterator termIter = Document::wordList.begin();
	while(termIter != Document::wordList.end()) {
		Term *temp = *termIter;
		preTermFile << temp->id << '\t' << temp->str << '\t' << temp->df << '\t' << temp->cf << endl;
		cout << temp->id << " / " << size << endl;
		termIter++;
	}

	preTermFile.close();
	cout << "complete preTermFile" << endl;

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

void readFiles(FilePaths *filePaths) {
	vector<Document> documentVector;
	string line;
	ifstream preTFFile (filePaths->preTFFile);
	ifstream preTermFile (filePaths->preTermFile);
	ifstream preDocFile (filePaths->preDocFile);

	if(preDocFile.is_open()) {
		while( getline(preDocFile, line)) {
			vector<string> result;
			char * c_str = strdup(line.c_str());
			char* tokenizer = " -\n\t,.:_()'`\"/{}[]";

			for(char * ptr = strtok(c_str, tokenizer); ptr != NULL; ptr = strtok(NULL, tokenizer)) {
				string temp = string(ptr);
				result.push_back(temp);
			}

			free(c_str);
			
			Document document = Document(stoi(result[0]), result[1], stof(result[3]));
			documentVector.push_back(document);
		}
		cout << "doc complete " << endl;
		preDocFile.close();
	}

	long tf_cnt = 0;
	if(preTermFile.is_open() && preTFFile.is_open()) {
		while( getline(preTermFile, line)) {
			vector<string> result;
			char * c_str = strdup(line.c_str());
			char* tokenizer = "\t";

			for(char * ptr = strtok(c_str, tokenizer); ptr != NULL; ptr = strtok(NULL, tokenizer)) {
				string temp = string(ptr);
				result.push_back(temp);
			}

			free(c_str);

			Term *temp = new Term;
			temp->id = stoi(result[0]);
			temp->str = result[1];
			temp->df = stoi(result[2]);
			temp->cf = stoi(result[3]);
			for(int i = 0; i < temp->df; i++) {
				getline(preTFFile, line);
				result.clear();
				c_str = strdup(line.c_str());

				for(char * ptr = strtok(c_str, tokenizer); ptr != NULL; ptr = strtok(NULL, tokenizer)) {
					string temp = string(ptr);
					result.push_back(temp);
					tf_cnt++;
				}
				// tf term이 다 같아야함
				temp->tf[stoi(result[2])] = stoi(result[3]);
				documentVector[stoi(result[2]) - 1].words.insert(temp);

				free(c_str);
				
			}
			Document::wordList.push_back(temp);
			//cout << Document::wordList[Document::wordList.size() - 1]->str << endl;
		}
		preTermFile.close();
		cout << "TF CNT : " << tf_cnt << endl;
	}

	for(int i = 1; i < Document::wordList.size(); i++) {
		//cout << i << endl;
		Term *yaho = Document::wordList[i];
		//cout << yaho->str << endl;
		map<int, int>::iterator iter = yaho->tf.begin();
		while(iter != yaho->tf.end()) {
			iter++;
		}
	}
	ofstream docFile (filePaths->docyFile);
	int size = Document::wordList.size();
	map<int, float> weights;
	for(int i = 0; i < Document::wordList.size(); i++) {
		if(i % 2000 == 0)
			startTimer();
		Term temp = *Document::wordList[i];
		
		int documentFrequency = temp.df;
		float dValue =	log(Document::getDocumentNumber() / documentFrequency);
		map<int, int>::iterator tfIterator = temp.tf.begin();

		while(tfIterator != temp.tf.end()) {
			int termFrequency = tfIterator->second;
			float numerator = (log((float)termFrequency) + 1.0f) * dValue;	//get doc.data and denominator
			float weight = numerator / documentVector[tfIterator->first - 1].denominator; // is denominator
			if(weight == 0) {
				cout << "Zero : " << temp.id << endl;
			}
			weights[tfIterator->first] += pow(weight, 2);
			// get figure of weight
			int temp_i = (int)weight;
			int count = 0;
			do {
				temp_i = int(temp_i / 10);
				count++;
			} while(temp_i > 0);

			tfIterator++;
		}

		if(i % 2000 == 0) {
			cout << temp.id << " / " << size << "   " << (float)((float)i / (float)size) * 100 << "% proceeding" << endl;
			if(endTimerAndGetMinute() > 0) {
				cout << "Word / Minute speed : " << (int)(i / endTimerAndGetMinute()) << endl;
			}
		}
	}
	
	map<int, float>::iterator weightIter = weights.begin();
	while(weightIter != weights.end()) {
			docFile << documentVector[weightIter->first].toString() << "\t" << sqrt(weightIter->second) << endl;
			weightIter++;
	}
	docFile.close();
}
