#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>


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

TGraphErrors* fillTreeWithVem(TTree*& muonTree, TH1I*& muonHist, TBranch*& vemBranch, TBranch*& vemErrorBranch, vector<int>& didNotPlot);
TF1* findVem(TH1I*);
TF1* findVemMultBinsTest(TH1I*);
float findVemError(TF1*);


void Usage(string myName) 
{
	cout << endl;
	cout << " Synopsis : " << endl;
	cout << myName << " <muon histogram ROOT TFile>" << endl << endl;

	cout << " Description :" << endl;  
	cout << myName << " takes a ROOT file with a TTree containing muon histograms and computes the " << endl
	<< "VEM for each histogram, adding it to a new branch in the ROOT tree. " << endl << endl;
	cout << " NOTES : " << endl;
	cout << "Sometimes finds VEM at very high numbers, well above the range of the histogram. Run the progam again to try again." << endl << endl;

	exit(0);
}

int main(int argc, char* argv[]) 
{
	  // Command line parsing
	if(argc != 2) Usage(argv[0]);
	const string fileName = argv[1];

	if (fileName.size() < 5 || fileName.substr(fileName.size() - 5, 5) != ".root") 
	{
		cout << "Invalid file name" << endl;
		return EXIT_SUCCESS;  
	}

 	 // start a root application. Needed for a compiled program to run code similar to that
  	// used by a root script (i.e. all the examples you find)
	TApplication theApp("Muon histogram tree read and update", 0, 0);

  	// Open TFile with muon histogram TTree 
	TFile f(fileName.c_str(), "update");
	if(!f.IsOpen()) 
	{
		cout << fileName << " failed to open. " << endl;
		exit(0);
	}

  	// grab the root tree from where it was stored in the root file
	TTree *muonTree = (TTree*)f.Get("muonTree");

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

	double muonHistVem, muonHistVemError;
	double *vemPtr = &muonHistVem, *vemErrorPtr = &muonHistVemError;

  	// branch definitions to hold new branches for VEM and VEMerror
	TBranch *vemBranch = NULL;
	TBranch *vemErrorBranch = NULL;
	gErrorIgnoreLevel=kError;
  	// check for the existence of a VEM branch. If this program has been run on a muon histogram tree already
  	// then a VEM branch will have already been added.  It's a bit more work up front, but allows any follow-on
  	// code to run exactly the same without checking back to which case we started with
	if (muonTree->GetBranch("muonHistVem")) 
	{
		// this program has already been run, use the existing branches
		cout << "Pre-existing VEM branch found, re-using..." << endl;
		muonTree->SetBranchAddress("muonHistVem", &vemPtr);
		muonTree->SetBranchAddress("muonHistVemError",  &vemErrorPtr);
		vemBranch = muonTree->GetBranch("muonHistVem");
		vemErrorBranch = muonTree->GetBranch("muonHistVemError");
	} 
	else 
	{
		// make the new branches
		cout << "No VEM branch found, making new..." << endl;
		vemBranch = muonTree->Branch("muonHistVem", &muonHistVem, "muonHistVem/D");
		vemErrorBranch = muonTree->Branch("muonHistVemError", &muonHistVemError, "muonHistVemError/D");
	}

	//Find VEM for each histogram
	vector<int> didNotPlot;
  	TGraphErrors *errPlot = fillTreeWithVem(muonTree, muonHist, vemBranch, vemErrorBranch, didNotPlot);

  	// overwrite the muon tree to include the new data
	muonTree->Write("", TObject::kOverwrite);
	TCanvas *canvas = new TCanvas();
	errPlot->SetTitle("Errors");
	errPlot->GetYaxis()->SetTitle("VEM");
	errPlot->SetMarkerStyle(20);
	errPlot->SetMarkerColor(kBlue);
	errPlot->SetMinimum(0);
	errPlot->Draw("AP");
	errPlot->Fit("pol0", "q");

	//Write failed indexes to file to plot using muonHistBatchPlotFailed.C
	if(didNotPlot.size() > 0)
	{
		ofstream file("failedIndexes.txt", ofstream::trunc);
		for (int i = 0; i < didNotPlot.size(); i++)
			file << didNotPlot[i] << endl;
		cout << "Failed plot indexes written to "<< "failedIndexes.txt" << endl;
		file.close();
	}

  	//f.Close();
	theApp.Run();
	cout << "TFile written to " << fileName << endl;

  	// if you want the program to exit to a root interpreter, leaving plots open, etc. don't 
 	 // close the root file, and insteda use 'theApp.Run()' at the end of the program.
	return EXIT_SUCCESS;
}

/*
Loops through histograms in the tree and finds the VEM and error in the VEM. Returns plot of VEM with error bars.
*/
TGraphErrors* fillTreeWithVem(TTree*& muonTree, TH1I*& muonHist, TBranch*& vemBranch, TBranch*& vemErrorBranch, vector<int>& didNotPlot)
{
	cout << "Finding VEM from histograms..." << endl;
	// find the size of the tree to limit looping beyond the end of the tree
	const int treeSize = muonTree->GetEntries();
	TGraphErrors *errPlot = new TGraphErrors();
	int point = 0;

	for (int treeStep = 0; treeStep < treeSize; treeStep++) 
	{
		muonTree->GetEntry(treeStep);

		//Protects against empty entries from crashing the program
		if (muonHist->GetEntries() < 64064/2) //64064 is the number of entries per file
			continue; 
		TF1 *fit = findVem(muonHist);
		if(fit == NULL)
		{
			didNotPlot.push_back(treeStep);
			continue;
		}
		double muonHistVem = fit->GetMaximumX();
		//Since our calculated error is so high, either use no error or bin error to visualize the curve
		//float error = muonHist->GetBinError(muonHistVem);
		float error = findVemError(fit);
		double muonHistVemError = error;
		// cout << muonHistVem << endl;
		// cout << error << endl;
		
		errPlot->SetPoint(point, point, muonHistVem);
		errPlot->SetPointError(point, 0 , error);
		point++;

		// fill the branches with the computed VEM and error values.    
		vemBranch->Fill();
		vemErrorBranch->Fill();
	}
	return errPlot;
}

/*
Finds VEM for a single histogram using peak finding and several fits
*/
TF1* findVem(TH1I* muonHistogram) 
{
  	//Search for peaks
	int binNumber = 5;
	muonHistogram->Rebin(binNumber);
	TSpectrum *spec = new TSpectrum(3);
	spec->Search(muonHistogram, 3, "nodrawnobackground", 0.25);
	float* xArray = spec->GetPositionX();
	float maxX = *max_element(xArray, xArray+3);

	int count=0;

	while(count < 20)
	{
		//65 was the sweet spot for finding VEM
		//Tried using a smarter range about the peak, but they had larger errors and lower success rate in finding VEM
		TF1* f1 = new TF1("f1", "pol2", maxX-65, maxX+65);
		muonHistogram->Fit("f1","NRq");
		if(abs(maxX - f1->GetMaximumX()) <= binNumber) 
		{
			return f1;
		}
		count++;
		maxX=f1->GetMaximumX();
	}
	return NULL;
}

/*
NOT BEING USED
Attempt to improve our fitting by slowly increasing our binning
On our tests it did worse than a set binning in findVem()
*/
TF1* findVemMultBinsTest(TH1I* muonHistogram) 
{
  	//Search for peaks
	TSpectrum *spec = new TSpectrum(2);
	spec->Search(muonHistogram, 3, "nobackground", 0.2);
	float* xArray = spec->GetPositionX();
	float spectrumX = *max_element(xArray, xArray+2);

  	//Fit around the second peak
	TF1 *f1 = new TF1("f1", "pol2", spectrumX-65, spectrumX+65);
	muonHistogram->Fit("f1","Rq");

	double maxFitX=f1->GetMaximumX();
	TH1I *muonHistCopy = (TH1I*)muonHistogram->Clone("muonHistCopy");

	TF1 *bestFit = (TF1*)f1->Clone("bestFit");
	int bestThreshold = -1;
	int threshold = 0;
	int bestBin = 0;

	for( int bin = 3; bin <=8; bin++)
	{
		//rebin the histogram
		muonHistCopy = (TH1I*)muonHistogram->Clone("muonHistCopy");
		muonHistCopy->Rebin(bin);
		TSpectrum *spec = new TSpectrum(2);
		spec->Search(muonHistCopy, 3, "nobackground", 0.5);
		float* xArray = spec->GetPositionX();
		float spectrumX = *max_element(xArray, xArray+2);

		int count=0;
		while(count > 20)
		{
			f1 = new TF1("f1", "pol2", maxFitX-65, maxFitX+65);
			muonHistCopy->Fit("f1","Rq");
			threshold = abs(maxFitX - f1->GetMaximumX());
			if(threshold <= bin)
			{
				return f1;
			}
			count++;
			maxFitX=f1->GetMaximumX();
		}
	}

	cout << threshold << " NO SUCCESS" << endl;

	return NULL;
}

/*
Calculates the error in our VEM based on the equation of a parabola
*/
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

	return stdDevX;
}