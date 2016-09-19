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

  //Try High Resolution Search
  //Int_t nbins = 500;
  //Int_t xMin = 0;
  //Int_t xMax = 500;
  //TH1I *d = new TH1I("d","", nbins,xMin, xMax);
  //float *source = new Double_t[nbins];
  //float *dest = new Double_t[nbins];
  //for(int i=0; i<nbins;i++) source[i]=muonHistogram->GetBinContent(i+1);
  TSpectrum *spec = new TSpectrum(10);
  spec->Search(muonHistogram, 1,"", 0.05);
  //spec->SearchHighRes(source,dest,nbins,1,10,false,3,kTRUE,5);
  //for (i=0; i<nbins;i++) d->SetBinContent(i+1, dest[i]);
  //d->SetLineColor(kRed);
  //d->Draw("SAME");
  //spec->Draw("SAME");

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
