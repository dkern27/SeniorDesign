#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <vector>


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
#include <TSpectrum.h>
#include <TList.h>
#include <TPolyMarker.h>
#include <TFormula.h>

using namespace std;

void Usage(string myName)
{
  cout << endl;
  cout << " Synopsis : " << endl;
  cout << myName << " <muon ascii file>" << endl << endl;
  
  cout << " Description :" << endl;  
  cout << myName << " extracts muon pulse integrated counts from <muon ascii file> " << endl
    << "generated by 'anamu' and populates a ROOT histogram, stored in a ROOT TFile. " << endl << endl;

  exit(0);
}



int main(int argc, char* argv[])
{
  
  // Command line parsing
  if(argc != 2) Usage(argv[0]);
  const char* inFileName = argv[1];

  //variables
  char title[50];
  
  const char* outFileName = "muonHistogram.root";
  // "update" to append to file, "recreate" to write over the file
  const char* outFileOpt = "recreate";
  const bool makeTracePlots = true;
  const int numTracePlots = 5;
  

/*          GENERAL NOTES 
  1) Loop over muons in text file
  2) read in 'a30' column for each muon (0-62)
  3) compute integrated counts for each muon
  4) store muons in a histogram


*/

  // Declaring Plots

  // 1D HISTOGRAMS

/*
  // Example of a 1D Histogram Declaration
  TH1F *DataSet = new TH1F("DataSet","DataSet",500,0,3000);

*/  
  TH1I *muonHistogram = new TH1I("muonHistogram", "muonHistogram", 500, 0, 2500);
  TH1I *muonTraceHistograms[numTracePlots]; 
  if (makeTracePlots) {
    
    for (int n = 0; n < numTracePlots; n++ ) {
      sprintf(title, "%s%i","muonTrace",n);
      muonTraceHistograms[n] = new TH1I(title, title, 63, 0, 62);
    }
  }

/*
  // Example of a 2D Histogram Declaration
  TH2F *DataSet = new TH2F("DataSet","DataSet",100,0,100,75,-15,0);
*/

  // SCATTERPLOTS  


/*
  // Example of Scatterplot Declaration
  TGraphErrors *ZenithRecBelow = new TGraphErrors();
  ZenithRecBelow->SetName("ZenithRecBelow");
*/

  cout << "Reading in data from " << inFileName << endl;

  ifstream inFile(inFileName);
  
  if(!inFile.is_open()) {
    cout << "Import file failed to open." << endl;
    return(1);
  }


  unsigned long int count = 0;
  //double nullDataRow[5];
  vector<unsigned int> muonIndex;
  unsigned int index, a30;
  vector<unsigned int> muonA30;
  string ignore;
  while(!inFile.eof()) {
    //read in muon index and a30 columns
    inFile >> index;
    inFile >> a30;
    muonIndex.push_back(index);
    muonA30.push_back(a30);
    // read in but do not use other values in a row of the input file.
    inFile >> ignore >> ignore >> ignore >> ignore >> ignore;
    count++;
  }


  inFile.close();

  cout << "Lines imported: " << count << endl;


  // compute the muon integrals and store in a histogram
  const int muonSize = 63;
  const int Nmuons = floor((float)muonA30.size()/(float)muonSize);
  
  // loop over the muons
  for (int nMu = 0; nMu < Nmuons; nMu++) {
    // loop through each muon to compute the muon integral
    unsigned int j;
    int muonIntegral = 0;
    for (unsigned int i = nMu*muonSize + 5; i < nMu*muonSize + 31; i++) {
      j =  + i;
      muonIntegral = muonIntegral + muonA30[i] - muonA30[i+30];
    }

    //add the integrated value to the histogram
    muonHistogram->Fill(muonIntegral);
  }

  // create plots of the muon traces, when desired
  if (makeTracePlots) {
    // loop over the muon traces to plot (the first however many muons)
    for (int n = 0; n < numTracePlots; n++ ) {
      // using a histogram to make this plot, could use a scatterplot or whatever
      // set values in the bins (which start at 1, bin 0 is underflow) to the A30 values for each muon trace
      for (int i = n*muonSize; (i < ((n+1)*muonSize)) && (i < muonA30.size()); i++) {
        muonTraceHistograms[n]->SetBinContent(muonIndex[i] + 1, muonA30[i] + n*120);
      }
    }
  }


  // Writing to a TFile
  TFile *f = new TFile(outFileName, outFileOpt);

  //GenericScatter->Write();
  muonHistogram->Write();
  if (makeTracePlots) {
    for (int n = 0; n < numTracePlots; n++ ) {
      muonTraceHistograms[n]->Write();
    }
  }
  
  f->Close();

  cout << "TFile successfully written to " << outFileName << endl;

}
