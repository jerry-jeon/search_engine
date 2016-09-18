#include <stack>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <map>

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time

using namespace std;
using namespace std::chrono;
string concatStringList(list<string> words);
list<string> stopwords;
class StemStatistics {
  public:
  string stem;
  list<string> words;
  int count = 0;

  StemStatistics(string _stem) {
    stem = _stem;
  }

  void addCount(string word) {
    if(find(words.begin(), words.end(), word) == words.end())
      words.push_back(word);
    count++;
  }

  string to_string() {
    return stem + " : " + concatStringList(words) + " " + std::to_string(count) + "번";
  }

  bool operator==(StemStatistics stemStatistics) {
    return stem == stemStatistics.stem;
  }
};

bool compareStemStatistics(StemStatistics s1, StemStatistics s2) {
  return s1.count > s2.count;
}

list<StemStatistics> stemStatisticsList;
void addStemStatistics(string stem, string word) {
  list<StemStatistics>::iterator iter = find(stemStatisticsList.begin(), stemStatisticsList.end(), stem);
  if(iter != stemStatisticsList.end()) {
    iter->addCount(word);
  } else {
    StemStatistics stemStatistics (stem);
    stemStatistics.addCount(word);
    stemStatisticsList.push_back(stemStatistics);
  }
}

void writeStemStatistics() {
  stemStatisticsList.sort(compareStemStatistics);
  ofstream outputFile ("stem_statistics");
  list<StemStatistics>::iterator iter = stemStatisticsList.begin();
  while( iter != stemStatisticsList.end()) {
    outputFile << iter->to_string() << endl;
    iter++;
  }
  outputFile.close();
}
string whitespaces (" \t\f\v\n\r");
#include "document.cpp"

stack<high_resolution_clock::time_point> startTimeStack;
void startTimer();
void endTimerAndPrint(string with);
void logList(list<string> strings);
string durationToString(long duration);
void makeWordStatistics(list<string> words);
void makeStemStatistics(list<string> stems);
//these above functions and variables are for development.

string inputDirectory, outputDirectory, stopwordsFile;
int handleArguments(int argc, char* argv[]);
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

void writeDocumentToFile(string fileName, list<Document> document);
string documentToString(Document document);


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

    transformFile("APW", 2000, 9, 30);
    //transformFile("APW", 2000, 8, 30);
    //transformFilesInFolder("APW");
    //transformFilesInFolder("NYT");

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
    document.transform();
    documentList.push_back(document);

    docTagStartPosition = findDOCTagPosition(fileString, docTagStartPosition);
  }

  return documentList;
}

// return npos if doc doens't exist
int findDOCTagPosition(string fileString, int startPosition) {
  string startTag = "<DOC>";
  int result = fileString.find(startTag, startPosition);
  return result != string::npos ? result + startTag.length() : string::npos;
}

Document parseToDocument(string file, int docTagStartPosition) {
  string headline = extractContentInTag(file, "HEADLINE", docTagStartPosition);
  string text = extractContentInTag(file, "TEXT", docTagStartPosition);
  return Document (headline, text);
}

string extractContentInTag(string fileString, string tag, int docTagStartPosition) {
  string startTag = "<" + tag + ">";
  string endTag = "</" + tag + ">";
  string result;
  int startPosition = fileString.find(startTag, docTagStartPosition) + startTag.size();
  int length = fileString.find(endTag, docTagStartPosition) - startPosition;
  result = fileString.substr(startPosition, length);
  return result;
}

void writeDocumentToFile(string fileName, list<Document> documentList) {
  ofstream outputFile (outputDirectory + fileName);
  list<Document>::iterator iter = documentList.begin();
  while( iter != documentList.end()) {
    outputFile << iter->to_string() << endl;
    iter++;
  }
  outputFile.close();
  writeStemStatistics();
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
