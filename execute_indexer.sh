cp output/doc.dat backup/doc.dat > /dev/null 2>&1
cp output/term.dat backup/doc.dat > /dev/null 2>&1
cp output/tf.dat backup/tf.dat > /dev/null 2>&1
cp output/index.dat backup/index.dat > /dev/null 2>&1

rm output/doc.dat > /dev/null 2>&1
rm output/term.dat > /dev/null 2>&1
rm output/tf.dat > /dev/null 2>&1
rm output/index.dat > /dev/null 2>&1

g++ -std=c++11 *.cpp && ./a.out resources/input/ output/ resources/stopwords.txt
rm ./a.out

open output/doc.dat > /dev/null 2>&1
open output/term.dat > /dev/null 2>&1
open output/tf.dat > /dev/null 2>&1
open output/index.dat > /dev/null 2>&1

