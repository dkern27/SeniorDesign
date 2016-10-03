#!/usr/bin/env python

import os
import sys
import argparse
from subprocess import call

parser = argparse.ArgumentParser(description='Fit all of the data in a given directory')

convertToAscii = 0
parser.add_argument('-d', action='store', dest='datFiles', help='Directory which contains the .dat files. If selected, this option will load all the files in DATFILES to ASCIIFILES')
parser.add_argument('-a', action='store', dest='asciiFiles', help='The folder which contains the output asciiFiles')

results = parser.parse_args()


def loadToASCII():
    call(['mkdir -p results.asciiFiles'], shell=True)
    for dirName, subdirList, fileList in os.walk(results.datFiles):
        print('Found directory: %s' % dirName)
        for fname in fileList:
            print 'From: %s To: %s'%(dirName+'/'+fname, results.asciiFiles+'/'+fname+'.txt')
            call(['./anamu -i %s -o %s'%(dirName+'/'+fname, results.asciiFiles+'/'+fname+'.txt')], shell=True)
            
# ./anamu -o $1 -i $2
# rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS
# ./muonHistogram $1


def makeHistograms():
    for dirName, subdirList, fileList in os.walk(results.asciiFiles):
        print('Found directory: %s' % dirName)
        for fname in fileList:
            print fname
            # call(['rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS'], shell=True)
            # call(['./muonHistogram %s'%(dirName+'/'+fname)], shell=True)
            # call(['rt muonHistogramPlot.C'], shell=True)

            # call(['/bin/bash', '-i', '-c', 'rootbuild -o muonHistogram muonHistogram.cc $ROOTLIBS'])
            call(['./muonHistogram', dirName+'/'+fname])
            # call(['/bin/bash', '-i', '-c', './muonHistogram', dirName+'/'+fname])
            # call(['/bin/bash', '-i', '-c', 'rt', '-q', 'muonHistogramPlot.C'])
            call(['rt', '-q', 'muonHistogramPlot.C'])

if __name__ == "__main__": 
    if (results.datFiles != None):
        loadToASCII()

    makeHistograms()


# Walk through folder to make plots