#include "search.h"
#include "../util.h"
#include "../porter2_stemmer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

using namespace std;
using namespace util;

list<string> stopwords;

Directories::Directories(string inputDirectory, string outputDirectory) {
	indexFile = inputDirectory + "/index.dat";
	docFile = inputDirectory + "/doc.dat";
	termFile = inputDirectory + "/term.dat";
	queryFile = inputDirectory + "/topics25.txt";
	resultVSMFile = outputDirectory + "/result_vsm.txt";
	resultBMFile = outputDirectory + "/result_bm.txt";
	stopwordsFile = inputDirectory + "/stopwords.txt";
}

Term::Term(string* tokens) {
	id = stoi(tokens[0]);
	word = tokens[1];
	df = stoi(tokens[2]);
	cf = stoi(tokens[3]);
	indexStart = stoi(tokens[4]);
}

bool Query::contains(string word) {
	return find(titleStems.begin(), titleStems.end(), word) != titleStems.end()
		|| find(descriptionStems.begin(), descriptionStems.end(), word) != descriptionStems.end()
		|| find(narrativeStems.begin(), narrativeStems.end(), word) != narrativeStems.end();
}

Index::Index(string indexFileLine) {
	cout << indexFileLine << endl;
	cout << indexFileLine.substr(0, 6) << endl;	
	cout << indexFileLine.substr(6, 6) << endl;	
	cout << indexFileLine.substr(12, 3) << endl;	
	cout << indexFileLine.substr(15, 7) << endl;	
	termId = stoi(indexFileLine.substr(0, 6));
	docId = stoi(indexFileLine.substr(6, 12));
	tf = stoi(indexFileLine.substr(12, 15));
	weight = stof(indexFileLine.substr(15, 23));
}

int main(int argc, char *argv[]) {
	if(validateArguments(argc, argv)) {
		Directories *directories = new Directories(string(argv[1]), string(argv[2]));
		stopwords = stopwordFileToList(directories->stopwordsFile);
		list<Query> queryList = queryFileToQueries(directories->queryFile);
		list<Term*> termList = termFileToMemory(directories->termFile);
		findRelavantDocuments(directories->indexFile, *(queryList.begin()), termList);

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

list<Term*> termFileToMemory(string termFile) {
	list<Term*> terms;
	string line;
	ifstream file (termFile);
	if(file.is_open()) {
		while(getline(file, line)) {
			istringstream iss(line);
			string tokens [5];
			string token;
			for(int i = 0; getline(iss, token, '\t'); i++) {
				tokens[i] = token;
			}
			terms.push_back(new Term(tokens));
		}
		file.close();
	}
	return terms;
}

list<Query> queryFileToQueries(string queryFile) {
	return parseToQueries(getFileIntoString(queryFile));
}

list<Query> parseToQueries(string fileString) {
	list<Query> queryList;
	int topTagStartPosition = findTopTagPosition(fileString, 0);
	while(topTagStartPosition != -1) {
		queryList.push_back(parseToQuery(fileString, topTagStartPosition));
		topTagStartPosition = findTopTagPosition(fileString, topTagStartPosition);
	}
	return queryList;
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

list<int> findRelavantDocuments(string indexFileName, Query query, list<Term*> termList) {
	list<Term*>::iterator iterator = termList.begin();
	list<Index*> indexes;

	cout << indexFileName << endl;
	ifstream indexFile (indexFileName);
	if(indexFile.is_open()) {
		while( iterator != termList.end()) {
			if(query.contains((*iterator)->word)) {
				cout << (*iterator)->word << endl;
				cout << (*iterator)->indexStart << endl;
				int termId = (*iterator)->id;
				string line;
				indexFile.clear();
				indexFile.seekg((*iterator)->indexStart);
				do {
					getline(indexFile, line);
					cout << "line : " << line << endl;
					Index *index = new Index(line);
					termId = index->termId;
					indexes.push_back(index);
					cout << index->weight << endl;
				} while(termId == (*iterator)->id);
			}
			iterator++;
		}
		indexFile.close();
	}
}
