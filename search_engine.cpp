#include "document.h"
#include "util.h"
#include "text_processor.h"
#include "index_maker.h"
#include "search_engine.h"

#include <iostream>

//using namespace util;
using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
	int option;
	cout <<	"Welcome!" << endl;
	cout << "----------------------------" << endl << endl;

	cout << "Choose what to do" << endl;
	cout << "1. Make index" << endl;
	cout << "2. Search with exist index" << endl;
	

	cout << "Type number : ";
	cin >> option;

	cout << option << endl;

	switch(option) {
		case 1:
			makeIndex();
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

void makeIndex() {
	// TODO get input from user
	FilePaths *filePaths = new FilePaths("input/", "index/", "result/");
	//TODO should be deleted
	Document::outputDirectory = "index/";

	startTimer();
	//TODO
	Document::textProcessor = TextProcessor (filePaths->stopwordsFile);

	startTimer();
	vector<Document> documentVector;
	transformFile(filePaths->articleFile("NYT", 2000, 8, 27), documentVector);
	//transformFilesInFolder(filePaths, "APW", documentVector);
	//transformFilesInFolder(filePaths, "NYT", documentVector);
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

/*
// return -1 if argument is not valid
bool validateArguments(int argc, char* argv[]) {
	if(argc < 4) {
		cout << "Use following format" << endl;
		cout << argv[0] << " input_folder output_folder stopword_file option(omittable)" << endl;
		cout << "ex) " << argv[0] << " input/ output/ stopwords.txt" << endl;
		cout << "option -s1 : create data files without indexing" << endl;
		cout << "option -s2 : create index file with already existing files" << endl;
		return false;
	} else {
		return true;
	}
}

*/

void search() {

}
