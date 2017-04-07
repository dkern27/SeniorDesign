# SeniorDesign
Sometimes we code stuff for science
Process single data file
1. binary -> ascii
gcc -o anamu anamu.c gpsutil.c
./anamu -o OUTPUTFILE.txt -i INPUTFILE.dat

2. Make Histogram
rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS
./muonHistogram OUTPUTFILE.txt (from step 2)

3. Plot Histogram
rt muonHistogramPlot.C

------------------------------------------------------

Run it for directory of data
rootbuild -o muonHistVEM muonHistVEM.cc $ROOTLIBS
rootbuild -o muonHistFromBinary muonHistFromBinary.cc $ROOTLIBS
./muonHistFromBinary inputFileDirectory
./muonHistVEM test.root

View multiple graphs
rt muonHistBatchPlot.C


data = {Energy : {angle : [array(scint, wc)] }}