
// ----------------------------------------------------------------------
// Header files to include

// C++
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <string>
#include <sstream> // string manipulation

// ROOT
// Files for certain specific of plots
#include "TROOT.h"
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

void usage();

// ----------------------------------------------------------------------
// Globals

// ----------------------------------------------------------------------

int 
main(int argc, char **argv) // argc = number arguments, argv = array containing them
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

  // Declare plots here

  TGraph *mip_vem_all = new TGraph();
  mip_vem_all->SetMarkerStyle(20);
  mip_vem_all->SetMarkerColor(kRed);

  TGraph *mip_vem_200 = new TGraph();
  mip_vem_200->SetMarkerStyle(20);
  mip_vem_200->SetMarkerColor(kGreen);

  // -------------------

  ifstream input_file;
  string line, file;

  int station_id, scin_sat_status, wcd_sat_status;
  double r_mc;
  double scint_tot, scin_em, scin_mu;
  double wcd_tot, wcd_em, wcd_mu;

  double energy, theta;

  int index_all=0, index_200=0;

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

    while (input_file) {
      input_file >> station_id >> r_mc
      >> scint_tot >> scin_em >> scin_mu 
      >> wcd_tot >> wcd_em >> wcd_mu
      >> scin_sat_status >> wcd_sat_status;
      
      if (theta==0.0) {

       mip_vem_all->SetPoint(index_all,wcd_tot,scint_tot);
       index_all++;

       if (energy==19.50) 
       {
         mip_vem_200->SetPoint(index_200,wcd_tot,scint_tot);
         index_200++;
       }
     }
   }

   input_file.close();

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

 TCanvas *c = new TCanvas();
 mip_vem_all->Draw("AP");
 mip_vem_all->GetXaxis()->SetTitle("WCD [VEM]");
 mip_vem_all->GetYaxis()->SetTitle("SSD [MIP]");
 mip_vem_200->Draw("Psame");
 mip_vem_all->Fit("pol1");
 c->Update();

 theApp.Run();

 return 0;

}

void
usage()
{
  printf("Usage:\n"
    "Describe here \n");
}
