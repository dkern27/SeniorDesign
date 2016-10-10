# SeniorDesign
Sometimes we code stuff for science

1. binary -> ascii
gcc -o anamu anamu.c gpsutil.c
./anamu -o OUTPUTFILE.txt -i INPUTFILE.dat

2. Make Histogram
rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS
./muonHistogram OUTPUTFILE.txt (from step 2)

3. Plot Histogram
rt muonHistogramPlot.C

Run it for directory of data
./doAllData outputDirectory inputFiles
ex) ./doAllData directory inputDirectory/*
