#include <algorithm>

using namespace std;

muonHistBatchPlotFailed(string fileName, string rootFile, bool showFit=false)
{
  cout << fileName << endl;
  ifstream file(fileName.c_str(), ios_base::in);
  vector<int> failed;

  if (file.is_open()){
    int tmp;
    while (!file.eof()){
      file >> tmp;
      failed.push_back(tmp);
    }
    for (int i = 0; i<failed.size(); i++){
      cout << failed[i] << " ";
    }
    cout << endl;
  } else {
    cout << "could not open file " << fileName << endl;
  }


  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0); 

  // open the root file where the muon histograms are stored
  TFile *f = new TFile("muons2014.root");
  
  //limit the number of histograms to attempt to plot
   const int maxPlot = 25; 

  // grab the root tree from where it was stored intthe root file
  TTree *muonTree = (TTree*)f->Get("muonTree");
  
  // find the size of the tree to limit looping beyond the end of the tree
  const int treeSize = muonTree->GetEntries();

  // In order to get object information out of he branch, a valid pointer must pre-exist
  // the Tree GetEntry(i) call for the ith object
  TH1I *muonHistList[maxPlot];

  // I know my re-binning wil cause warnings some of the time, ignoring them
  gErrorIgnoreLevel = kWarning +2;

  int j = 0;
  for(j=0; j < failed.size() && j < maxPlot; j++) {

    int i = failed[j];
    // It is not good enough for a pointer of the correct object to exist, it must be a valid pointer
    // either to an object or NULL in order for the GetEntry() call to work
    muonHistList[j] = NULL;

    // must tell the branch where to put the object prior to the GetEntry() call
    muonTree->SetBranchAddress("muonHist", &muonHistList[j]);

    // note: There are other values/variables stored in the tree, but i've only laid the groundwork to fetch
    // one of them ( the histogram I want to plot )

    // get the ith histogram from the tree
    muonTree->GetEntry(i);
   
    // make a new canvas each time, or the script will re-use the canvas that was last in scope
    // and overwrite the previous histogram
    TCanvas* c = new TCanvas();

    // muonHistList[j]->Rebin(5);



    // make some style changes, for demonstration
    muonHistList[j]->Draw();
    muonHistList[j]->Rebin(5);
    muonHistList[j]->GetXaxis()->CenterTitle();
    muonHistList[j]->GetYaxis()->CenterTitle();
    muonHistList[j]->GetYaxis()->SetTitleOffset(1.15);
    muonHistList[j]->GetYaxis()->SetTitleSize(0.040);
    muonHistList[j]->GetXaxis()->SetRangeUser(30,1200);
    muonHistList[j]->SetLineWidth(2);
    muonHistList[j]->SetLineColor(kBlue+2);
    
    // Display the final fit produced for the plot
    if(showFit){
      //Search for peaks
      TSpectrum *spec = new TSpectrum(2);
      spec->Search(muonHistList[j], 3, "nobackground", 0.5);
      float* xArray = spec->GetPositionX();
      float spectrumX = *max_element(xArray, xArray+2);

      //Fit around the second peak
      TF1 *f1 = new TF1("f1", "pol2", spectrumX-65, spectrumX+65);
      muonHistList[j]->Fit("f1","Rq");

      double maxFitX=f1->GetMaximumX();

      bool keepFitting = true;
      int count=1;
      while(keepFitting)
      {

        f1 = new TF1("f1", "pol2", maxFitX-65, maxFitX+65);
        muonHistList[j]->Fit("f1","Rq");
        if(abs(maxFitX - f1->GetMaximumX()) <= 3){//Play with threshhold
          keepFitting = false;
          //cout << "Fit stabilized after " << count << " iteration(s)." << endl;
        }
        else if(count > 20) {
          //cout << "Fit never stabilized, used 20 iterations for fit" << endl;
          break;
        }
        count++;
        //maxFitX=f1->GetMaximumX();
      }
    }

    TSpectrum *spec = new TSpectrum(2);
    spec->Search(muonHistList[j], 3, "nobackground", 0.5);

    // don't be a good programmer and clean up your memory, or you will lose your plots

  }

  if (j == maxPlot)
    cout << "Over " << maxPlot << " failed fits!" << endl;


}
