#include "include/evaluator.h"
#include "include/util.h"
#include <fstream>
#include <sstream>
#include <iostream>

map<int, float> averageRecallPrecision(map<int, map<string, bool>> relevantDocuments, map<int, list<string>> evaluatingDocuments) {
	map<int, float> recallPrecisionSum;
	map<int, list<string>>::iterator iter = evaluatingDocuments.begin();
	while(iter != evaluatingDocuments.end()) {
		int queryNum = iter->first;
		map<int, float> rp = recallPrecision(evaluatingDocuments[queryNum], relevantDocuments[queryNum]);
		map<int, float>::iterator rpIter = rp.begin();
		while(rpIter != rp.end()) {
			recallPrecisionSum[rpIter->first] += rpIter->second;
			rpIter++;
		}
		iter++;
	}

	map<int, float>::iterator rpIter = recallPrecisionSum.begin();
	while(rpIter != recallPrecisionSum.end()) {
		recallPrecisionSum[rpIter->first] /= evaluatingDocuments.size();
		rpIter++;
	}
	
	return recallPrecisionSum;
}

map<int, float> recallPrecision(list<string> evaluatingDocuments, map<string, bool> relevantDocuments) {
	float hit = 0;
	float evaluateSize = 0;
	float recall, precision;
	int level = 1;
	map<int, float> rp;
	int relevantSize = relevantDocuments.size();
	list<string>::iterator evalIter = evaluatingDocuments.begin();
	while(evalIter != evaluatingDocuments.end()) {
		//cout << *evalIter << endl;
		if(relevantDocuments[*evalIter]) {
			hit++;
			recall = hit / relevantSize;
			precision = hit / evaluateSize;
			if(recall > ((float)level / 10.0)) {
				rp[level++] = precision;
			}

		}
		evalIter++;
		evaluateSize += 1;
	}
	return rp;
}

map<int, map<string, bool>> getRelevantDocuments(string relevantFilePath) {
	map<int, map<string, bool>> relevants;
	ifstream file (relevantFilePath);
	string line;
	if(file.is_open()) {
		while(getline(file, line)) {
			istringstream iss(line);
			string tokens [2];
			string token;
			for(int i = 0; getline(iss, token, '\t'); i++) {
				tokens[i] = token;
			}
			util::trim(tokens[1]);
			relevants[stoi(tokens[0])][tokens[1]] = true;
			//cout << relevants[stoi(tokens[0])][tokens[1]] << endl;
		}
		file.close();
	}
	return relevants;
}

map<int, list<string>> getEvaluatingDocuments(string resultFilePath) {
	map<int, list<string>> evaluates;
	ifstream file (resultFilePath);
	string line;
	if(file.is_open()) {
		while(getline(file, line)) {
			int queryNum = stoi(line);
			getline(file, line);
			istringstream iss(line);
			list<string> tokens;
			string token;
			for(int i = 0; getline(iss, token, '\t'); i++) {
				util::trim(token);
				tokens.push_back(token);
			}
			evaluates[queryNum] = tokens;
		}
		file.close();
	}
	return evaluates;
}
