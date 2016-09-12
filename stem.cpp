#include <stack>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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

string whitespaces (" \t\f\v\n\r");
list<string> stopwords;
void initializeStopwords();

void transformFile(string type, int year, int month, int day);
void transformDocument(Document &document);
int findDOCTagPosition(string fileString, int startPosition);

string getFileIntoString(string fileName);
list<Document> extractDocumentList(string file);
Document extractDocument(string file, int docPosition);
string extractContentByTag(string fileString, string tag, int docPosition);
list<string> convertStringToWordList(string str);
void trim(string &str);
void rightTrim(string &str);
void leftTrim(string &str);

void removeStopwordInDocument(Document &document);
void removeStopword(list<string> &words);

void stemDocument(Document &document);
void stemWordList(list<string> &words);

void writeDocumentToFile(string fileName, list<Document> document);
string documentToString(Document document);
string concatStringList(list<string> words);

string makeFileName(string type, int year, int month, int day);
void transformFilesInFolder(string type);

void removePunctuation( string &str );

// TODO consider pass by parameter (not global variable)
string outputDirectory;
string inputDirectory = "./input/";

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

// TODO use argc, argv to get file names, etc..
int main(int argc, char *argv[]) {
  startTimer();

  // TODO argument 처리하는 function 작성
  initializeStopwords();
  outputDirectory = argv[1];

  transformFilesInFolder("APW");
  transformFilesInFolder("NYT");

  endTimerAndPrint("구동시간-------------------------------------");
}

void initializeStopwords() {
  //TODO stopword file argv로 받도록
  string line;
  ifstream file ("stopwords.txt");
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
    list<Document> documentList = extractDocumentList(fileString);
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

list<Document> extractDocumentList(string file) {
  list<Document> documentList;
  for(int docPosition = findDOCTagPosition(file, 0); docPosition != string::npos; docPosition = getDocPosition(file, docPosition)) {
    Document document = extractDocument(file, docPosition);
    transformDocument(document);
    documentList.push_back(document);
  }
  return documentList;
}

// return if doc doens't exist
int findDOCTagPosition(string fileString, int startPosition) {
  string startTag = "<DOC>";
  int result = fileString.find(startTag, startPosition);
  return result != string::npos ? result + startTag.length() : string::npos;
}

//TODO rename
Document extractDocument(string file, int docPosition) {
  string headline = extractContentByTag(file, "HEADLINE", docPosition);
  removePunctuation(headline);
  list<string> headlineWords = convertStringToWordList(headline);
  string text = extractContentByTag(file, "TEXT", docPosition);
  removePunctuation(text);
  list<string> textWords = convertStringToWordList(text);
  Document document = { extractContentByTag(file, "DOCNO", docPosition), headlineWords, textWords };
  return document;
}

void removePunctuation( string &str ) {
  char* charsToRemove = "?()'`\",.;";
  for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
    str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
  }
}

list<string> convertStringToWordList(string str) {
  list<string> result;
  istringstream iss(str);
  do {
    string temp;
    iss >> temp;
    result.push_back(temp);
  } while(iss);
  return result;
}

string extractContentByTag(string fileString, string tag, int docPosition) {
  string startTag = "<" + tag + ">";
  string endTag = "</" + tag + ">";
  string result;
  int startPosition = fileString.find(startTag, docPosition) + startTag.size();
  int length = fileString.find(endTag, docPosition) - startPosition;
  result = fileString.substr(startPosition, length);
  trim(result);
  //TODO should move somewhere
  return result;
}

void trim(string &str) {
  rightTrim(str);
  leftTrim(str);
}

// TODO clear 처리
void rightTrim(string &str) {
  size_t found = str.find_last_not_of(whitespaces);
  if (found!=string::npos) {
    str.erase(found+1);
  }else {
    str.clear();
  }
}

void leftTrim(string &str) {
  str.erase(0, str.find_first_not_of(whitespaces));
}

void transformDocument(Document &document) {
  removeStopwordInDocument(document);
  stemDocument(document);
}

void removeStopwordInDocument(Document &document) {
  removeStopword(document.textWords);
  removeStopword(document.headlineWords);
}

void removeStopword(list<string> &words) {
  list<string>::iterator iter = stopwords.begin();
  while( iter != stopwords.end()) {
    words.remove(*iter);
    iter++;
  }
}

void stemDocument(Document &document) {
  stemWordList(document.headlineWords);
  stemWordList(document.textWords);
}

void stemWordList(list<string> &words) {
  list<string>::iterator iter = words.begin();
  while( iter != words.end()) {
    Porter2Stemmer::trim(*iter);
    Porter2Stemmer::stem(*iter);
    iter++;
  }
  logList(words);
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

//TODO should rename becuase It's not words. (It's stems)
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
