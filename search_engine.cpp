#include "include/document.h"
#include "include/util.h"
#include "include/text_processor.h"
#include "include/index_maker.h"
#include "include/search_engine.h"
#include "include/search.h"

#include <iostream>

//using namespace util;
using namespace std;
using namespace std::chrono;

bool developMode = false;
int main(int argc, char *argv[]) {
	if(argc > 1 && string(argv[1]) == "-d") {
		developMode = true;
	}

	int option;
	string input, index, result;
	cout <<	"Welcome!" << endl;
	cout << "----------------------------" << endl << endl;

	cout << "Type directory names with following these format:" << endl;
	cout << "-------------------------------------------------------------" << endl;
	cout << "input_directory index_dircetory result_dircetory" << endl;
	cout << "-------------------------------------------------------------" << endl;
	if(developMode) {
		input = "input";
		index = "index";
		result = "/Users/jeongyeongju/Dropbox/ir";
	} else {
		cin >> input >> index >> result;
	}
	cout << endl;
	FilePaths *filePaths = new FilePaths(input, index, result);

	cout << "Choose what to do" << endl;
	cout << "1. Make index" << endl;
	cout << "2. Search with exist index" << endl;
	

	cout << "Type number : ";
	if(developMode) {
		option = 2;
	} else {
		cin >> option;
	}


	switch(option) {
		case 1:
			makeIndex(filePaths);
			break;
		case 2:
			search(filePaths);
			break;
		default:
			cout << "There is no option " << option << endl;
			break;

	}
	/*
	startTimer();
	endTimerAndPrint("Reading input file -------------------------------------");
	*/
}

void makeIndex(FilePaths *filePaths) {
	//TODO should be deleted
	Document::outputDirectory = filePaths->indexFile;

	startTimer();
	//TODO
	Document::textProcessor = TextProcessor (filePaths->stopwordsFile);

	startTimer();
	vector<Document> documentVector;
	//transformFile(filePaths->articleFile("NYT", 2000, 8, 27), documentVector);
	transformFilesInFolder(filePaths, "APW", documentVector);
	transformFilesInFolder(filePaths, "NYT", documentVector);
	endTimerAndPrint("Reading input file -------------------------------------");

	string mode = "-s3";

	if(mode == "-s1") {
		writeTFFile(filePaths, documentVector);
	} else if(mode == "-s2") {
		readFiles(filePaths);
		//writeIndex();
	} else {

		cout << "Start cacluate denimonator and write document file..." << endl;
		startTimer();
		writeDocDataFile(filePaths, documentVector);
		endTimerAndPrint("Writing document file -------------------------------------");
		
		startTimer();
		cout << "Start write index and term file..." << endl;
		writeIndexFile(filePaths, documentVector);
		endTimerAndPrint("Writing index file -------------------------------------");
	}
	endTimerAndPrint("All time -------------------------------------");
}

void search(FilePaths *filePaths) {
	int option;
	startTimer();
	cout << "Start reading stopwords file..." << endl;
	TextProcessor textProcessor = TextProcessor (filePaths->stopwordsFile);
	endTimerAndPrint("Finished reading stopwords file");
	cout << endl << "Start reading term file..." << endl;
	startTimer();
	map<string, Term*> terms = termFileToMemory(filePaths->termFile);
	endTimerAndPrint("Finished reading term file");
	cout << endl << "Start reading document file..." << endl;
	startTimer();
	vector<Document*> documents = documentFileToMemory(filePaths->docFile);
	endTimerAndPrint("Finished reading document file");
	startTimer();
	cout << endl <<"Start reading query file..." << endl;
	list<Query> queryList = parseToQueries(textProcessor, filePaths->queryFile);
	endTimerAndPrint("Finished reading query file");
	
	cout << endl << "Choose model" << endl;
	cout << "1. Vector space model" << endl;
	cout << "2. Language model" << endl;

	cout << "Type number : ";
	if(developMode) {
		option = 2;
	} else {
		cin >> option;
	}

	modelFunction model = getModelFromOption(option);

	cout << "Start ranking..." << endl;
	startTimer();
	list<Query>::iterator queryIter = queryList.begin();
	while(queryIter != queryList.end()) {
		cout << endl;
		cout << "Scoring Query - " + queryIter->title << endl;
		cout << "===============================================" << endl;
		startTimer();
		startTimer();
		map<Document*, map<string, Index*>> relevantDocuments = findRelevantDocuments(filePaths->indexFile, *queryIter, terms, documents);
		cout << "Relevant document size : " << relevantDocuments.size() << endl;
		endTimerAndPrint("Find relevant documents");
		list<Result> resultList = model(*queryIter, relevantDocuments, terms);

		resultToFile(*queryIter, resultList, filePaths->resultFile);
		endTimerAndPrint("Rank documents");
		queryIter++;

	}
	cout << endl;
	endTimerAndPrint("All rank time");
}
