#include "search.h"
#include "../util.h"
#include "../porter2_stemmer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace util;

list<string> stopwords;
int total_cf = 0;
int total_words = 0;

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

Document::Document(string* tokens) {
	id = stoi(tokens[0]);
	docNo = tokens[1];
	size = stoi(tokens[2]);
	//sqrt_of_weight_square_sum = stoi(tokens[4]);
}

bool Query::contains(string word) {
	return find(titleStems.begin(), titleStems.end(), word) != titleStems.end()
		|| find(descriptionStems.begin(), descriptionStems.end(), word) != descriptionStems.end()
		|| find(narrativeStems.begin(), narrativeStems.end(), word) != narrativeStems.end();
}

int Query::tf(string word) {
	return count(allStems.begin(), allStems.end(), word);
}

Index::Index(string indexFileLine) {
	termId = stoi(indexFileLine.substr(0, 6));
	docId = stoi(indexFileLine.substr(6, 6));
	tf = stoi(indexFileLine.substr(12, 3));
	weight = stof(indexFileLine.substr(15, 7));
}

bool Result::operator<(const Result &other) const {
	return score < other.score;
}

int main(int argc, char *argv[]) {
	if(validateArguments(argc, argv)) {
		Directories *directories = new Directories(string(argv[1]), string(argv[2]));
		stopwords = stopwordFileToList(directories->stopwordsFile);
		list<Query> queryList = queryFileToQueries(directories->queryFile);
		list<Term*> termList = termFileToMemory(directories->termFile);
		vector<Document*> documents = documentFileToMemory(directories->docFile);
		
		list<Document> relevantDocuments = findRelevantDocuments(directories->indexFile, *(queryList.begin()), termList, documents);
		list<Result> resultList = rankByVectorSpace(*(queryList.begin()), relevantDocuments);
		list<Result> resultList2 = rankByLanguageModel(*(queryList.begin()), relevantDocuments);

		cout << "Vector space " << endl;
		printResult(resultList);
		cout << "Language model" << endl;
		printResult(resultList2);

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
			total_cf += stoi(tokens[3]);
			terms.push_back(new Term(tokens));
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

	query.allStems.insert(query.allStems.end(), query.titleStems.begin(), query.titleStems.end());
	query.allStems.insert(query.allStems.end(), query.descriptionStems.begin(), query.descriptionStems.end());
	query.allStems.insert(query.allStems.end(), query.narrativeStems.begin(), query.narrativeStems.end());
	
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

list<Document> findRelevantDocuments(string indexFileName, Query query, list<Term*> termList, vector<Document*> documents) {
	list<Term*>::iterator iterator = termList.begin();
	list<Document> relevantDocuments;

	ifstream indexFile (indexFileName);
	if(indexFile.is_open()) {
		while( iterator != termList.end()) {
			if(query.contains((*iterator)->word)) {
				int termId = (*iterator)->id;
				string line;
				indexFile.clear();
				int yaho = (((*iterator)->indexStart) / 23) * 24;
				indexFile.seekg(yaho);
				int df = (*iterator)->df;
				cout << termId << " , " << (*iterator)->word  << " : " << df << endl;
				for(int i = 0; i < (*iterator)->df; i++) {
					getline(indexFile, line);
					Index *index = new Index(line);
					termId = index->termId;
					index->str = (*iterator)->word;
					index->cf = (*iterator)->cf;
					list<Document>::iterator iter;
					cout << index->termId << " : " << index->tf << endl;
					cout << index->docId << " : " << index->weight << endl;
					iter = find(relevantDocuments.begin(), relevantDocuments.end(), *documents[index->docId - 1]);
					if(iter != relevantDocuments.end()) {
						iter->indexes.push_back(index);
						iter->words.insert(*iterator);
					} else {
						Document relevant = *documents[index->docId - 1];
						relevant.indexes.push_back(index);
						relevant.words.insert(*iterator);
						relevantDocuments.push_back(relevant);
					}

					if((*iterator)->word != index->str) {
						cout << "wrong" << endl;
					}
				}
			}
			iterator++;
		}
		indexFile.close();
	}

	return relevantDocuments;
}

list<Result> rankByVectorSpace(Query query, list<Document> relevantDocuments) {
	list<Result> resultList;
	list<Document>::iterator iter = relevantDocuments.begin();
	while(iter != relevantDocuments.end()) {
		Result result;
		result.docNo = iter->docNo; 
		float score = 0;
		list<Index*>::iterator indexIter = (iter->indexes).begin();	
		while(indexIter != (iter->indexes).end()) {
			score += (*indexIter)->weight * query.tf((*indexIter)->str);
			indexIter++;
		}
		result.score = score;
		resultList.push_back(result);
		iter++;
	}
	resultList.sort();
	return resultList;
}

list<Result> rankByLanguageModel(Query query, list<Document> relevantDocuments) {
	float mu = 1500.0f;
	list<Result> resultList;
	list<Document>::iterator iter = relevantDocuments.begin();
	while(iter != relevantDocuments.end()) {
		Result result;
		result.docNo = iter->docNo; 
		float score = 0;
		list<Index*>::iterator indexIter = (iter->indexes).begin();	
		while(indexIter != (iter->indexes).end()) {
			int tf = query.tf((*indexIter)->str);
			float cf = (*indexIter)->cf;
			cout << "tf : " << tf << endl;
			cout << mu << endl;
			cout << cf << endl;
			cout << total_cf << endl;
			cout << "total_words : " << total_words << endl;
			score += log(((float)tf + mu * cf / (float)total_cf) / (float)total_words + mu);
			indexIter++;
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
		cout << iter->docNo << "\t" << iter->score << endl;
		iter++;
	}
}
