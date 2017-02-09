
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
#include "TLegend.h"

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

	//Grouped by energy
	TGraph *mip_vem_all = new TGraph();
	mip_vem_all->SetMarkerStyle(20);
	mip_vem_all->SetMarkerSize(1);
	mip_vem_all->SetMarkerColor(kRed);

	TGraph *mip_vem_186 = new TGraph();
	mip_vem_186->SetMarkerStyle(20);
	mip_vem_186->SetMarkerSize(0.8);
	mip_vem_186->SetMarkerColor(kBlue);

	TGraph *mip_vem_190 = new TGraph();
	mip_vem_190->SetMarkerStyle(20);
	mip_vem_190->SetMarkerSize(0.6);
	mip_vem_190->SetMarkerColor(kYellow);

	TGraph *mip_vem_195 = new TGraph();
	mip_vem_195->SetMarkerStyle(20);
	mip_vem_195->SetMarkerSize(0.4);
	mip_vem_195->SetMarkerColor(kCyan);

	TGraph *mip_vem_200 = new TGraph();
	mip_vem_200->SetMarkerStyle(20);
	mip_vem_200->SetMarkerSize(0.2);
	mip_vem_200->SetMarkerColor(kGreen);


	//Grouped by angle
	TGraph *mip_vem_alla = new TGraph();
	mip_vem_alla->SetMarkerStyle(20);
	mip_vem_alla->SetMarkerSize(0.4);
	mip_vem_alla->SetMarkerColor(kRed);

	TGraph *mip_vem_00a = new TGraph();
	mip_vem_00a->SetMarkerStyle(20);
	mip_vem_00a->SetMarkerSize(0.4);
	mip_vem_00a->SetMarkerColor(kBlue);

	TGraph *mip_vem_12a = new TGraph();
	mip_vem_12a->SetMarkerStyle(20);
	mip_vem_12a->SetMarkerSize(0.4);
	mip_vem_12a->SetMarkerColor(kYellow);

	TGraph *mip_vem_25a = new TGraph();
	mip_vem_25a->SetMarkerStyle(20);
	mip_vem_25a->SetMarkerSize(0.4);
	mip_vem_25a->SetMarkerColor(kCyan);

  TGraph *mip_vem_36a = new TGraph();
  mip_vem_36a->SetMarkerStyle(20);
  mip_vem_36a->SetMarkerSize(0.4);
  mip_vem_36a->SetMarkerColor(kGreen);

  TGraph *mip_vem_45a = new TGraph();
  mip_vem_45a->SetMarkerStyle(20);
  mip_vem_45a->SetMarkerSize(0.4);
  mip_vem_45a->SetMarkerColor(kOrange);

  TGraph *mip_vem_53a = new TGraph();
  mip_vem_53a->SetMarkerStyle(20);
  mip_vem_53a->SetMarkerSize(0.4);
  mip_vem_53a->SetMarkerColor(kViolet);
 //  // -------------------

	ifstream input_file;
	string line, file;

	int station_id, scin_sat_status, wcd_sat_status;
	double r_mc;
	double scint_tot, scin_em, scin_mu;
	double wcd_tot, wcd_em, wcd_mu;

	double energy, theta;

	int index_all=0, index_186=0, index_190=0, index_195=0, index_200=0, index_alla=0, index_00a=0, index_12a=0,
  index_25a=0, index_36a=0, index_45a=0, index_53a=0;


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

		//cout << "reading file: " << file << " Log(E/eV)= " << energy << " Zenith angle (Deg)= " << theta << endl;

    // First line is made of headers - ignoring  
		getline(input_file,line);


		while (input_file) {
			input_file >> station_id >> r_mc
			>> scint_tot >> scin_em >> scin_mu 
			>> wcd_tot >> wcd_em >> wcd_mu
			>> scin_sat_status >> wcd_sat_status;
			if(theta != 0)
			{
				double phi = atan(20000/((20000/tan(theta * TMath::Pi()/180)) - r_mc/sin(theta * TMath::Pi()/180)));
				scint_tot = scint_tot / abs(cos(phi));
			}
			if (theta==53) {
				mip_vem_all->SetPoint(index_all,wcd_tot,scint_tot);
				index_all++;

				if (energy==18.60) 
				{
					mip_vem_186->SetPoint(index_186,wcd_tot,scint_tot);
					index_186++;
				}
				if (energy==19.00) 
				{
					mip_vem_190->SetPoint(index_190,wcd_tot,scint_tot);
					index_190++;
				}
				if (energy==19.50) 
				{
					mip_vem_195->SetPoint(index_195,wcd_tot,scint_tot);
					index_195++;
				}
				if (energy==20.00) 
				{
					mip_vem_200->SetPoint(index_200,wcd_tot,scint_tot);
					index_200++;
				}
			}

			if (energy==20.00) {
				mip_vem_alla->SetPoint(index_alla,wcd_tot,scint_tot);
				index_alla++;

				if (theta==00) 
				{
					mip_vem_00a->SetPoint(index_00a,wcd_tot,scint_tot);
					index_00a++;
				}
				if (theta==12) 
				{
					mip_vem_12a->SetPoint(index_12a,wcd_tot,scint_tot);
					index_12a++;
				}
				if (theta==25) 
				{
					mip_vem_25a->SetPoint(index_25a,wcd_tot,scint_tot);
					index_25a++;
				}
				if (theta==36) 
				{
					mip_vem_36a->SetPoint(index_36a,wcd_tot,scint_tot);
					index_36a++;
				}
				if (theta==45) 
				{
					mip_vem_45a->SetPoint(index_45a,wcd_tot,scint_tot);
					index_45a++;
				}
				if (theta==53) 
				{
					mip_vem_53a->SetPoint(index_53a,wcd_tot,scint_tot);
					index_53a++;
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
	mip_vem_195->Draw("Psame");
	mip_vem_190->Draw("Psame");
	mip_vem_186->Draw("PSame");
	mip_vem_all->Fit("pol1");

	TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
    leg->AddEntry(mip_vem_all,"All Energies","p");
    leg->AddEntry(mip_vem_186,"18.6","p");
    leg->AddEntry(mip_vem_190,"19.0","p");
    leg->AddEntry(mip_vem_195,"19.5","p");
    leg->AddEntry(mip_vem_200,"20.0","p");
    leg->Draw("PSame");

	c->Update();


  // TCanvas *c2 = new TCanvas();
  // mip_vem_00a->Draw("AP");
  // mip_vem_12a->Draw("Psame");
  // mip_vem_25a->Draw("Psame");
  // mip_vem_36a->Draw("PSame");
  // mip_vem_45a->Draw("PSame");
  // mip_vem_53a->Draw("PSame");
  // c2->Update();

	theApp.Run();

	return 0;

}

void
usage()
{
	printf("Usage:\n"
		"Describe here \n");
}
