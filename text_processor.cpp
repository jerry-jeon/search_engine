#include "include/text_processor.h"
#include "include/porter2_stemmer.h"
#include "include/util.h"
#include <regex>
#include <fstream>
#include <cstdio>
#include <map>
#include <iostream>

using namespace std;

TextProcessor::TextProcessor() {}

TextProcessor::TextProcessor(string stopwordsFile) {
	initializeStopwords(stopwordsFile);
}

void TextProcessor::initializeStopwords(string stopwordsFile) {
	string line;
	ifstream file (stopwordsFile);
	if(file.is_open()) {
		while(getline(file, line)) {
			if(!line.empty())
				stopwords.push_back(line);
		}
		file.close();
	}
}

void TextProcessor::removePunctuation( string &str ) {
	char* charsToRemove = "%#!?;*$\\+@="; // <>
	for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
		str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
	}
}

void TextProcessor::removeNumberWords( list<string> &words ) {
	regex numReg(".*[0-9]+.*");
	regex tagReg("<[^>]*>");
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		if(regex_match(*iter, numReg) || regex_match(*iter, tagReg)) {
			iter = words.erase(iter);
		} else
			iter++;
	}
}

bool TextProcessor::isStopword(string word) {
	list<string>::iterator iter = stopwords.begin();
	while( iter != stopwords.end()) {
		if(word == *iter)
			return true;
		iter++;
	}
	return false;
}

string TextProcessor::concatStringList(list<string> words) {
	string result;
	list<string>::iterator iter = words.begin();
	result = *iter++;
	while( iter != words.end()) {
		result += " " + *iter;
		iter++;
	}
	return result;
}

list<string> TextProcessor::tokenize(string str) {
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

map<string, int> TextProcessor::stem(list<string> words) {
	map<string, int> stemMap;
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		string word = *iter;
		util::trim(word);
		Porter2Stemmer::stem(word);

		if(!word.empty()) {
			stemMap[word]++;
		}

		iter++;
	}
	return stemMap;
}

map<string, int> TextProcessor::stringToRefinedStems(string str) {
	removePunctuation(str);
	list<string> wordList = tokenize(str);
	removeNumberWords(wordList);
	list<string> stemList;
	return stem(wordList);
}


