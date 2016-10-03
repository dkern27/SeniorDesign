#!/usr/bin/env bash

outdir=$1
echo "using outdir $outdir"
echo -n > parameter0.txt
shift
# mkdir -p $outdir
for fullfile in $*;
do
	echo processing $fullfile
	filename=$(basename "$fullfile")
	extension="${filename##*.}"
	filename="${filename%.*}"
	echo $extension
	if test "$extension" = "dat" ; then
		./anamu -i $fullfile -o $outdir/$filename.txt
		fullfile=$outdir/$filename.txt
	# elif [$extension == .txt]; then
	# 	cp $fullfile $outdir/$filename.txt
	# else
	# 	echo "unrecognized input type"
	# 	exit 1
	fi
	./muonHistogram $fullfile
	echo -n "${filename} "  >> parameter0.txt
	root -l -q muonHistogramPlot.C

done