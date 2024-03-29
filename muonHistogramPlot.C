#include <TFormula.h>

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
  float maxFitY=f1->GetMaximum();

  bool keepFitting = true;
  int count=1;
  while(keepFitting)
  {

    f1 = new TF1("f1", "pol2", maxFitX-65, maxFitX+65);
    muonHistogram->Fit("f1","R");
    if(abs(maxFitX - f1->GetMaximumX()) <= 0.001){
      keepFitting = false;
      cout << "Fit stabilized after " << count << " iteration(s)." << endl;
    }
    else if(count >20) {
      keepFitting = false;
      cout << "Fit never stabilized, used 20 iterations for fit" << endl;
    }
    count++;
    maxFitX=f1->GetMaximumX();
    maxFitY=f1->GetMaximum();
  }

  //Output parameters
  //maxFitX = f1->GetMaximumX();
  float c = f1->GetParameter(0);
  float cerr = f1 ->GetParError(0);
  float b = f1->GetParameter(1);
  float berr = f1 ->GetParError(1);
  float a = f1->GetParameter(2);
  float aerr = f1 ->GetParError(2);

  float maxX = (b*-1)/(2*a);
  float dxda = b/(2*a*a);
  float dxdb = -1/(2 * a);
  float varianceX = pow(dxda*aerr, 2) + pow(dxdb * berr, 2);
  float stdDevX = sqrt(varianceX);


  float dyda = maxFitX*maxFitX;
  float dydb = maxFitX;
  float varianceY = pow(dyda*aerr, 2) + pow(dydb*berr, 2) + pow(cerr, 2);
  float stdDevY = sqrt(varianceY);

  cout << "X: " << maxFitX << " Variance: " << varianceX << endl;
  cout << "Y: " << maxFitY << " Variance: " << varianceY << endl;
  ofstream file;
  file.open("parameter0.txt", std::ios_base::app);
  file << c << " " << cerr << endl;

  
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
