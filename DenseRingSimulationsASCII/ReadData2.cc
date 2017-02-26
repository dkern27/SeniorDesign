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

// This function is really useful for debugging. Simply calling
// PL()
// will output the current line of code that the CINT compiler is currently on
// to the screen.
#define PL() printf("%i\n",__LINE__);

//----------------------------------------------------------------------
using namespace std; // So the compiler interprets cout as std::cout

//struct used to hold data read in from file
struct DataPoint {
	double energy;
	double angle;
	double wcd_tot;
	double scint_tot;
	double corrected_scint_tot;
};

//Function Declarations
void usage();
vector<DataPoint> ReadFile(ifstream& input_file, string file);
vector<double> getFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);
vector<double> getCorrectedFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);
double doCorrectionOne(double scint_tot, double angle, double coreDistance, double height);
double doCorrectionTwo(double scint_tot, double angle, double coreDistance, double height);


//Global Variables

/*
argc = number arguments, argv = array containing them so for example ./ReadADST $filename1 $filename2
has argc = 3.
*/
int main(int argc, char **argv)
{
  // Check if the user specified what files to analyse
	if (argc < 2) 
	{
		usage();
		return 1;
	}

  // Makes your plots look "normal". The default background isn't plain white.
  //gROOT->Reset();
	gROOT->SetStyle("Plain");
  // This makes all plots drawn have X and Y grid lines.
	gStyle->SetPadGridX(true);
	gStyle->SetPadGridY(true);
	//Remove stats box
	gStyle->SetOptStat(0);

	//Maybe get these values when we read the files?
	vector<double> energies = {18.6, 19.0, 19.5, 20.0};
	vector<double> angles = {0, 12, 25, 36, 45, 53};

	//args: name, title, num_bins_x, min_x, max_x, num_bins_y, min_y, max_y
	TH2F *fit_slopes = new TH2F("slopes", "Slopes", energies.size(), 0, energies.size(), angles.size(), 0, angles.size());
	TH2F *corrected_fit_slopes = new TH2F("slopes", "Slopes", energies.size(), 0, energies.size(), angles.size(), 0, angles.size());
	//Set according to range of z values
	fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
	corrected_fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);

  // -------------------
	vector<DataPoint> data;
	
  // ----------------------------------------------------------------------
  // Loop over files
  // read the file as the second to last argument
	for (int iFile = 1; iFile <= argc - 1; iFile++) 
	{
		ifstream input_file;
		string file;

		input_file.open(argv[iFile]);
		file = argv[iFile];

		vector<DataPoint> newData = ReadFile(input_file, file);
		input_file.close();
		data.insert(data.end(), newData.begin(), newData.end());
	}

  // ----------------------------------------------------------------------
  // If you want to draw any plots to the screen, you first need to declare a TApplication,
  // then call .Run() before the end of your main function.

  // A TApplication is required for drawing
  // Once a TApplication is declared it absorbs the argc and argv parameters, so if you want to
  // do anything with them (like in this code), declare your TApplication afterwards. Otherwise it
  // can be initialised right at the start of main().

	TApplication theApp("app", &argc, argv);  
	printf("Complete.\n");

	gStyle->SetPalette(1);

	TCanvas *c = new TCanvas();
	//Go through each angle and make a graph for each energy
	for (int i = 0; i < angles.size(); i++)
	{
		vector<double> slopes = getFitSlopes(data, angles[i], energies);
		for (int j = 0; j < slopes.size(); j++)
		{
			cout << slopes[j] << endl;
			fit_slopes->Fill(j, i, slopes[j]);
		}
	}
	fit_slopes->Draw("colz");
	c->Update();

	TCanvas *c2 = new TCanvas();
	//Go through each angle and make a graph for each energy
	for (int i = 0; i < angles.size(); i++)
	{
		vector<double> corrected_slopes = getCorrectedFitSlopes(data, angles[i], energies);
		for (int j = 0; j < corrected_slopes.size(); j++)
		{
			cout << corrected_slopes[j] << endl;
			corrected_fit_slopes->Fill(j, i, corrected_slopes[j]);
		}
	}
	corrected_fit_slopes->Draw("colz");
	c2->Update();

	theApp.Run();

	return 0;

}

//DO THIS LATER
void usage()
{
	printf("Usage:\n"
		"Describe here \n");
}

/*
Reads in a file that has already been opened
params
	ifstream input_file file that has been opened to be read
	string file name of file being read
*/
vector<DataPoint> ReadFile(ifstream& input_file, string file)
{
	vector<DataPoint> data;
	int station_id, scin_sat_status, wcd_sat_status;
	double r_mc;
	double scint_tot, scin_em, scin_mu;
	double wcd_tot, wcd_em, wcd_mu;

	double energy, theta;

	string line;

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

	while (input_file) 
	{
		input_file >> station_id >> r_mc
		>> scint_tot >> scin_em >> scin_mu 
		>> wcd_tot >> wcd_em >> wcd_mu
		>> scin_sat_status >> wcd_sat_status;

		//Using 20000 as height of shower axis
		DataPoint newPoint = {energy, theta, wcd_tot, scint_tot, doCorrectionOne(scint_tot, theta, r_mc, 20000)};
		data.push_back(newPoint);
	}
	return data;
}

/*
Plots and fits data filtered by angle and energy
params
	vector<DataPoint>& data All data to be filtered through
	double angle the angle to filter by
	vector<double> energies the energies to filter by
*/
vector<double> getFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies)
{
	vector<TGraph*> graphs;
	//If we want a graph with all points
  	//graphs.push_back(new TGraph()); //All energies
	for(int i = 0; i < energies.size(); i++)
	{
		graphs.push_back(new TGraph());
	}

	int indexes[4] = {0, 0, 0, 0};

	//Filters into several graphs
	for (DataPoint d : data)
	{
		if(d.angle == angle)
		{
			//graphs[0]->SetPointindexes[0], d.wcd_tot, d.scint_tot);
			//indexes[0]++;
			//This could be simplified with an int set in the if/else, then add to graph after
			if (d.energy == energies[0])
			{
				graphs[0]->SetPoint(indexes[0], d.wcd_tot, d.scint_tot);
				indexes[0]++;
			}
			else if (d.energy == energies[1])
			{
				graphs[1]->SetPoint(indexes[1], d.wcd_tot, d.scint_tot);
				indexes[1]++;
			}
			else if (d.energy == energies[2])
			{
				graphs[2]->SetPoint(indexes[2], d.wcd_tot, d.scint_tot);
				indexes[2]++;
			}
			else if (d.energy == energies[3])
			{
				graphs[3]->SetPoint(indexes[3], d.wcd_tot, d.scint_tot);
				indexes[3]++;
			}
		}
	}
	//Fit each graph and get fit result
	TF1 *fit = new TF1("fit","pol1",0,3000);
	vector<double> slopes;
	for (TGraph* graph : graphs)
	{
		graph->Draw();
		graph->Fit("fit", "q");
		slopes.push_back(fit->GetParameter(1));
	}
	return slopes;
}

vector<double> getCorrectedFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies)
{
	vector<TGraph*> graphs;
	//If we want a graph with all points
  	//graphs.push_back(new TGraph()); //All energies
	for(int i = 0; i < energies.size(); i++)
	{
		graphs.push_back(new TGraph());
	}

	int indexes[4] = {0, 0, 0, 0};

	//Filters into several graphs
	for (DataPoint d : data)
	{
		if(d.angle == angle)
		{
			//graphs[0]->SetPointindexes[0], d.wcd_tot, d.scint_tot);
			//indexes[0]++;
			//This could be simplified with an int set in the if/else, then add to graph after
			if (d.energy == energies[0])
			{
				graphs[0]->SetPoint(indexes[0], d.wcd_tot, d.corrected_scint_tot);
				indexes[0]++;
			}
			else if (d.energy == energies[1])
			{
				graphs[1]->SetPoint(indexes[1], d.wcd_tot, d.corrected_scint_tot);
				indexes[1]++;
			}
			else if (d.energy == energies[2])
			{
				graphs[2]->SetPoint(indexes[2], d.wcd_tot, d.corrected_scint_tot);
				indexes[2]++;
			}
			else if (d.energy == energies[3])
			{
				graphs[3]->SetPoint(indexes[3], d.wcd_tot, d.corrected_scint_tot);
				indexes[3]++;
			}
		}
	}
	//Fit each graph and get fit result
	TF1 *fit = new TF1("fit","pol1",0,3000);
	vector<double> slopes;
	for (TGraph* graph : graphs)
	{
		graph->Draw();
		graph->Fit("fit", "q");
		slopes.push_back(fit->GetParameter(1));
	}
	return slopes;
}

double doCorrectionOne(double scint_tot, double angle, double coreDistance, double height)
{
	if(angle == 0) return scint_tot;
	double phi = atan(height/((height/tan(angle * TMath::Pi()/180)) - coreDistance/sin(angle * TMath::Pi()/180)));
	return scint_tot/abs(cos(phi));
}

double doCorrectionTwo(double scint_tot, double angle, double coreDistance, double height)
{
	if(angle == 0) return scint_tot;
	double phi = atan(height/((height/tan(angle * TMath::Pi()/180)) + coreDistance/sin(angle * TMath::Pi()/180)));
	return scint_tot/abs(cos(phi));
}
