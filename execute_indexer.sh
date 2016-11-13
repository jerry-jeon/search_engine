OPTION=$1
STEP_OPTION=$2

rm ./a.out > /dev/null 2>&1

if [ "$OPTION" == "-d" ]
then
	if g++ -std=c++11 -O3 *.cpp && ./a.out -d; then
		echo "Start execution"
	fi
elif [ "$OPTION" == "-c" ]
then
	g++ -std=c++11 *.cpp;
else
	if g++ -std=c++11 -O3 *.cpp && ./a.out; then
		echo "Start execution"
	fi
fi
