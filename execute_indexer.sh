OPTION=$1
STEP_OPTION=$2

rm ./a.out > /dev/null 2>&1

if [ "$OPTION" == "-r" ]
then
	g++ -std=c++11 -O3 *.cpp
	if [ "$STEP_OPTION" == "-s1" ]
	then
		if ./a.out resources/input/ output/ resources/stopwords.txt -s1; then
			echo "Start execution"
		fi
	elif [ "$STEP_OPTION" == "-s2" ]
	then
		if ./a.out resources/input/ output/ resources/stopwords.txt -s2; then
			echo "Start execution"
		fi
	else
		if ./a.out resources/input/ output/ resources/stopwords.txt; then
			echo "Start execution"
		fi
	fi
elif [ "$OPTION" == "-d" ]
then
	if g++ -std=c++11 -g *.cpp && gdb a.out; then
		echo "Start execution"
	fi
elif [ "$OPTION" == "-c" ]
then
	g++ -std=c++11 *.cpp;
else
	if g++ -std=c++11 *.cpp && ./a.out resources/input/ output/ resources/stopwords.txt; then
		echo "Start execution"
	fi
fi
