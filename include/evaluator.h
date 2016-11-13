#ifndef _EVALUATOR_
#define _EVALUATOR_

#include <string>
#include <list>
#include <map>

using namespace std;

map<int, map<string, bool>> getRelevantDocuments(string relevantFilePath);
map<int, list<string>> getEvaluatingDocuments(string resultFilePath);

map<int, float> averageRecallPrecision(map<int, map<string, bool>> relevantDocuments, map<int, list<string>> evaluatingDocuments);
map<int, float> recallPrecision(list<string> evaluatingDocuments, map<string, bool> relevantDocuments);

#endif
