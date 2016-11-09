#include "include/util.h"
#include "include/porter2_stemmer.h"
#include "include/text_processor.h"
#include "include/document.h"
#include "include/search.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace util;

long total_cf = 0;
int total_words = 0;

bool Result::operator<(const Result &other) const {
	return score > other.score;
}

void checkIndex(string index) {
	ifstream indexFile (index);
	string line;
	long linecnt = 0;
	while(getline(indexFile, line)) {

		if(line.length() != 25) {
			cout << "START" << endl;
			cout << linecnt << endl;
			cout << line.length() << endl;
			cout << line << endl;
			cout << "END" << endl;
		}
		linecnt++;
	}
	cout << "GOOD" << endl;
}

map<string, Term*> termFileToMemory(string termFile) {
	map<string, Term*> terms;
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
			total_cf += stoi(tokens[3]);
			terms[tokens[1]] = new Term(tokens);
		}
		file.close();
	}
	return terms;
}

vector<Document*> documentFileToMemory(string documentFile) {
	vector<Document*> documents;
	string line;
	ifstream file (documentFile);
	if(file.is_open()) {
		while(getline(file, line)) {
			istringstream iss(line);
			string tokens [4];
			string token;
			for(int i = 0; getline(iss, token, '\t'); i++) {
				tokens[i] = token;
			}
			documents.push_back(new Document(tokens));
			total_words++;
		}
		file.close();
	}
	return documents;
}

list<Query> parseToQueries(TextProcessor textProcessor, string queryFile) {
	string fileString = getFileIntoString(queryFile);
	list<Query> queryList;
	int topTagStartPosition = findTopTagPosition(fileString, 0);
	while(topTagStartPosition != -1) {
		queryList.push_back(parseToQuery(textProcessor, fileString, topTagStartPosition));
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

Query parseToQuery(TextProcessor textProcessor, string file, int topTagStartPosition) {
	Query query;

	string num_str = stringUntilNextTag(file, "num", topTagStartPosition);
	query.num = stoi(num_str.substr(num_str.find(":") + 1));

	string title = stringUntilNextTag(file, "title", topTagStartPosition);
	query.title = title;
	query.titleStems = textProcessor.stringToRefinedStems(title);

	string desc = stringUntilNextTag(file, "desc", topTagStartPosition);
	query.descriptionStems = textProcessor.stringToRefinedStems(desc);

	string narr = stringUntilNextTag(file, "narr", topTagStartPosition);
	query.narrativeStems = textProcessor.stringToRefinedStems(narr);

	map<string, int>::iterator iter = query.titleStems.begin();
	while(iter != query.titleStems.end()) {
		query.allStems[iter->first] += iter->second;
		iter++;
	}
	iter = query.descriptionStems.begin();
	while(iter != query.descriptionStems.end()) {
		query.allStems[iter->first] += iter->second;
		iter++;
	}
	iter = query.narrativeStems.begin();
	while(iter != query.narrativeStems.end()) {
		query.allStems[iter->first] += iter->second;
		iter++;
	}
	
	return query;
}

string stringUntilNextTag(string fileString, string tag, int topTagStartPosition) {
	string startTag = "<" + tag + ">";
	int startPosition = fileString.find(startTag, topTagStartPosition) + startTag.size();
	int length = fileString.find("<", startPosition) - startPosition;
	return fileString.substr(startPosition, length);
}

map<Document*, map<string, Index*>> findRelevantDocuments(string indexFileName, Query query, map<string, Term*> terms, vector<Document*> documents) {
	map<Document*, map<string, Index*>> relevantDocuments;

	ifstream indexFile (indexFileName);
	if(indexFile.is_open()) {
		map<string, int>::iterator iterator = query.allStems.begin();
		while( iterator != query.allStems.end()) {
			string line;
			Term* term = terms[iterator->first];
			indexFile.seekg(term->indexStart);

			for(int i = 0; i < term->df; i++) {
				getline(indexFile, line);
				Index *index = new Index(line);
				index->term = term;
				//
				//TODO id들 0부터 시작하게
				Document *document = documents[index->docId - 1];
				relevantDocuments[document][term->str] = index;
			}
			iterator++;
		}
		indexFile.close();
	}

	return relevantDocuments;
}

bool cmp (const Result &left, const Result &right) {
	return left.score > right.score;
}

list<Result> rankByVectorSpace(Query query, map<Document*, map<string, Index*>> relevantDocuments) {
	list<Result> resultList;
	map<Document*, map<string, Index*>>::iterator iter = relevantDocuments.begin();
	while(iter != relevantDocuments.end()) {
		float numerator = 0, d_denominator = 0;
		
		Document *document = iter->first;
		map<string, Index*> indexList = iter->second;
		Result result;

		result.docno = document->docno;
		map<string, Index*>::iterator indexIter = indexList.begin();	
		while(indexIter != indexList.end()) {
			Index *index = indexIter->second;
			numerator += index->weight * query.allStems[index->term->str];
			indexIter++;
		}
		d_denominator = document->weightSum;
		result.score = numerator / sqrt(d_denominator);
		resultList.push_back(result);
		iter++;
	}
	resultList.sort();

	return resultList;
}


list<Result> rankByLanguageModel(Query query, map<Document*, map<string, Index*>> relevantDocuments, map<string, Term*> terms) {	
	float mu = 1500.0f;
	list<Result> resultList;
	map<Document*, map<string, Index*>>::iterator iter = relevantDocuments.begin();
	while(iter != relevantDocuments.end()) {
		Result result;
		Document *document = iter->first;
		result.docno = document->docno;
		float score = 0;
		map<string, int>::iterator queryIter = query.allStems.begin();
		while(queryIter != query.allStems.end()) {
			Term *term = terms[queryIter->first];
			int tf = 0;
			if(iter->second[term->str] != NULL) {
				tf = iter->second[term->str]->tf;
			}
			float cf = term->cf;
			
			/*
			cout << "tf : " << tf << endl;
			cout << mu << endl;
			cout << cf << endl;
			cout << total_cf << endl;
			cout << "total_words : " << total_words << endl;
			*/
			
			score += log(((float)tf + mu * cf / (float)total_cf) / ((float)document->size + mu));
			queryIter++;

		}
		result.score = score;
		resultList.push_back(result);
		iter++;
	}
	resultList.sort();
	return resultList;
}



void printResult(list<Result> resultList) {
	list<Result>::iterator iter = resultList.begin();
	while(iter != resultList.end()) {
		cout << iter->docno << "\t" << iter->score << endl;
		iter++;
	}
}

void resultToFile(Query query, list<Result> resultList, string resultFile) {
	ofstream file (resultFile, ios::app);

	file << "topicnum : " << query.num << endl;
	file << "query : " << query.title << endl;
	list<Result>::iterator iter = resultList.begin();
	while(iter != resultList.end()) {
		file << iter->docno << "\t" << iter->score << endl;
		iter++;
	}
	file.close();
}
