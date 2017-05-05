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
#include "TGraphErrors.h"
#include "TMultiGraph.h"

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
void getConstantEnergyRatioPlots(set<string> stationIds);
void getConstantAngleRatioPlots(set<string> stationIds);

//Function Declarations for Plotters
void plotPoints(vector<DataPoint> data);
void fitSlopes2DHistogram(vector<DataPoint> data, vector<DataPoint> minMaxData);
void mipVemCandlePlots(vector<DataPoint> data);
void mipVemCompareTwoStations(vector<DataPoint> data);
void mipVemConstAngleOrEnergy(vector<DataPoint> data);


//Global Variables
const vector<double> ENERGIES = {18.6, 19.0, 19.5, 20.0};
const vector<double> ANGLES = {0, 12, /*25, 36,*/ 45, 53};
const vector<int> CORE_DISTANCES = {600, 800, 1000};
const vector<string> STATION_IDS = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"};
const vector<Color_t> COLORS = {kBlue, kRed, kMagenta+1, kCyan+2, kGreen+1, kOrange+7};

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
	cout << "3: Slopes at core distances, candle plot" << endl;
	cout << "4: Slopes at core distances, compare two stations" << endl;
	cout << "5: Slopes at core distances, constant angle or energy" << endl;

	string options;
	cin >> options;


	for (char& c : options) 
	{
		int n = c - '0';
		if (n > 6 || n < 1)
		{
			cout << c << " is not a valid option, skipping" << endl;
		}
		else 
		{
			if (n == 1){
        		plotPoints(data);

			} 
			else if (n == 2){
        		fitSlopes2DHistogram(data, minMaxData);
				
			} 
			else if (n == 3){
				mipVemCandlePlots(data);
				
			}
			else if (n == 4){
				mipVemCompareTwoStations(data);
				
			}
			else if (n==5){
				mipVemConstAngleOrEnergy(data);
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

void usage()
{
        cout << "Usage:" << endl;
        cout << "To run: ./DRS_Plots lg*/*/*" << endl;
        cout << "lg*/*/* Is the directories of data" << endl;
        cout << "Description:" << endl;
        cout << "Reads in all specified data files and creates DataPoint objects out of the rows" << endl;
        cout << "Various plotting methods can then be done by following the instructions when the program is run." << endl;

        exit(0);
}


/*
Checks if the string contains only integers
params
	srint str : the possible integer
*/
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

/*
Gets station ids from the user
params
	none
*/
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


/*
Gets only two station ids from the user
params
	none
*/
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

/*
Creates a plot of mip/vem ratio for four different core distances.
Holds energy constant and produces a line for each angle
params
	set<string> stationIds : The station ids the user provided
*/
void getConstantEnergyRatioPlots(set<string> stationIds)
{
	TGraphErrors* graph;
	for (double energy : ENERGIES)
	{
		TCanvas* c = new TCanvas();
		TMultiGraph* mg = new TMultiGraph();
		TLegend* leg = new TLegend(0.75, 0.75, 1.0, 1.0);
		string stationOne = *stationIds.begin();
		string stationTwo = *next(stationIds.begin(),1);
		for(string id : stationIds)
		{
			for (int i = 0; i < ANGLES.size(); i++)
			{
				graph = (TGraphErrors*)gDirectory->Get(Plotter::MakeGraphName(energy, ANGLES[i], id).c_str());
				if(id == stationOne)
				{
					graph->SetMarkerStyle(kFullCircle);
					graph->SetLineStyle(1);
				}
				else
				{
					graph->SetMarkerStyle(kFullTriangleUp);
					graph->SetLineStyle(2);
				}
				graph->SetMarkerColor(COLORS[i%ANGLES.size()]);
				graph->SetLineColor(COLORS[i%ANGLES.size()]);
				mg->Add(graph);
				leg->AddEntry(graph, Plotter::MakeGraphName(energy, ANGLES[i], id).c_str(), "LP");
			}
		}
		mg->SetTitle(("Energy: " + to_string(energy)).c_str());
		mg->Draw("ALP");
		mg->GetXaxis()->SetTitle("Core Distance");
		mg->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
		leg->Draw();
		c->Update();
	}
}

/*
Creates a plot of mip/vem ratio for four different core distances.
Holds angle constant and produces a line for each energy
params
	set<string> stationIds : The station ids the user provided
*/
void getConstantAngleRatioPlots(set<string> stationIds)
{
	TGraphErrors* graph;
	for (double angle : ANGLES)
	{
		TCanvas* c = new TCanvas();
		TMultiGraph* mg = new TMultiGraph();
		TLegend* leg = new TLegend(0.75, 0.75, 1.0, 1.0);
		string stationOne = *stationIds.begin();
		for(string id : stationIds)
		{
			for (int i = 0; i < ENERGIES.size(); i++)
			{
				graph = (TGraphErrors*)gDirectory->Get(Plotter::MakeGraphName(ENERGIES[i], angle, id).c_str());
				if(id == stationOne)
				{
					graph->SetMarkerStyle(kFullCircle);
					graph->SetLineStyle(1);
				}
				else
				{
					graph->SetMarkerStyle(kFullTriangleUp);
					graph->SetLineStyle(2);
				}
				graph->SetMarkerColor(COLORS[i%ENERGIES.size()]);
				graph->SetLineColor(COLORS[i%ENERGIES.size()]);
				mg->Add(graph);
				leg->AddEntry(graph, Plotter::MakeGraphName(ENERGIES[i], angle, id).c_str(), "LP");
			}
		}
		mg->SetTitle(("Angle: " + to_string(angle)).c_str());
		mg->Draw("ALP");
		mg->GetXaxis()->SetTitle("Core Distance");
		mg->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
		leg->Draw();
		c->Update();
	}
}

/*
Plot all of the data on one plot. Uses all energies, angles, core distances. Plots mip vs vem
params
	vector<DataPoint> data : All of the data
*/
void plotPoints(vector<DataPoint> data){
	TCanvas* c1 = new TCanvas();
	TGraph* g1 = Plotter::plotData(data);
	g1->Draw("AP");
	c1->Update();
}

/*
Makes 3 2D histograms of the slopes of the fitted data, filtered by angle and energy.
Plotted angle vs energy. No units, only used to see the trend of the data.

params
	vector<DataPoint> data : all of the data
	vector<DataPoint> mimMaxData : has only the data from the min and max energies and angles
*/
void fitSlopes2DHistogram(vector<DataPoint> data, vector<DataPoint> minMaxData){
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


/*
Creates a candle plot of the mip to vem ratios found by detectors at each given core distance.
Plots seperated by angle and energy.

params
	vector<DataPoint> data : all of the data
*/
void mipVemCandlePlots(vector<DataPoint> data){
	set<string> stationIds = getStationIds();
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

/*
Plots the average mip to vem ratios found by the two desired detectors. Allows a comparison of the detectors.
Plots seperated by angle and energy

params
	vector<DataPoint> data : all of the data
*/

void mipVemCompareTwoStations(vector<DataPoint> data){
	set<string> stationIds = getTwoStationIds();
	string stationOne = *stationIds.begin();
	string stationTwo = *next(stationIds.begin(),1);
	for (double energy : ENERGIES)
	{
		for (double angle : ANGLES)
		{
			TCanvas* c = new TCanvas();
			TGraphErrors* g = Plotter::getSlopeVsDistanceSingleStation(data, CORE_DISTANCES, angle, energy, stationOne, kRed, kFullCircle, 1);
			TGraphErrors* g2 = Plotter::getSlopeVsDistanceSingleStation(data, CORE_DISTANCES, angle, energy, stationTwo, kBlue, kFullSquare, 2);
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

/*
Makes plots of the mip to vem ratio over core distance at two desired detectors.
Plots the data for all energies at each angle, and all angles at each energy.
Allows us to see the affect of angle and energy on the mip to vem ratio

params
	vector<DataPoint> data : all of the data
*/

void mipVemConstAngleOrEnergy(vector<DataPoint> data){
	set<string> stationIds = getTwoStationIds();
	TGraphErrors* graph;
	//Make all the plots we need
	for (double energy : ENERGIES)
	{
		for (double angle : ANGLES)
		{
			for(string stationId : stationIds)
			{
				graph = Plotter::getSlopeVsDistanceSingleStation(data, CORE_DISTANCES, angle, energy, stationId);
				gDirectory->GetList()->Add(graph);
			}
		}
	}
	getConstantEnergyRatioPlots(stationIds);
	getConstantAngleRatioPlots(stationIds);

}

