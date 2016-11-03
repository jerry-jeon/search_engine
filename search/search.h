#ifndef _SEARCH_
#define _SEARCH_

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

using namespace std;

struct Directories {
	Directories(string inputDirectory, string outputDirectory);

	string indexFile;
	string docFile;
	string termFile;
	string queryFile;
	string resultVSMFile;
	string resultLMFile;
	string resultFile;
	string stopwordsFile;
};

struct Query {
	int num;
	string title;
	list<string> titleStems;
	list<string> descriptionStems;
	list<string> narrativeStems;
	list<string> allStems;
	bool contains(string word);
	int tf(string word);
};

struct Term {
	Term(string* tokens);

	int id;
	string word;
	int df;
	int cf;
	unsigned long long indexStart;
	bool operator==(const Term &other) const {
		return id == other.id;
	}
	bool operator==(int _id) {
		return id == _id;
	}
};

struct Index {
	Index(string indexFileLine);

	int termId;
	int docId;
	int tf;
	int cf;
	string str;
	float weight;
};

struct Document {
	Document(string* tokens);

	int id;
	string docNo;
	int size;
	float sqrt_of_weight_square_sum;
	list<Index*> indexes;

	set<Term*> words; // this is for language model

	bool operator==(const Document &other) {
		return id == other.id;
	}
};

struct Result {
	string docNo;
	float score;
	bool operator<(const Result &other) const;
};

int main(int argc, char *argv[]);
bool validateArguments(int argc, char* argv[]);
list<string> stopwordFileToList(string stopwordsFile);
map<string, Term*> termFileToMemory(string termFile);
vector<Document*> documentFileToMemory(string documentFile);

list<Query> queryFileToQueries(string queryFile);
list<Query> parseToQueries(string fileString);
int findTopTagPosition(string fileString, int startPosition);
Query parseToQuery(string file, int topTagStartPosition);
string stringUntilNextTag(string fileString, string tag, int topTagStartPosition);
list<string> stringToRefinedStems(string str);
void removePunctuation( string &str );
list<string> tokenize(string str);
bool isStopword(string word);
void stem(list<string> &stemList, list<string> words);
void removeNumberWords( list<string> &words );

map<int, list<Index*>> findRelevantDocuments(string indexFileName, Query query, map<string, Term*> terms, vector<Document*> documents);

list<Result> rankByVectorSpace(Query query, map<int, list<Index*>> docMap, vector<Document*> documents);	
list<Result> rankByLanguageModel(Query query, map<int, list<Index*>> docMap, vector<Document*> documents);	

void printResult(list<Result> resultList);
void resultToFile(Query query, list<Result> resultList, string resultFile); 

#endif
