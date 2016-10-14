#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <string>


// root include files
#include "Riostream.h"
#include <TROOT.h>
#include "TApplication.h"

#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TMath.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>

using namespace std;

void Usage(string myName) {
  cout << endl;
  cout << " Synopsis : " << endl;
  cout << myName << " <muon histogram ROOT TFile>" << endl << endl;
  
  cout << " Description :" << endl;  
  cout << myName << " takes a ROOT file with a TTree containing muon histograms and computes the " << endl
      << "VEM for each histogram, adding it to a new branch in the ROOT tree. " << endl << endl;

  exit(0);
}



int main(int argc, char* argv[]) {
  
  // Command line parsing
  if(argc != 2) Usage(argv[0]);
  const string fileName = argv[1];

  if (fileName.size() < 5 || fileName.substr(fileName.size() - 5, 5) != ".root") {
    cout << "Invalid file name" << endl;
    exit(0);  
  }

  // start a root application. Needed for a compiled program to run code similar to that
  // used by a root script (i.e. all the examples you find)
  TApplication theApp("Muon histogram tree read and update", 0, 0);
  
  // Open TFile with muon histogram TTree 
  TFile f(fileName.c_str(), "update");
  if(!f.IsOpen()) {
    cout << fileName << " failed to open. " << endl;
    exit(0);
  }

  // grab the root tree from where it was stored intthe root file
  TTree *muonTree = (TTree*)f.Get("muonTree");
  
  // find the size of the tree to limit looping beyond the end of the tree
  const int treeSize = muonTree->GetEntries();

  // In order to get object information out of he branch, a valid pointer must pre-exist
  // the Tree GetEntry(i) call for the ith object 
  TH1I *muonHist = NULL;
  unsigned int *muonHistDate = NULL, *muonHistYear = NULL, *muonHistMonth = NULL, *muonHistDay = NULL;
  double* muonHistTime = NULL;  
  
  // Tell the tree where the branches should read out to
  muonTree->SetBranchAddress("muonHist", &muonHist);
  muonTree->SetBranchAddress("muonHistDate", &muonHistDate);    
  muonTree->SetBranchAddress("muonHistYear", &muonHistYear);
  muonTree->SetBranchAddress("muonHistYear", &muonHistYear);
  muonTree->SetBranchAddress("muonHistMonth", &muonHistMonth);
  muonTree->SetBranchAddress("muonHistDay", &muonHistDay);
  muonTree->SetBranchAddress("muonHistTime", &muonHistTime);

  double muonHistVem = 0, muonHistVemError = 0;
  double *vemPtr = &muonHistVem, *vemErrorPtr = &muonHistVemError;

  // branch definitions to hold new branches for VEM and VEMerror
  TBranch *vemBranch = NULL;
  TBranch *vemErrorBranch = NULL;

  // check for the existence of a VEM branch. If this program has been run on a muon histogram tree already
  // then a VEM branch will have already been added.  It's a bit more work up front, but allows any follow-on
  // code to run exactly the same without checking back to which case we started with
  if (muonTree->GetBranch("muonHistVem")) {
    // this program has already been run, use the existing branches
    cout << "Pre-existing VEM branch found, re-using" << endl;
    muonTree->SetBranchAddress("muonHistVem", &vemPtr);
    muonTree->SetBranchAddress("muonHistVemError",  &vemErrorPtr);
    vemBranch = muonTree->GetBranch("muonHistVem");
    vemErrorBranch = muonTree->GetBranch("muonHistVemError");
  } else {
    // make the new branches
    cout << "No VEM branch found, making new" << endl;
    vemBranch = muonTree->Branch("muonHistVem", &muonHistVem, "muonHistVem/D");
    vemErrorBranch = muonTree->Branch("muonHistVemError", &muonHistVemError, "muonHistVemError/D");
  }


  // loop through the histogram cases
  for (int treeStep = 0; treeStep < treeSize; treeStep++) {
    muonTree->GetEntry(treeStep);

    ///////////////////////////Find VEM /////////////////////////////////////////
    
    // my utterly fake 'VEM' which uses some info from the histogram so the 
    // numbers will change from histogram to histogram, only here for example
    // and so the branch filling code has something to work with
    muonHistVem = muonHist->GetMean();
    muonHistVemError = muonHist->GetMeanError();

    // I recommend making your own function which is called here, takes the current histogram
    // as an argument, and gives back the VEM, VEMerror, and anything else you think is important

    // If you are working on multiple VEM computation methods, you can create a function for each
    // and a new branch for each (more code needed above for additional new branches of course)
    // and then do a comparative study later


    //////////////////////////////////////////////////////////////////////////////

    // fill the branches with the computed VEM and error values.    vemBranch->Fill();
    vemErrorBranch->Fill();



  }

  // overwrite the muon tree to include the new data
  muonTree->Write("", TObject::kOverwrite);


  f.Close();

  cout << "TFile written to " << fileName << endl;
  
  // if you want the program to exit to a root interpreter, leaving plots open, etc. don't 
  // close the root file, and insteda use 'theApp.Run()' at the end of the program.

}
