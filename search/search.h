#ifndef _SEARCH_
#define _SEARCH_

#include <string>
#include <list>

using namespace std;

struct Directories {
	string indexFile;
	string docFile;
	string wordFile;
	string queryFile;
	string resultVSMFile;
	string resultBMFile;
	string stopwordsFile;

	Directories(string inputDirectory, string outputDirectory);
};

struct Query {
	int num;
	list<string> titleStems;
	list<string> descriptionStems;
	list<string> narrativeStems;
};

int main(int argc, char *argv[]);
bool validateArguments(int argc, char* argv[]);
void queryFileToQueries();
void findRelavantDocuments();
void rankByVectorSpace();	
void rankByBM25();	
void rankByLanguageModel();	

list<string> stopwordFileToList(string stopwordsFile);
void parseToQueries(string fileString);
void queryFileToQueries(string queryFile);
int findTopTagPosition(string fileString, int startPosition);
Query parseToQuery(string file, int topTagStartPosition);
string stringUntilNextTag(string fileString, string tag, int topTagStartPosition);
list<string> stringToRefinedStems(string str);
void removePunctuation( string &str );
list<string> tokenize(string str);
bool isStopword(string word);
void stem(list<string> &stemList, list<string> words);
void removeNumberWords( list<string> &words );

#endif
