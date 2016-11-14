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
#include <TSpectrum.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TMath.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TPolyMarker.h>
#include <TCanvas.h>

using namespace std;

TF1* findVem(TH1I*);
float findVemError(TF1*);


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

  // grab the root tree from where it was stored in the root file
  TTree *muonTree = (TTree*)f.Get("muonTree");
  
  // find the size of the tree to limit looping beyond the end of the tree
  const int treeSize = muonTree->GetEntries();

  // In order to get object information out of the branch, a valid pointer must pre-exist
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

  TGraphErrors *errPlot = new TGraphErrors();

  vector<int> didNotPlot;

  // loop through the histogram cases
  for (int treeStep = 0; treeStep < treeSize; treeStep++) {
    muonTree->GetEntry(treeStep);
    ///////////////////////////Find VEM /////////////////////////////////////////

    TF1 *fit = findVem(muonHist);
    if(fit == NULL){
      didNotPlot.push_back(treeStep);
      continue;
    }
    muonHistVem = fit->GetMaximumX();
    float error = findVemError(fit);
    muonHistVemError = error;
    //cout << "muonHistVem" << endl;
    errPlot->SetPoint(treeStep, treeStep, muonHistVem);
    errPlot->SetPointError(treeStep, 0 , error);
    //////////////////////////////////////////////////////////////////////////////

    // fill the branches with the computed VEM and error values.    
    vemBranch->Fill();
    vemErrorBranch->Fill();
  }

  // overwrite the muon tree to include the new data
  muonTree->Write("", TObject::kOverwrite);
  TCanvas *canvas = new TCanvas();
  errPlot->SetTitle("Errors");
  errPlot->SetMarkerStyle(20);
  errPlot->SetMarkerColor(kBlue);
  errPlot->Draw("AP");
  errPlot->Fit("pol0");

  for (int i = 0; i < didNotPlot.size(); i++)
    cout << didNotPlot[i] << " ";
  cout << endl;
  //f.Close();
  theApp.Run();
  cout << "TFile written to " << fileName << endl;


  
  // if you want the program to exit to a root interpreter, leaving plots open, etc. don't 
  // close the root file, and insteda use 'theApp.Run()' at the end of the program.

}

TF1* findVem(TH1I* muonHistogram) 
{
  //Search for peaks
  TSpectrum *spec = new TSpectrum(2);
  spec->Search(muonHistogram, 3, "nobackground", 0.5);
  TList* functions = muonHistogram -> GetListOfFunctions();
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
  //int npeaks = spec->GetNPeaks();
  Double_t* pmXArray = pm->GetX();

  //Fit around the second peak
  TF1 *f1 = new TF1("f1", "pol2", pmXArray[1]-65,pmXArray[1]+65);
  muonHistogram->Fit("f1","Rq");

  float maxFitX=f1->GetMaximumX();
  float maxFitY=f1->GetMaximum();

  bool keepFitting = true;
  int count=1;
  while(keepFitting)
  {

    f1 = new TF1("f1", "pol2", maxFitX-65, maxFitX+65);
    muonHistogram->Fit("f1","Rq");
    if(abs(maxFitX - f1->GetMaximumX()) <= 0.001){
      keepFitting = false;
      cout << "Fit stabilized after " << count << " iteration(s)." << endl;
    }
    else if(count > 20) {
      keepFitting = false;
      cout << "Fit never stabilized, used 20 iterations for fit" << endl;
      return NULL;
    }
    count++;
    maxFitX=f1->GetMaximumX();
  }

  return f1;
}

float findVemError(TF1* fit) {
  //Output parameters
  float c = fit->GetParameter(0);
  float cerr = fit->GetParError(0);
  float b = fit->GetParameter(1);
  float berr = fit->GetParError(1);
  float a = fit->GetParameter(2);
  float aerr = fit->GetParError(2);

  float maxX = (b*-1)/(2*a);
  float dxda = b/(2*a*a);
  float dxdb = -1/(2 * a);
  float varianceX = pow(dxda*aerr, 2) + pow(dxdb * berr, 2);
  float stdDevX = sqrt(varianceX);
  //cout << stdDevX << endl;
  return stdDevX;
}