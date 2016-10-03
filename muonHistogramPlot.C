muonHistogramPlot()
{
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetPalette(1,0); 

  TFile *f = new TFile("muonHistogram.root");
  
  char title[200];
  const bool makeTracePlots = false;
  
  

  // histogram plotting

  // muon histogram
  TCanvas *c1 = new TCanvas();
  sprintf(title, "%s", "A30 integrated muon peaks; A30 integrated peak counts; Number of muons ");
  muonHistogram->SetTitle(title);
  muonHistogram->SetAxisRange(0,500);
  muonHistogram->Draw();
  muonHistogram->GetXaxis()->CenterTitle();
  muonHistogram->GetYaxis()->CenterTitle();

  //Search for peaks
  TSpectrum *spec = new TSpectrum(2);
  spec->Search(muonHistogram, 1, "", 0.05);
  TList* functions = muonHistogram -> GetListOfFunctions();
  TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
  int npeaks = spec->GetNPeaks();
  Double_t* pmXArray = pm->GetX();
  //Print the X positions of peaks
  for (Int_t i=0; i<npeaks; i++)
  {
  	cout << "Peak " << i << ": "<< pmXArray[i] << endl;
  }
  //Fit around the second peak
  TF1 *f1 = new TF1("f1", "pol2", pmXArray[1]-65,pmXArray[1]+65);
  muonHistogram->Fit("f1","R");

  float maxFitX=f1->GetMaximumX();
  //Fit based on peak of previous fit
  for(int i=0; i<5; i++)
  {
  	maxFitX=f1->GetMaximumX();
  	f1 = new TF1("f1", "pol2", maxFitX-65, maxFitX+65);
  	muonHistogram->Fit("f1","R");
  }

  float param0 = f1->GetParameter(0);
  float param0err = f1 ->GetParError(0);
  ofstream file;
  file.open("parameter0.txt", std::ios_base::app);
  file << param0 << " " << param0err << endl;
  // muon traces
  if (makeTracePlots) {
    TCanvas *c2 = new TCanvas();
    sprintf(title, "%s", "Sample A30 muon traces (vertically offset for visability); muon file timing bin (10ns each); A30 counts ");
    muonTrace0->SetTitle(title);
    muonTrace0->GetYaxis()->SetRangeUser(500, 1100);
    muonTrace0->Draw();
    muonTrace1->Draw("same");
    muonTrace2->Draw("same");
    muonTrace3->Draw("same");
    muonTrace4->Draw("same");
  }

}
