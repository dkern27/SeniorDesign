// Header files to include

// C++
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <string>
#include <sstream> // string manipulation
#include <vector>
#include <set>
#include <cctype>

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
#include "TLegend.h"

// Generic ROOT files, always include these
#include "TMath.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"

#include "DataPoint.h"
#include "Plotter.h"

// This function is really useful for debugging. Simply calling
// PL()
// will output the current line of code that the CINT compiler is currently on
// to the screen.
#define PL() printf("%i\n",__LINE__);

//----------------------------------------------------------------------
using namespace std; // So the compiler interprets cout as std::cout

//Function Declarations
void usage();
bool isInteger(string str);
set<string> getStationIds();
set<string> getTwoStationIds();

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
	const vector<double> ENERGIES = {18.6, 19.0, 19.5, 20.0};
	const vector<double> ANGLES = {0, 12, 25, 36, 45, 53};
	const vector<int> CORE_DISTANCES = {600, 800, 1000};

  // -------------------
	vector<DataPoint> data;
	vector<DataPoint> minMaxData;
	
  // ----------------------------------------------------------------------
  // Loop over files
  // read the file as the second to last argument
	cout << "Reading in data..." << endl;
	for (int iFile = 1; iFile <= argc - 1; iFile++) 
	{
		ifstream input_file;
		string file;

		input_file.open(argv[iFile]);
		file = argv[iFile];

		vector<DataPoint> newData = DataPoint::ReadFile(input_file, file);
		input_file.close();

		vector<DataPoint> newData2 = DataPoint::GetMinAndMaxData(newData);

		data.insert(data.end(), newData.begin(), newData.end());
		minMaxData.insert(minMaxData.end(), newData2.begin(), newData2.end());
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


	cout << "What plots do you want to display?:" << endl;
	cout << "(ex: 23 will plot options 2 and 3)" << endl;
	cout << "1: Plot all points" << endl;
	cout << "2: Show Fit Slopes on a 2D histogram" << endl;
	cout << "3: Slopes at core distances, points" << endl;
	cout << "4: Slopes at core distances, candle plot" << endl;
	cout << "5: Slopes at core distances, compare two stations" << endl;

	string options;
	cin >> options;

	for (char& c : options) 
	{
		int n = c - '0';
		if (n > 5 || n < 1)
		{
			cout << c << " is not a valid option, skipping" << endl;
		}
		else 
		{
			if (n == 1)
			{
        		// PLOT POINTS
				TCanvas* c1 = new TCanvas();
				TGraph* g1 = Plotter::plotData(data);
				g1->Draw("AP");
				c1->Update();
			} 
			else if (n == 2)
			{
        		// FIT SLOPES 2D HISTOGRAM
				TCanvas* c2 = new TCanvas();
				TH2F* fit_slopes = Plotter::make2DHistogram(data, ANGLES, ENERGIES, false);
				fit_slopes->SetTitle("Fit Slopes");
				fit_slopes->Draw("colz");
				c2->Update();

				TCanvas* c3 = new TCanvas();
				TH2F* corrected_fit_slopes = Plotter::make2DHistogram(data, ANGLES, ENERGIES, true);
				corrected_fit_slopes->SetTitle("Corrected Fit Slopes");
				corrected_fit_slopes->Draw("colz");
				c3->Update();

				TCanvas* c4 = new TCanvas();
				TH2F* corrected_fit_slopes2 = Plotter::make2DHistogram(minMaxData, ANGLES, ENERGIES, true);
				corrected_fit_slopes2->SetTitle("Corrected Fit Slopes of min/max scint_tot data");
				corrected_fit_slopes2->Draw("colz");
				c4->Update();
			} 
			else if (n == 3) 
			{
        		// FOR PLOTTING POINTS OF SLOPES AT CORE DIST
				for (double energy : ENERGIES)
				{
					for (double angle : ANGLES)
					{
						string title = "Energy: " + to_string(energy) + " Angle: " + to_string(angle);
						TCanvas* c = new TCanvas();
						TGraph* g = Plotter::getSlopesForCoreDistance(data, angle, energy);
						g->GetXaxis()->SetTitle("Core Distance");
						g->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
						g->SetMarkerSize(.75);
						g->SetMarkerStyle(20);
						g->SetTitle(title.c_str());
						g->Draw("AP");
						c->Update();
					}
				}
			} 
			else if (n == 4)
			{
				set<string> stationIds = getStationIds();
       			// FOR PLOTTING CANDLE PLOTS OF SLOPES AT CORE DIST
				for (double energy : ENERGIES)
				{
					for (double angle : ANGLES)
					{
						TCanvas* c = new TCanvas();
						TH2F* g = Plotter::getSlopeVsDistanceCandlePlot(data, angle, energy, stationIds);
						g->Draw("candle2");
						c->Update();
					}
				}
			}
			else if (n == 5)
			{
				set<string> stationIds = getTwoStationIds();
				string stationOne = *stationIds.begin();
				string stationTwo = *next(stationIds.begin(),1);
				for (double energy : ENERGIES)
				{
					for (double angle : ANGLES)
					{
						TCanvas* c = new TCanvas();
						TGraph* g = Plotter::getSlopeVsDistanceSingleStation(data, CORE_DISTANCES, angle, energy, stationOne, kRed, kFullCircle);
						TGraph* g2 = Plotter::getSlopeVsDistanceSingleStation(data, CORE_DISTANCES, angle, energy, stationTwo, kBlue, kFullSquare);
						g->Draw("ALP");
						g2->Draw("LPSAME");

						TLegend* legend = new TLegend(0.1, 0.75, 0.25, 0.9);
						legend->AddEntry(g, stationOne.c_str(), "LP");
						legend->AddEntry(g2, stationTwo.c_str(), "LP");
						legend->Draw();

						c->Update();
					}
				}
			}
		}

	}

	theApp.Run();

	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// END OF MAIN ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

//DO THIS LATER
void usage()
{
	printf("Usage:\n"
		"Describe here \n");
}

bool isInteger(string str)
{
	if(str.length() == 0)
		return false;

	for(int i = 0; i < str.length(); i++)
	{
		if(!isdigit(str[i]))
			return false;
	}
	return true;
}

set<string> getStationIds()
{
	cout << "Enter the last two digits of the station ids one at a time. No station ids will plot all station ids. Enter 'done' to start plotting." << endl;
	set<string> stationIds;
	string sid = "";
	while (sid != "done")
	{
		cout << "Station ID: ";
		cin >> sid;
		if (isInteger(sid))
		{
			stationIds.insert(sid);
		}
		else if (sid != "done")
		{
			cout << "Input was not an integer" << endl;
		}
	}
	return stationIds;
}

set<string> getTwoStationIds()
{
	cout << "Enter the last two digits of two stations' ids one at a time" << endl;
	set<string> stationIds;
	string sid = "";
	while(stationIds.size() < 2)
	{
		cout << stationIds.size() + 1 << ": ";
		cin >> sid;
		if (isInteger(sid))
		{
			stationIds.insert(sid);
		}
		else
		{
			cout << "Input was not an integer" << endl;
		}
	}
	return stationIds;
}





