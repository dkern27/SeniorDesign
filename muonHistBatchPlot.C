muonHistBatchPlot()
{

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0); 

  // open the root file where the muon histograms are stored
  TFile *f = new TFile("test.root");
  
  //limit the number of histograms to attempt to plot
   const int maxPlot = 3; 

  // grab the root tree from where it was stored intthe root file
  TTree *muonTree = (TTree*)f->Get("muonTree");
  
  // find the size of the tree to limit looping beyond the end of the tree
  const int treeSize = muonTree->GetEntries();

  // In order to get object information out of he branch, a valid pointer must pre-exist
  // the Tree GetEntry(i) call for the ith object
  TH1I *muonHistList[maxPlot];

  // I know my re-binning wil cause warnings some of the time, ignoring them
  gErrorIgnoreLevel = kWarning +2;

  for(int i = 0; i < treeSize && i < maxPlot; i++) {

    // It is not good enough for a pointer of the correct object to exist, it must be a valid pointer
    // either to an object or NULL in order for the GetEntry() call to work
    muonHistList[i] = NULL;

    // must tell the branch where to put the object prior to the GetEntry() call
    muonTree->SetBranchAddress("muonHist", &muonHistList[i]);

    // note: There are other values/variables stored in the tree, but i've only laid the groundwork to fetch
    // one of them ( the histogram I want to plot )

    // get the ith histogram from the tree
    muonTree->GetEntry(i);
   
    // make a new canvas each time, or the script will re-use the canvas that was last in scope
    // and overwrite the previous histogram
    TCanvas* c = new TCanvas();

    // make some style changes, for demonstration
    muonHistList[i]->Draw();
    muonHistList[i]->Rebin(i%9+1);
    muonHistList[i]->GetXaxis()->CenterTitle();
    muonHistList[i]->GetYaxis()->CenterTitle();
    muonHistList[i]->GetYaxis()->SetTitleOffset(1.15);
    muonHistList[i]->GetYaxis()->SetTitleSize(0.040);
    muonHistList[i]->GetXaxis()->SetRangeUser(30,1200);
    muonHistList[i]->SetLineWidth(2);
    muonHistList[i]->SetLineColor(kBlue+2);
    
    // don't be a good programmer and clean up your memory, or you will lose your plots

  }


}
