#ifndef _SEARCH_
#define _SEARCH_

#include "document.h"
#include "text_processor.h"
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

using namespace std;

struct Query {
	int num;
	string title;
	map<string, int> titleStems;
	map<string, int> descriptionStems;
	map<string, int> narrativeStems;
	map<string, int> allStems;
};

struct Result {
	string docno;
	float score;
	bool operator<(const Result &other) const;
};

int main(int argc, char *argv[]);
bool validateArguments(int argc, char* argv[]);
map<string, Term*> termFileToMemory(string termFile);
vector<Document*> documentFileToMemory(string documentFile);

list<Query> parseToQueries(TextProcessor textProcessor, string queryFile);
int findTopTagPosition(string fileString, int startPosition);
Query parseToQuery(TextProcessor textProcessor, string file, int topTagStartPosition);
string stringUntilNextTag(string fileString, string tag, int topTagStartPosition);

//TODO 꼭 이방법밖에 없을까...
map<Document*, map<string, Index*>> findRelevantDocuments(string indexFileName, Query query, map<string, Term*> terms, vector<Document*> documents);

list<Result> rankByVectorSpace(Query query, map<Document*, map<string, Index*>> relevantDocuments);	
list<Result> rankByLanguageModel(Query query, map<Document*, map<string, Index*>> relevantDocuments, map<string, Term*> terms);	

void printResult(list<Result> resultList);
void resultToFile(Query query, list<Result> resultList, string resultFile); 

#endif
