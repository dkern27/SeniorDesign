
// ----------------------------------------------------------------------
// Header files to include

// C++
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <string>
#include <sstream> // string manipulation
#include <vector>

// ROOT
// Files for certain specific of plots
#include "TROOT.h"
#include "TF1.h"
#include "TH1F.h"   // 1D Histogram
#include "TH2F.h"   // 2D Histogram
#include "TGraph.h" // Basic scatter plot
#include "TProfile.h" // Profile plot - useful for plotting the *mean* of one variable against another
#include "TFile.h"
#include "TVector3.h"

// Generic ROOT files, always include these
#include "TMath.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"

// ----------------------------------------------------------------------
// Functions go here

// This function is really useful for debugging. Simply calling
// PL()
// will output the current line of code that the CINT compiler is currently on
// to the screen.
#define PL() printf("%i\n",__LINE__);

//----------------------------------------------------------------------
using namespace std; // So the compiler interprets cout as std::cout

struct DataPoint {
  double energy;
  double angle;
  double wcd_tot;
  double scint_tot;
};

void usage();
vector<Double_t> doPlotsByAngle(vector<DataPoint>& data, double angle, vector<Double_t> energies);

// ----------------------------------------------------------------------
// Globals

// ----------------------------------------------------------------------


int main(int argc, char **argv) // argc = number arguments, argv = array containing them
{                           // so for example ./ReadADST $filename1 $filename2
                            // has argc = 3.

  // Check if the user specified what files to analyse
  if (argc < 2) {
    usage();
    return 1;
  }

  // Makes your plots look "normal". The default background isn't plain white.
  //gROOT->Reset();
  gROOT->SetStyle("Plain");
  // This makes all plots drawn have X and Y grid lines.
  gStyle->SetPadGridX(true);
  gStyle->SetPadGridY(true);

  vector<Double_t> energies = {18.6, 19.0, 19.5, 20.0};
  // Declare plots here

  TGraph *mip_vem_all = new TGraph();
  mip_vem_all->SetMarkerStyle(20);
  mip_vem_all->SetMarkerColor(kRed);

  TGraph *mip_vem_200 = new TGraph();
  mip_vem_200->SetMarkerStyle(20);
  mip_vem_200->SetMarkerColor(kGreen);

  TF1 *f_all = new TF1("f_all","pol1",0,3000);
  TF1 *f_200 = new TF1("f_200","pol1",0,3000);
  TH2F *slopes = new TH2F("slopes","Slopes",5,0,5,5,0,5);

  // -------------------

  ifstream input_file;
  string line, file;

  int station_id, scin_sat_status, wcd_sat_status;
  double r_mc;
  double scint_tot, scin_em, scin_mu;
  double wcd_tot, wcd_em, wcd_mu;

  double energy, theta;

  int index_all=0, index_200=0;

  vector<DataPoint> data;
  // ----------------------------------------------------------------------
  // Loop over files
  // read the file as the second to last argument
  for (int iFile = 1; iFile <= argc - 1; iFile++) {

    input_file.open(argv[iFile]);

    file = argv[iFile];

    // Fetching energy (from lgenergyXX.XX)
    string::size_type e = file.find('y');
    string::size_type ee = file.find('/');
    stringstream(file.substr(e+1,ee-e-1))>>energy;

    // Fetching theta (from thetaXX)
    string::size_type t = file.find('a');
    string::size_type tt = file.find('/');
    stringstream(file.substr(t+1,tt-t-1))>>theta;

    cout << "reading file: " << file << " Log(E/eV)= " << energy << " Zenith angle (Deg)= " << theta << endl;

    // First line is made of headers - ignoring  
    getline(input_file,line);

    while (input_file) 
    {
      input_file >> station_id >> r_mc
      >> scint_tot >> scin_em >> scin_mu 
      >> wcd_tot >> wcd_em >> wcd_mu
      >> scin_sat_status >> wcd_sat_status;
      
      DataPoint newPoint = {energy, theta, wcd_tot, scint_tot};
      data.push_back(newPoint);

      input_file.close();

    }
  }

  // ----------------------------------------------------------------------
  // If you want to draw any plots do the screen, you first need to declare a TApplication,
  // the .Run() it before the end of your main function.

  // A TApplication is required for drawing
  // Once a TApplication is declared it absorbs the argc and argv parameters, so if you want to
  // do anything with them (like in this code), declare your TApplication afterwards. Otherwise it
  // can be initialised right at the start of main().
  
  TApplication theApp("app", &argc, argv);  
  printf("Complete.\n");

  gStyle->SetPalette(1);
  
  vector<Double_t> fit_slopes = doPlotsByAngle(data, 0., energies);
  TCanvas *c = new TCanvas();
  for (int i = 0; i < fit_slopes.size(); i ++)
  {
    slopes->Fill(energies[i], 0.0, fit_slopes[i]);
  }

  slopes->Draw("colz");

  c->Update();

  theApp.Run();

  return 0;

}

void usage()
{
  printf("Usage:\n"
  "Describe here \n");
}

vector<Double_t> doPlotsByAngle(vector<DataPoint>& data, double angle, vector<Double_t> energies)
{
  vector<TGraph*> graphs;
  for(int i = 0; i < energies.size(); i++)
  {
    graphs.push_back(new TGraph());
  }

  int index186=0, index190=0, index195=0, index200=0;
  for (DataPoint d : data)
  {
    if(d.angle == angle)
    {

      if (d.energy == energies[0])
        graphs[0]->SetPoint(index186, d.wcd_tot, d.scint_tot);
      else if (d.energy == energies[1])
        graphs[1]->SetPoint(index190, d.wcd_tot, d.scint_tot);
      else if (d.energy == energies[2])
        graphs[2]->SetPoint(index195, d.wcd_tot, d.scint_tot);
      else if (d.energy == energies[3])
        graphs[3]->SetPoint(index200, d.wcd_tot, d.scint_tot);
    }
  }
  TF1 *fit = new TF1("fit","pol1",0,3000);
  vector<Double_t> fit_slopes;
  for (TGraph* graph : graphs)
  {
    graph->Draw();
    graph->Fit("fit");
    fit_slopes.push_back(fit->GetParameter(1));
  }
  return fit_slopes;
}
