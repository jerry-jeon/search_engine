OPTION=$1
STEP_OPTION=$2

cp output/doc.dat backup/doc.dat > /dev/null 2>&1
cp output/term.dat backup/doc.dat > /dev/null 2>&1
cp output/tf.dat backup/tf.dat > /dev/null 2>&1
cp output/index.dat backup/index.dat > /dev/null 2>&1

if [ "$OPTION" == "-r" ]
then
	g++ -std=c++11 -O3 *.cpp
	if [ "$STEP_OPTION" == "-s1" ]
	then
		if ./a.out resources/input/ output/ resources/stopwords.txt -s1; then
			echo "Start execution"
		fi
	elif [ "$STEP_OPTION" == "-s1" ]
	then
		if ./a.out resources/input/ output/ resources/stopwords.txt -s2; then
			echo "Start execution"
		fi
	else [ "$STEP_OPTION" == "-s1" ]
		if ./a.out resources/input/ output/ resources/stopwords.txt; then
			echo "Start execution"
		fi
	fi
elif [ "$OPTION" == "-d" ]
then
	if g++ -std=c++11 -g *.cpp && gdb a.out; then
		echo "Start execution"
	fi
else
	if g++ -std=c++11 *.cpp && ./a.out resources/input/ output/ resources/stopwords.txt; then
		echo "Start execution"
	fi
fi

rm ./a.out > /dev/null 2>&1

open output/doc.dat > /dev/null 2>&1
open output/term.dat > /dev/null 2>&1
open output/tf.dat > /dev/null 2>&1
open output/index.dat > /dev/null 2>&1
