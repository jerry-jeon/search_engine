#include <stack>
#include <algorithm>
#include <cstdio>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include "porter2_stemmer.cpp"
#include <map>

#include <chrono> // this is for check execution time
#include <ctime> // this is for check execution time
#include "stem_statistics.cpp"

void removePunctuation( string &str );
void removeNumberWords( list<string> &words );
bool isStopword(string word);
void stem(list<string> &stemList, list<string> words);

class Document {
  public:
    static list<string> stopwords;
    string docno;
    list<string> headlineWords;
    list<string> textWords;
    map<string, int> frequencies;
    list<string> headlineStems;
    list<string> textStems;
    int maxFrequency = 0;

    Document(string _docno, string headline, string text) {
      docno = _docno;
      removePunctuation(headline);
      removePunctuation(text);
      headlineWords = tokenize(headline);
      textWords = tokenize(text);
    }

    list<string> tokenize(string str);
    float termFrequency(string term);
    float idf(string term, list<Document> documents);
    float tfidf(string term, list<Document> documents);
    bool contain(string term);
    void transform();

    string to_string();
};


list<string> Document::stopwords = {};

list<string> Document::tokenize(string str) {
  list<string> result;
  istringstream iss(str);
  do {
    string temp;
    iss >> temp;
    // make lowercase for remove stopword
    ::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
    if(isStopword(temp))
      continue;
    // TODO 메소드로 분리하는게 좋을듯(max frequency 구하는 부분)
    if(maxFrequency < ++frequencies[temp]) {
      maxFrequency = frequencies[temp];
    }
    result.push_back(temp);
  } while(iss);
  return result;
  
}

void removePunctuation( string &str ) {
  char* charsToRemove = "?()'`\",.;_:";
  for (unsigned int  i = 0; i < strlen(charsToRemove); ++i) {
    str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end());
  }
}

void Document::transform() {
  removeNumberWords(headlineWords);
  removeNumberWords(textWords);
  stem(headlineStems, headlineWords);
  stem(textStems, textWords);

  //headlineWords.unique();
  //textStems.unique();
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

bool isStopword(string word) {
  list<string>::iterator iter = Document::stopwords.begin();
  while( iter != Document::stopwords.end()) {
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
    // TODO 옮기자!!!!!
    if(!word.empty()) {
      stemList.push_back(word);
    }
    //addStemStatistics(*iter, word);
    iter++;
  }
}

// tokenize 과정에서 처리해주는 것으로 보임. 나중에 삭제
// Deprecated
void trim(string &str) {
  string whitespaces (" \t\f\v\n\r");
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


string Document::to_string() {
  string result = "";
  result += "[DOCNO] : " + docno + "\n";
  result += "[HEADLINE] : " + concatStringList(headlineStems) + "\n";
  result += "[TEXT] : " + concatStringList(textStems) + "\n";

  return result;
}

float Document::termFrequency(string term) {
  //augmented frequency, to prevent a bias towards longer documents
  return 0.5f + (0.5f * (float)frequencies[term] / (float)maxFrequency);
}

bool Document::contain(string term) {
  return frequencies.find(term) != frequencies.end();
}

float Document::idf(string term, list<Document> documents) {
  int termAppearedDocumentNumber = 0;
  list<Document>::iterator iter = documents.begin();
  while( iter != documents.end()) {
    if(iter->contain(term))
      termAppearedDocumentNumber++;
    iter++;
  }

  return log10(documents.size() / (termAppearedDocumentNumber + 1));
}

float Document::tfidf(string term, list<Document> documents) {
  return termFrequency(term) * idf(term, documents);
}

void writeHighRankedTfIdfWords(list<Document> documentList) {
  ofstream outputFile ("tfidf");
  list<Document>::iterator iter = documentList.begin();
  while( iter != documentList.end()) {
    map<string, int>::iterator wordIter = iter->frequencies.begin();
    while( wordIter != iter->frequencies.end()) {
      float tfidf = iter->tfidf(wordIter->first, documentList);
      if(tfidf < 0.2)
        outputFile << wordIter->first << " : " <<  tfidf << endl;
      wordIter++;
    }
    iter++;
  }
  outputFile.close();
  
}
