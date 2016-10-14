#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <dirent.h>

// root include files
#include "Riostream.h"
#include <TROOT.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TMath.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>

// Laurent's header files, unique to AN tank data
#include "timestamp.h"
#include "events.h"

// Author: Jeff Johnsen <jjohnsen@mines.edu>
// 10/9/2016
// Makes ROOT histograms of integrated muon trace counts taken from muon binary files. 
// Binary file parsing per code provided by Laurent Guglielmi <laurent.guglielmi@apc.univ-paris7.fr>

using namespace std;

void Usage(string myName);
bool fileExists(const string &name);
bool isDir(const string &name);
void recursiveFileAndDirectoryCheck(vector<string> &inFileNames, const string &inputFileName);
void muonFileDateTimeFromFileName(const string &muonFileName, unsigned int &muonFileDate, unsigned int &muonFileYear, unsigned int &muonFileMonth, unsigned int &muonFileDay, double &muonFileTime);
unsigned int readMuonBuffer( unsigned int * data, int size, const bool &verbose, vector<unsigned int> &muonA30 );
void importMuons(const string &muonFileName, const bool &verbose, vector<unsigned int> &muonA30);
void computeMuonHist(TH1I &muonHist, const vector<unsigned int> muonA30);
void sortMuonFileNames(vector<string> &muonFileNames, const bool &verbose);
double muonFileNameDecimalDateTime(const string &muonFileName);


int main(int argc, char* argv[]) {
  
  // Command line parsing 
  if(argc < 2) Usage(argv[0]);
  vector<string> inFileNames;
  string outFileName = "muonHistograms.root";
  bool verbose = false;
  bool firstFileCall = true;
  for (unsigned int argNum = 1; argNum < argc; argNum++) {
    const string inputArg = argv[argNum];
    if (inputArg == "-o") {
      if (argNum < argc - 1) { // an argument follows the '-o'
        argNum++;
        const string testFileName = argv[argNum];
        // verify the supplied output filename is a valid ROOT TFile filename
        if (testFileName.size() > 5) {
          if (testFileName.substr(testFileName.size() - 5, 5) == ".root") {
            outFileName = testFileName;
            cout << "Output file name specified: " << outFileName << endl;
          } else {
            cout << "Output file name must end in '.root', using " << outFileName << endl;
          }
        }
      } 
    }
    else if (inputArg == "-v") {
      verbose = true;
    } else { // recursively find muon files
      if (firstFileCall) {
        cout << "Accessing muon files..." << endl;
        firstFileCall = false;
      }
      recursiveFileAndDirectoryCheck(inFileNames, inputArg);
    }
  }

  // sort the muon file names and remove any repeated files. 
  if (inFileNames.size() > 1) {
    sortMuonFileNames(inFileNames, verbose);
  }

  unsigned int muonHistDate = 0, muonHistYear = 0, muonHistMonth = 0, muonHistDay = 0;
  double muonHistTime = 0.;  
  TH1I muonHistogram("muonHistogram", "muonHistogram", 2500, 0, 2500);

  // open a TFile and create a TTree and appropriate branches to populate
  TFile outFile(outFileName.c_str(), "recreate");
  TTree muonTree("muonTree", "Muon Histogram Root Tree");
  muonTree.Branch("muonHistDate", &muonHistDate, "muonHistDate/i");
  muonTree.Branch("muonHistYear", &muonHistYear, "muonHistYear/i");
  muonTree.Branch("muonHistMonth", &muonHistMonth, "muonHistMonth/i");
  muonTree.Branch("muonHistDay", &muonHistDay, "muonHistDay/i");
  muonTree.Branch("muonHistTime", &muonHistTime, "muonHistTime/D");
  muonTree.Branch("muonHist", &muonHistogram);


  // dump the input file names to terminal
  for (unsigned int fileNum = 0; fileNum < inFileNames.size(); fileNum++) {

    cout << "Processing: " << inFileNames[fileNum] << endl;

    muonFileDateTimeFromFileName(inFileNames[fileNum], muonHistDate, muonHistYear, muonHistMonth, muonHistDay, muonHistTime);
     
    // Read in the binary muon file using Laurent's procedure, keeping only the indices and a30 values to create the muon histograms. 
    // Unlike Laurent's code, this simplified version does not allow subselection of muons from within a file. 
    const unsigned int estNumMuonFileEntries = 64*1005*63;
    vector<unsigned int> muonA30;
    muonA30.reserve(estNumMuonFileEntries);

    importMuons(inFileNames[fileNum], verbose, muonA30);

    // Integrate the muon traces and compute the muon histogram
    computeMuonHist(muonHistogram, muonA30);
    const string histTitle = "Histogram of A30 integrated muon ADC, from " + inFileNames[fileNum].substr(inFileNames[fileNum].size() - 19, 19 ) + 
      ";integrated A30 counts;number of muon traces";
    muonHistogram.SetTitle(histTitle.c_str());

    // populate the current variable/object values as specified in the tree branch definitions to the TTree branch structure 
    // as a new instance, similar to vector.push_back(var) but without any arguments, because the specification has already 
    // been made in the branch definitions
    muonTree.Fill();

    muonA30.clear();

  }

  // write the TTree to the ROOT TFile and close TFile
  muonTree.Write();
  outFile.Close();
  cout << "Processed " << inFileNames.size() << " muon data files. " << endl;
  cout << "ROOT TFile " << outFileName << " written to disk. " << endl;


  return 0;
}



void Usage(string myName) {
  cout << endl;
  cout << " Synopsis : " << endl;
  cout << myName << " <muon binary file(s) or directory> "  << endl
    << " Options: " << endl
    << "     -o <output ROOT TFile>  |  specifies output ROOT TFile" << endl 
    << "     -v                      |  increases verbosity" << endl << endl;
  
  cout << " Description :" << endl;  
  cout << myName << " extracts muon pulse integrated counts from <muon binary file(s)> " << endl
    << "and populates a ROOT histogram for each file. If a directory is specified a recursive " << endl
    << "directory search is performed to locate the <muon binary file(s)>. The histograms are " << endl 
    << "stored in a ROOT TTree and saved to a <ROOT TFile>. " << endl << endl;

  exit(0);
}


// checks for the existance of a given filename
bool fileExists(const string &name) {
  struct stat fileInfo;   
  return (stat (name.c_str(), &fileInfo) == 0); 
}


// checks if the given name is a directory
bool isDir(const string &name) {
  struct stat fileInfo;
  stat(name.c_str(), &fileInfo);
  return (S_ISDIR(fileInfo.st_mode));
}


// Function recursively searches directories, storing muon filnames in vector for analysis as muon files.
//   Muon files are stored in the format YYYYMMDD_hhmmss.dat use "_" and ".dat" locations to identify possible muon data files. 
//   The actual filename string may be longer as it can contain directory information prior to the actual file name.
void recursiveFileAndDirectoryCheck(vector<string> &inFileNames, const string &inputFileName) {

  // check if the current filename is actually a directory
  if (isDir(inputFileName)) {
    // recursively call this function for all file names and directories found within this valid directory
    DIR *folder;
    struct dirent *folderEntry;
    if ((folder = opendir(inputFileName.c_str())) == NULL) { // verify the folder can be opened
      cout << "ERROR: Couldn't open " << inputFileName << "." << endl;
    } else {
      cout << "Searching " << inputFileName << " for muon data files. " << endl;
      while ((folderEntry = readdir(folder)) != NULL) {
        if (folderEntry->d_name != string(".") && folderEntry->d_name != string("..")) {
          string nextFileName;
          if (inputFileName.at(inputFileName.size() - 1) == '/') {
            nextFileName = inputFileName + folderEntry->d_name;
          } else {
            nextFileName = inputFileName + "/" + folderEntry->d_name;
          }
          recursiveFileAndDirectoryCheck(inFileNames, nextFileName);
        }
      }
      closedir(folder);
    }
  }

  // check if the current filename is "YYYYMMDD_hhmmss.dat" format for a muon file
  else if (inputFileName.size() > 16) { // avoids out of range errors for short file/directory names
    if (inputFileName.substr(inputFileName.size() - 4, 4) == ".dat" && inputFileName.at(inputFileName.size() - 11) == '_') {
      // check that the file exists in the filesystem, and if so add to the list of muon files
      if (fileExists(inputFileName)) {
        inFileNames.push_back(inputFileName);
      } 
    } 
  }

  return;
}


// parse a muon filename of format <YYYYMMDD_hhmmss.dat> to extract integer date <YYYYMMDD>, year <YYYY>, month <MM>, day <DD>, and time in decimal hour
void muonFileDateTimeFromFileName(const string &muonFileName, unsigned int &muonFileDate, unsigned int &muonFileYear, unsigned int &muonFileMonth, unsigned int &muonFileDay, double &muonFileTime) {
  if (muonFileName.size() > 16) {
    const string dateString = muonFileName.substr(muonFileName.size() - 19, 8);
    const string timeString = muonFileName.substr(muonFileName.size() - 10, 6);
    const string year = dateString.substr(0,4);
    const string month = dateString.substr(4,2);
    const string day = dateString.substr(6,2);
    const string hour = timeString.substr(0,2);
    const string minute = timeString.substr(2,2);
    const string second = timeString.substr(4,2);
    muonFileDate = atoi(dateString.c_str());
    muonFileYear = atoi(year.c_str());
    muonFileMonth = atoi(month.c_str());
    muonFileDay = atoi(day.c_str());

    muonFileTime = (double)(atoi(hour.c_str())) + (double)(atoi(minute.c_str())) / 60. + (double)(atoi(second.c_str())) / 3600.;
  }
  return;
}


// reads and stores muon traces from a single muon buffer
unsigned int readMuonBuffer( unsigned int * data, int size, const bool &verbose, vector<unsigned int> &muonA30) {
  int nmuons = 0, idx = 0;
  unsigned int the_date;
  unsigned int a30;
  
  for( int i = 0 ; i < (size/sizeof( unsigned int)) ; i++, data++ ) {
    if ( (*data & 0x80000000 ) != 0 ) {
      /// Start of a burst ///
      nmuons++;
      the_date = *data & 0x3FFFFFFF;
      idx = 0; // index not actually in the binary file, generated in this loop
    }
    // Simplified muon binary file reading which saves only the index and A30 values
    else {
      a30 = *data & 0x3FF; // a30 memory location in binary file
      muonA30.push_back(a30);
      idx++;
    }

  }
  if ( verbose ) printf( "  Number of Muons in buffer: %d\n", nmuons );

  return nmuons;
}


// Read in the binary muon file using Laurent's procedure, keeping only the indices and a30 values to create the muon histograms. 
// Unlike Laurent's code, this simplified version does not allow subselection of muons from within a file. 
void importMuons(const string &muonFileName, const bool &verbose, vector<unsigned int> &muonA30) {
  
  MUON_EVENT pmuon ;
  unsigned int bufferCount = 0 ;

  FILE * InFile = NULL ;
  InFile = fopen( muonFileName.c_str(), "r" );

  unsigned int totalMuons = 0 ;
  double TimeStamp = 0.;
  bool superVerbose = false;

  // Read one-time and bufsize, then read the rest 
  ONE_TIME date ;
  int bufsize ;

  while ( fread( &date, 1, sizeof( ONE_TIME ), InFile ) ==
    sizeof( ONE_TIME ) ) {
    /// Read bufsize ///
    fread( &bufsize, 1, sizeof( bufsize ), InFile ) ;
    pmuon.date = date ;
    pmuon.bufsize = bufsize ;
    /// Read the rest ///
    fread( pmuon.data, 1, bufsize, InFile ) ;

    TimeStamp = pmuon.date.second + ((double)pmuon.date.nano/100000000.) ;
    if ( verbose ) {
      printf( "*** Muon Buffer %d\n ", bufferCount ) ;
      if (superVerbose) {
        printf( " Timestamp (GPS): %.9lf, ", TimeStamp ) ;
        printf( " Buffer Size: %d\n", pmuon.bufsize ) ;
      }
    }
    /// Read in muon traces from a single buffer ///
    totalMuons += readMuonBuffer( pmuon.data, bufsize, verbose, muonA30) ;

    bufferCount++ ;
  }

  printf( "Finished with %d muon buffers read and % d muons\n", bufferCount, totalMuons ) ;

  fclose( InFile );

}


// performs integration of muon traces, and fills muon histogram
void computeMuonHist(TH1I &muonHist, const vector<unsigned int> muonA30) {

  muonHist.Reset();
  const int muonSize = 63;
  const int Nmuons = floor((float)muonA30.size()/(float)muonSize);

  // loop over the muons
  for (int nMu = 0; nMu < Nmuons; nMu++) {

    // loop through each muon to compute the muon integral: muon pulse taken to lie within 
    //   time bins (5,31) and the pedestal is represented by time bins (35,61).
    int muonIntegral = 0;
    for (unsigned int i = nMu*muonSize + 5; i < nMu*muonSize + 31; i++) {
      muonIntegral = muonIntegral + muonA30[i] - muonA30[i+30];
    }

    //add the integrated value to the histogram
    muonHist.Fill(muonIntegral);
  }

  return;
}


// sorts the muon file names, and removes redundant names. 
// uses insert sort. probably isn't a particularly efficient implementation
void sortMuonFileNames(vector<string> &muonFileNames, const bool &verbose) {
  cout << "Sorting muon file names for chronological processing..." << endl;

  // look for repeated files (possibly copies in different directories), keep only the first. 
  const double thresh = 1e-6;
  for (int fileN = 0; fileN < muonFileNames.size() - 1; fileN++) {
    const double fileVal = muonFileNameDecimalDateTime(muonFileNames[fileN]);
    for (int fileM = muonFileNames.size() - 1; fileM > fileN; fileM--) {
      const double compVal = muonFileNameDecimalDateTime(muonFileNames[fileM]);
      if (compVal < fileVal + thresh && compVal > fileVal - thresh) { 
        //file name with equvalent date & time found, remove the offending file from the list
        if (true) cout << "dulicate file found at " << muonFileNames[fileM] << ", removing from process list" << endl;
        muonFileNames.erase(muonFileNames.begin() + fileM);
      }
    }
  }

  // sort the files, algorithm mostly taken from insert sort wikipedia page
  for (unsigned int fileN = 1; fileN < muonFileNames.size(); fileN++) {
    int fileM = fileN - 1;
    const string thisFile = muonFileNames[fileN];
    const double fileVal = muonFileNameDecimalDateTime(thisFile);
    //cout << setprecision(15) << fileVal << ": " << thisFile << endl;
    while (fileM > -1 && muonFileNameDecimalDateTime(muonFileNames[fileM]) > fileVal ) {
      muonFileNames[fileM + 1] = muonFileNames[fileM];
      fileM--;
    }
    muonFileNames[fileM + 1] = thisFile;
  }



  // 
  return;
}


// returns YYYYMMDD.hhmmss, as taken from the name of a muon data file.
double muonFileNameDecimalDateTime(const string &muonFileName) {
  const string dateString = muonFileName.substr(muonFileName.size() - 19, 8);
  const string timeString = muonFileName.substr(muonFileName.size() - 10, 6);
  return (double)atoi(dateString.c_str()) + (double)atoi(timeString.c_str()) / 1000000.;  
}
