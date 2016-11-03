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
long total_cf = 0;
int total_words = 0;

Directories::Directories(string inputDirectory, string outputDirectory) {
	indexFile = inputDirectory + "/index.dat";
	docFile = inputDirectory + "/doc.dat";
	termFile = inputDirectory + "/term.dat";
	queryFile = inputDirectory + "/topics25.txt";
	resultVSMFile = outputDirectory + "/result_vsm.txt";
	resultLMFile = outputDirectory + "/result_bm.txt";
	resultFile = outputDirectory + "/result.txt";
	stopwordsFile = inputDirectory + "/stopwords.txt";
}

Term::Term(string* tokens) {
	id = stoi(tokens[0]);
	word = tokens[1];
	df = stoi(tokens[2]);
	cf = stoi(tokens[3]);
	indexStart = stoull(tokens[4]);
}

Document::Document(string* tokens) {
	id = stoi(tokens[0]);
	docNo = tokens[1];
	size = stoi(tokens[2]);
	//sqrt_of_weight_square_sum = stoi(tokens[4]);
}

Index::Index(string indexFileLine) {
	docId = stoi(indexFileLine.substr(6, 6));
	tf = stoi(indexFileLine.substr(12, 4));
	weight = stof(indexFileLine.substr(16, 7));
}

bool Result::operator<(const Result &other) const {
	return score > other.score;
}

int main(int argc, char *argv[]) {
	if(validateArguments(argc, argv)) {
		Directories *directories = new Directories(string(argv[1]), string(argv[2]));
		stopwords = stopwordFileToList(directories->stopwordsFile);
		list<Query> queryList = queryFileToQueries(directories->queryFile);
		map<string, Term*> terms = termFileToMemory(directories->termFile);
		vector<Document*> documents = documentFileToMemory(directories->docFile);

		list<Query>::iterator queryIter = queryList.begin();
		while(queryIter != queryList.end()) {
			map<Document*, map<string, Index*>> relevantDocuments = findRelevantDocuments(directories->indexFile, *queryIter, terms, documents);
			//list<Result> resultList = rankByVectorSpace(*queryIter, relevantDocuments);
			cout << "find ENd " << endl;
			list<Result> resultList = rankByLanguageModel(*queryIter, relevantDocuments, terms);

			//printResult(resultList);
			resultToFile(*queryIter, resultList, directories->resultFile);
			cout << "query end :  " << (*queryIter).title << endl;
			break;
			queryIter++;

		}

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
	query.title = title;
	query.titleStems = stringToRefinedStems(title);

	string desc = stringUntilNextTag(file, "desc", topTagStartPosition);
	query.descriptionStems = stringToRefinedStems(desc);

	string narr = stringUntilNextTag(file, "narr", topTagStartPosition);
	query.narrativeStems = stringToRefinedStems(narr);

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

map<string, int> stringToRefinedStems(string str) {
	removePunctuation(str);
	list<string> wordList = tokenize(str);
	removeNumberWords(wordList);
	list<string> stemList;
	return stem(wordList);
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

map<string, int> stem(list<string> words) {
	map<string, int> stemMap;
	list<string>::iterator iter = words.begin();
	while( iter != words.end()) {
		string word = *iter;
		Porter2Stemmer::trim(word);
		Porter2Stemmer::stem(word);

		if(!word.empty()) {
			stemMap[word]++;
		}

		iter++;
	}
	return stemMap;
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

				//TODO
				if(index->weight == 0) // there is bug in index so It's temporary fix
					continue;


				/*
				cout << "term id : " << term->id << endl;;
			  cout << "tf : " << index->tf << endl;
			  cout << "docId : " << index->docId << endl;
			  cout << "weight : " << index->weight << endl;
				*/
				//TODO id들 0부터 시작하게
				Document *document = documents[index->docId - 1];
				relevantDocuments[document][term->word] = index;
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
		bool check = false;
		float numerator = 0, q_denominator = 0,  d_denominator = 0;
		
		Document *document = iter->first;
		map<string, Index*> indexList = iter->second;
		Result result;

		result.docNo = document->docNo;
		map<string, Index*>::iterator indexIter = indexList.begin();	
		while(indexIter != indexList.end()) {
			Index *index = indexIter->second;
			numerator += index->weight * query.allStems[index->term->word];
			q_denominator += pow(query.allStems[index->term->word], 2);
			d_denominator += pow(index->weight, 2);
			indexIter++;
		}
		result.score = numerator / sqrt(q_denominator * d_denominator);
		if(check) {
			cout << numerator << endl;
			cout << q_denominator << endl;
			cout << d_denominator << endl;
			cout << q_denominator * d_denominator << endl;
			cout << sqrt(q_denominator * d_denominator) << endl;
			cout << result.score << endl;
			getchar();
		}
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
		result.docNo = document->docNo;
		float score = 0;
		map<string, int>::iterator queryIter = query.allStems.begin();
		while(queryIter != query.allStems.end()) {
			Term *term = terms[queryIter->first];
			int tf = 0;
			if(iter->second[term->word] != NULL) {
				tf = iter->second[term->word]->tf;
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
		cout << iter->docNo << "\t" << iter->score << endl;
		iter++;
	}
}

void resultToFile(Query query, list<Result> resultList, string resultFile) {
	ofstream file (resultFile);

	file << "topicnum : " << query.num << endl;
	file << "query : " << query.title << endl;
	list<Result>::iterator iter = resultList.begin();
	while(iter != resultList.end()) {
		file << iter->docNo << "\t" << iter->score << endl;
		iter++;
	}
	file.close();
}
