#include "search.h"
#include "../util.h"
#include "../porter2_stemmer.h"

#include <fstream>
#include <iostream>
#include <regex>

using namespace std;
using namespace util;

list<string> stopwords;

Directories::Directories(string inputDirectory, string outputDirectory) {
	indexFile = inputDirectory + "/index.dat";
	docFile = inputDirectory + "/doc.dat";
	wordFile = inputDirectory + "/word.dat";
	queryFile = inputDirectory + "/topics25.txt";
	resultVSMFile = outputDirectory + "/result_vsm.txt";
	resultBMFile = outputDirectory + "/result_bm.txt";
	stopwordsFile = inputDirectory + "/stopwords.txt";
}

int main(int argc, char *argv[]) {
	if(validateArguments(argc, argv)) {
		Directories *directories = new Directories(string(argv[1]), string(argv[2]));
		stopwords = stopwordFileToList(directories->stopwordsFile);
		queryFileToQueries(directories->queryFile);
	} else {
		cout << "Use following format" << endl;
		cout << argv[0] << " input_folder output_folder" << endl;
		cout << "ex) " << argv[0] << " input output" << endl;
	}
}

bool validateArguments(int argc, char* argv[]) {
	if(argc < 2) {
		return false;
	} else {
		return true;
	}
}

list<string> stopwordFileToList(string stopwordsFile) {
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
	return stopwords;
}

void queryFileToQueries(string queryFile) {
	parseToQueries(getFileIntoString(queryFile));
}

void parseToQueries(string fileString) {
	int topTagStartPosition = findTopTagPosition(fileString, 0);
	while(topTagStartPosition != -1) {
		Query query = parseToQuery(fileString, topTagStartPosition);
		
		logList(query.titleStems);
		logList(query.descriptionStems);
		logList(query.narrativeStems);

		topTagStartPosition = findTopTagPosition(fileString, topTagStartPosition);
	}
}

// return -1 if doc doens't exist
int findTopTagPosition(string fileString, int startPosition) {
	string startTag = "<top>";
	int result = fileString.find(startTag, startPosition);
	return result != string::npos ? result + startTag.length() : -1;
}

Query parseToQuery(string file, int topTagStartPosition) {
	Query query;

	string num_str = stringUntilNextTag(file, "num", topTagStartPosition);
	query.num = stoi(num_str.substr(num_str.find(":") + 1));

	string title = stringUntilNextTag(file, "title", topTagStartPosition);
	query.titleStems = stringToRefinedStems(title);

	string desc = stringUntilNextTag(file, "desc", topTagStartPosition);
	query.descriptionStems = stringToRefinedStems(desc);

	string narr = stringUntilNextTag(file, "narr", topTagStartPosition);
	query.narrativeStems = stringToRefinedStems(narr);
	
	return query;
}

string stringUntilNextTag(string fileString, string tag, int topTagStartPosition) {
	string startTag = "<" + tag + ">";
	int startPosition = fileString.find(startTag, topTagStartPosition) + startTag.size();
	int length = fileString.find("<", startPosition) - startPosition;
	return fileString.substr(startPosition, length);
}

list<string> stringToRefinedStems(string str) {
	removePunctuation(str);
	list<string> wordList = tokenize(str);
	removeNumberWords(wordList);
	list<string> stemList;
	stem(stemList, wordList);
	return stemList;
}

void removePunctuation( string &str ) {
	char* charsToRemove = "%#!?;*$\\+@="; // <>
	for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
		str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
	}
}

list<string> tokenize(string str) {
	list<string> result;
	char * c_str = strdup(str.c_str());
	char* tokenizer = " -\n\t,.:_()'`\"/{}[]\r\n";

	for(char * ptr = strtok(c_str, tokenizer); ptr != NULL; ptr = strtok(NULL, tokenizer)) {
		string temp = string(ptr);
		::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		if(isStopword(temp))
			continue;
		result.push_back(temp);
	}

	free(c_str);
	return result;
}

bool isStopword(string word) {
	list<string>::iterator iter = stopwords.begin();
	while( iter != stopwords.end()) {
		if(word == *iter)
			return true;
		iter++;
	}
	return false;
}

void stem(list<string> &stemList, list<string> words) {
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		string word = *iter;
		Porter2Stemmer::trim(word);
		Porter2Stemmer::stem(word);

		if(!word.empty()) {
			stemList.push_back(word);
		}

		iter++;
	}
}

void removeNumberWords( list<string> &words ) {
	regex reg(".*[0-9]+.*");
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		if(regex_match(*iter, reg)) {
			iter = words.erase(iter);
		} else
			iter++;
	}
}

