./anamu -o $1 -i $2
rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS
./muonHistogram $1
