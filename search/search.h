#ifndef _SEARCH_
#define _SEARCH_

#include <string>
#include <list>

using namespace std;

struct Directories {
	Directories(string inputDirectory, string outputDirectory);

	string indexFile;
	string docFile;
	string termFile;
	string queryFile;
	string resultVSMFile;
	string resultBMFile;
	string stopwordsFile;
};

struct Query {
	int num;
	list<string> titleStems;
	list<string> descriptionStems;
	list<string> narrativeStems;
	bool contains(string word);
};

struct Term {
	Term(string* tokens);

	int id;
	string word;
	int df;
	int cf;
	int indexStart;
};

struct Index {
	Index(string indexFileLine);

	int termId;
	int docId;
	int tf;
	float weight;
};

int main(int argc, char *argv[]);
bool validateArguments(int argc, char* argv[]);
list<string> stopwordFileToList(string stopwordsFile);
list<Term*> termFileToMemory(string termFile);

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

list<int> findRelavantDocuments(string indexFileName, Query query, list<Term*> termList);

void rankByVectorSpace();	
void rankByBM25();	
void rankByLanguageModel();	


#endif
