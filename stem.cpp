#include <stack>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include "porter2_stemmer.cpp"

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time

using namespace std;
using namespace std::chrono;

struct Document {
  string docno;
  list<string> headlineWords;
  list<string> textWords;
};


stack<high_resolution_clock::time_point> startTimeStack;
void startTimer();
void endTimerAndPrint(string with);
void logList(list<string> strings);
string durationToString(long duration);
//these above functions and variables are for development.

string inputDirectory, outputDirectory, stopwordsFile;
int handleArguments(int argc, char* argv[]);
string whitespaces (" \t\f\v\n\r");
list<string> stopwords;
void initializeStopwords();

string makeFileName(string type, int year, int month, int day);
string getFileIntoString(string fileName);
void transformFilesInFolder(string type);
void transformFile(string type, int year, int month, int day);
void transformDocument(Document &document);
int findDOCTagPosition(string fileString, int startPosition);

list<string> tokenize(string str);
list<Document> parseToDocuments(string fileString);
Document parseToDocument(string file, int docTagStartPosition);
string extractContentInTag(string fileString, string tag, int docTagStartPosition);
void trim(string &str);

void removePunctuation( string &str );
void removeNumberWords( list<string> &words );
void removeStopwordInDocument(Document &document);
void removeStopword(list<string> &words);
void stemDocument(Document &document);
void stemWordList(list<string> &words);

void writeDocumentToFile(string fileName, list<Document> document);
string documentToString(Document document);
string concatStringList(list<string> words);

void startTimer() {
  startTimeStack.push(high_resolution_clock::now());
}

void endTimerAndPrint(string with) {
  auto duration = duration_cast<milliseconds>( high_resolution_clock::now() - startTimeStack.top() ).count();
  cout << with << endl;
  cout << durationToString(duration) << endl;
  startTimeStack.pop();
}

string durationToString(long duration) {
  string result = "";
  int deciSecond = (duration / 100) % 10;
  int second = (duration / 1000) % 60;
  int minute = (duration / (1000 * 60)) % 60;
  int hour = (duration / (1000 * 60 * 60)) % 24;

  if(hour > 0)
    result += to_string(hour) + "시간 ";
  if(minute > 0)
    result += to_string(minute) + "분 ";
  result += to_string(second) + "." + to_string(deciSecond) + "초 걸렸습니다.";

  return result;
}

void logList(list<string> strings) {
  list<string>::iterator iter = strings.begin();
  while( iter != strings.end()) {
    cout << *iter << endl;
    iter++;
  }
}
//these above functions are for development.

int main(int argc, char *argv[]) {
  startTimer();

  if(handleArguments(argc, argv) == -1) {
    return -1;
  } else {
    initializeStopwords();

    //transformFile("APW", 1998, 6, 1);
    //transformFilesInFolder("APW");
    transformFilesInFolder("NYT");

    endTimerAndPrint("구동시간-------------------------------------");
    return 0;
  }
 
}

// return -1 if argument is not valid
int handleArguments(int argc, char* argv[]) {
  if(argc < 4) {
    cout << "다음의 형태로 사용해주세요" << endl;
    cout << argv[0] << " input폴더 output폴더 stopword파일" << endl;
    cout << "ex) " << argv[0] << " input/ output/ stopwords.txt" << endl;
    return -1;
  } else {
    inputDirectory = argv[1];
    outputDirectory = argv[2];
    stopwordsFile = argv[3];
    return 0;
  }
}

void initializeStopwords() {
  string line;
  ifstream file (stopwordsFile);
  if(file.is_open()) {
    while(getline(file, line)) {
      if(!line.empty())
        stopwords.push_back(line);
    }
    file.close();
  }
}


void transformFilesInFolder(string type) {
  for(int year = 1998; year <= 2000; year++) {
    for(int month = 1; month <= 12; month++) {
      for(int day = 1; day <= 31; day++) {
        transformFile(type, year, month, day);
      }
    }
  }
}

void transformFile(string type, int year, int month, int day) { 
  string fileName = makeFileName(type, year, month, day);
  string fileString = getFileIntoString(inputDirectory + type + "/" + to_string(year) + "/" + fileName);

  if(fileString != "") {
    startTimer();
    list<Document> documentList = parseToDocuments(fileString);
    writeDocumentToFile(fileName, documentList);
    endTimerAndPrint("refine complete " + fileName);
  }
}

string makeFileName(string type, int year, int month, int day) {
  char buffer[100];
  if(type == "APW") {
    std::sprintf(buffer, "%d%02d%02d_APW_ENG", year, month, day);
  } else if(type == "NYT") {
    std::sprintf(buffer, "%d%02d%02d_NYT", year, month, day);
  } else {
    //TODO throw error
  }
  return string(buffer);
}


string getFileIntoString(string fileName) {
  string line, result = "";
  ifstream file (fileName);
  if(file.is_open()) {
    result.assign( (istreambuf_iterator<char>(file) ), (istreambuf_iterator<char>()));
    file.close();
    cout << "read complete " << fileName << endl;
  }
  return result;
}

list<Document> parseToDocuments(string fileString) {
  list<Document> documentList;
  int docTagStartPosition = findDOCTagPosition(fileString, 0);
  while(docTagStartPosition != string::npos) {
    Document document = parseToDocument(fileString, docTagStartPosition);
    transformDocument(document);
    documentList.push_back(document);

    docTagStartPosition = findDOCTagPosition(fileString, docTagStartPosition);
    //return documentList;
  }

  return documentList;
}

// return npos if doc doens't exist
int findDOCTagPosition(string fileString, int startPosition) {
  string startTag = "<DOC>";
  int result = fileString.find(startTag, startPosition);
  return result != string::npos ? result + startTag.length() : string::npos;
}

// TODO 여기서 바로 받으면서 처리해야 하는지. 속도는 빠르지만 코드 가독성이 낮아서 일단은 분리하는 방향으로 진행함.
Document parseToDocument(string file, int docTagStartPosition) {
  string headline = extractContentInTag(file, "HEADLINE", docTagStartPosition);
  removePunctuation(headline);
  string text = extractContentInTag(file, "TEXT", docTagStartPosition);
  removePunctuation(text);
  Document document = { extractContentInTag(file, "DOCNO", docTagStartPosition), tokenize(headline), tokenize(text) };
  return document;
}

void removePunctuation( string &str ) {
  char* charsToRemove = "?()'`\",.;_";
  for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
    str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
  }
}

list<string> tokenize(string str) {
  list<string> result;
  istringstream iss(str);
  do {
    string temp;
    iss >> temp;
    result.push_back(temp);
  } while(iss);
  return result;
}

string extractContentInTag(string fileString, string tag, int docTagStartPosition) {
  string startTag = "<" + tag + ">";
  string endTag = "</" + tag + ">";
  string result;
  int startPosition = fileString.find(startTag, docTagStartPosition) + startTag.size();
  int length = fileString.find(endTag, docTagStartPosition) - startPosition;
  result = fileString.substr(startPosition, length);
  //TODO should move somewhere
  return result;
}

// tokenize 과정에서 처리해주는 것으로 보임. 나중에 삭제
// Deprecated
void trim(string &str) {
  // trim right
  size_t found = str.find_last_not_of(whitespaces); 
  if (found!=string::npos) {
    str.erase(found+1);
  }else {
    str.clear();
  }

  // trim left
  str.erase(0, str.find_first_not_of(whitespaces));
}

void transformDocument(Document &document) {
  removeNumberWords(document.headlineWords);
  removeNumberWords(document.textWords);
  removeStopword(document.headlineWords);
  removeStopword(document.textWords);
  stemWordList(document.headlineWords);
  stemWordList(document.textWords);
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

void removeStopword(list<string> &words) {
  list<string>::iterator iter = stopwords.begin();
  while( iter != stopwords.end()) {
    words.remove(*iter);
    iter++;
  }
}

void stemWordList(list<string> &words) {
  list<string>::iterator iter = words.begin();
  while( iter != words.end()) {
    Porter2Stemmer::trim(*iter);
    Porter2Stemmer::stem(*iter);
    iter++;
  }
}

void writeDocumentToFile(string fileName, list<Document> documentList) {
  ofstream outputFile (outputDirectory + fileName);
  list<Document>::iterator iter = documentList.begin();
  while( iter != documentList.end()) {
    outputFile << documentToString(*iter) << endl;
    iter++;
  }
  outputFile.close();
}

string documentToString(Document document) {
  string result = "";
  result += "[DOCNO] : " + document.docno + "\n";
  result += "[HEADLINE] : " + concatStringList(document.headlineWords) + "\n";
  result += "[TEXT] : " + concatStringList(document.textWords) + "\n";
  return result;
}

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
