#ifndef _INDEXMAKER_
#define _INDEXMAKER_

#include "util.h"
#include <string>

using namespace util;
using namespace std;

void transformFilesInFolder(FilePaths *filePaths, string type, vector<Document> &documentVector);
void transformFile(string filePath, vector<Document> &documentVector);
int findDOCTagPosition(string fileString, int startPosition);

void parseToDocuments(string fileString, vector<Document> &documentVector);
Document parseToDocument(string file, int docTagStartPosition);
string extractContentInTag(string fileString, string tag, int docTagStartPosition);
void writeIndexFile(FilePaths *filePaths, vector<Document> documentVector);
void writeDocDataFile(FilePaths *filePaths, vector<Document> &documentVector);

#endif
