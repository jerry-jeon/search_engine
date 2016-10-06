cp output/doc.dat backup/doc.dat
cp output/term.dat backup/doc.dat
cp output/tf.dat backup/tf.dat
cp output/index.dat backup/index.dat

rm output/doc.dat
rm output/term.dat
rm output/tf.dat
rm output/index.dat

g++ -std=c++11 *.cpp && ./a.out input/ output/ stopwords.txt
open output/doc.dat
open output/term.dat
open output/tf.dat
open output/index.dat
