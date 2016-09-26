#include <stack>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <map>

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time
#include "stem_statistics.h"

using namespace std;

StemStatistics::StemStatistics(string _stem) {
	stem = _stem;
}

void StemStatistics::addCount(string word) {
	if (find(words.begin(), words.end(), word) == words.end())
		words.push_back(word);
	count++;
}

string StemStatistics::to_string() {
	return stem + " : " + concatStringList(words) + " " + std::to_string(count);
}

bool StemStatistics::operator==(StemStatistics stemStatistics) {
	return stem == stemStatistics.stem;
}

bool compareStemStatistics(StemStatistics s1, StemStatistics s2) {
	return s1.count > s2.count;
}
list<StemStatistics> stemStatisticsList;
string concatStringList(list<string> words) {
	string result;
	list<string>::iterator iter = words.begin();
	result = *iter++;
	while( iter != words.end()) {
		result += " " + *iter;
		iter++;
	}
	return result;
}

void addStemStatistics(string stem, string word) {
	list<StemStatistics>::iterator iter = find(stemStatisticsList.begin(), stemStatisticsList.end(), stem);
	if(iter != stemStatisticsList.end()) {
		iter->addCount(word);
	} else {
		StemStatistics stemStatistics (stem);
		stemStatistics.addCount(word);
		stemStatisticsList.push_back(stemStatistics);
	}
}

void writeStemStatistics() {
	// TODO 전역변수 삭제
	/*
	stemStatisticsList.sort(compareStemStatistics);
	ofstream outputFile ("stem_statistics");
	list<StemStatistics>::iterator iter = stemStatisticsList.begin();
	while( iter != stemStatisticsList.end()) {
		outputFile << iter->to_string() << endl;
		iter++;
	}
	outputFile.close();
	*/
}
