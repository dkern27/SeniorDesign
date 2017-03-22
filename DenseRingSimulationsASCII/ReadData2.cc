// Header files to include

// C++
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <string>
#include <sstream> // string manipulation
#include <vector>
#include <climits>
#include <map>

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
	double core_distance;
	double energy;
	double angle;
	double wcd_tot;
	double scint_tot;
	double corrected_scint_tot;
};

//Function Declarations
void usage();
vector<DataPoint> ReadFile(ifstream& input_file, string file);
vector<DataPoint> IDontKnowWhatToCallThis(vector<DataPoint>& data); //Hanna help
TH2F* makeHistogram(vector<DataPoint>& data, vector<double> angles, vector<double> energies, bool corrected);
vector<double> getFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);
vector<double> getCorrectedFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);
double doCorrectionOne(double scint_tot, double angle, double coreDistance, double height);
double doCorrectionTwo(double scint_tot, double angle, double coreDistance, double height);
TGraph* plotData(vector<DataPoint>& data, double angle = -1, double energy = -1, bool doCorrected = false);
vector<DataPoint> filterData(vector<DataPoint>& data, double angle, double energy);
TGraph* getSlopesForCoreDistance(vector<DataPoint>& data, double angle, double energy);
TH2F* getSlopesForCoreDistanceCandlePlot(vector<DataPoint>& data, double angle, double energy);


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

  // -------------------
	vector<DataPoint> data;
	vector<DataPoint> data2;
	
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

		vector<DataPoint> newData2 = IDontKnowWhatToCallThis(newData);

		data.insert(data.end(), newData.begin(), newData.end());
		data2.insert(data2.end(), newData2.begin(), newData2.end());
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

string options;
cin >> options;

for (char& c : options) {
  int n = c - '0';
  if (n > 4 || n < 1)
    cout << c << " is not a valid option, skipping" << endl;
  else {
    if (n == 1){
        // PLOT POINTS
        TCanvas* c4 = new TCanvas();
        TGraph* g1 = plotData(data);
        g1->Draw("AP");
        c4->Update();
    } else if (n == 2){
        // FIT SLOPES 2D HISTOGRAM
        TCanvas* c1 = new TCanvas();
        TH2F* fit_slopes = makeHistogram(data, angles, energies, false);
        fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
        fit_slopes->Draw("colz");
        c1->Update();
        
        TCanvas* c2 = new TCanvas();
        TH2F* corrected_fit_slopes = makeHistogram(data, angles, energies, true);
        corrected_fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
        corrected_fit_slopes->Draw("colz");
        c2->Update();

        TCanvas* c3 = new TCanvas();
        TH2F* corrected_fit_slopes2 = makeHistogram(data2, angles, energies, true);
        corrected_fit_slopes2->GetZaxis()->SetRangeUser(0.75, 1.6);
        corrected_fit_slopes2->Draw("colz");
        c3->Update();
    } else if (n == 3) {
        // FOR PLOTTING POINTS OF SLOPES AT CORE DIST
        for (double energy : energies){
          for (double angle : angles){
              string title = "Energy: " + to_string(energy) + " Angle: " + to_string(angle);
              TCanvas* c = new TCanvas();
              TGraph* g = getSlopesForCoreDistance(data, angle, energy);
              g->GetXaxis()->SetTitle("Core Distance");
              g->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
              g->SetMarkerSize(.75);
              g->SetMarkerStyle(20);
              g->SetTitle(title.c_str());
              g->Draw("AP");
              c->Update();
          }
        }
      } else if (n == 4) {
       // FOR PLOTTING CANDLE PLOTS OF SLOPES AT CORE DIST
        for (double energy : energies){
          for (double angle : angles){
              string title = "Energy: " + to_string(energy) + " Angle: " + to_string(angle);
              TCanvas* c = new TCanvas();
              TH2F* g = getSlopesForCoreDistanceCandlePlot(data, angle, energy);
              g->GetXaxis()->SetTitle("Core Distance");
              g->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
              // g->SetBarWidth(0.4);
              // g->SetBarOffset(-0.25);
              // g->SetFillStyle(1001);
              g->SetTitle(title.c_str());
              g->Draw("candle2");
              c->Update();
          }
        }
      }
    }

  }


 //  // FOR PLOTTING POINTS OF SLOPES AT CORE DIST
 //  for (double energy : energies){
 //    for (double angle : angles){
 //        string title = "Energy: " + to_string(energy) + " Angle: " + to_string(angle);
 //        TCanvas* c = new TCanvas();
 //        TGraph* g = getSlopesForCoreDistance(data, angle, energy);
 //        g->GetXaxis()->SetTitle("Core Distance");
 //        g->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
 //        g->SetMarkerSize(.75);
 //        g->SetMarkerStyle(20);
 //        g->SetTitle(title.c_str());
 //        g->Draw("AP");
 //        c->Update();
 //    }
 //  }

 //  // FOR PLOTTING CANDLE PLOTS OF SLOPES AT CORE DIST
 //  for (double energy : energies){
 //    for (double angle : angles){
 //        string title = "Energy: " + to_string(energy) + " Angle: " + to_string(angle);
 //        TCanvas* c = new TCanvas();
 //        TH2F* g = getSlopesForCoreDistanceCandlePlot(data, angle, energy);
 //        g->GetXaxis()->SetTitle("Core Distance");
 //        g->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");
 //        // g->SetBarWidth(0.4);
 //        // g->SetBarOffset(-0.25);
 //        // g->SetFillStyle(1001);
 //        g->SetTitle(title.c_str());
 //        g->Draw("candle2");
 //        c->Update();
 //    }
 //  }

 //  // FIT SLOPES 2D HISTOGRAM
	// TCanvas* c1 = new TCanvas();
	// TH2F* fit_slopes = makeHistogram(data, angles, energies, false);
	// fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
	// fit_slopes->Draw("colz");
	// c1->Update();
	
	// TCanvas* c2 = new TCanvas();
	// TH2F* corrected_fit_slopes = makeHistogram(data, angles, energies, true);
	// corrected_fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
	// corrected_fit_slopes->Draw("colz");
	// c2->Update();

	// TCanvas* c3 = new TCanvas();
	// TH2F* corrected_fit_slopes2 = makeHistogram(data2, angles, energies, true);
	// corrected_fit_slopes2->GetZaxis()->SetRangeUser(0.75, 1.6);
	// corrected_fit_slopes2->Draw("colz");
	// c3->Update();


 //  // PLOT POINTS
	// TCanvas* c4 = new TCanvas();
	// TGraph* g1 = plotData(data);
	// g1->Draw("AP");
	// c4->Update();

	theApp.Run();

	return 0;

}

//////////////////////////////////////////// END OF MAIN ////////////////////////////////////////////

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
		DataPoint newPoint = {r_mc, energy, theta, wcd_tot, scint_tot, doCorrectionOne(scint_tot, theta, r_mc, 20000)};
		data.push_back(newPoint);
	}
	return data;
}

//I hate this function so much
vector<DataPoint> IDontKnowWhatToCallThis(vector<DataPoint>& data)
{
	DataPoint max600 = {0,0,0,0,INT_MIN,0};
	DataPoint min600 = {0,0,0,0,INT_MAX,0};
	DataPoint max800 = {0,0,0,0,INT_MIN,0};
	DataPoint min800 = {0,0,0,0,INT_MAX,0};
	DataPoint max1000 = {0,0,0,0,INT_MIN,0};
	DataPoint min1000 = {0,0,0,0,INT_MAX,0};
	for(DataPoint d:data)
	{
		if(d.core_distance == 600)
		{
			if(d.scint_tot < min600.scint_tot)
			{
				min600 = d;
			}
			else if(d.scint_tot > max600.scint_tot)
			{
				max600 = d;
			}
		}
		else if(d.core_distance == 800)
		{
			if(d.scint_tot < min800.scint_tot)
			{
				min800 = d;
			}
			else if(d.scint_tot > max800.scint_tot)
			{
				max800 = d;
			}
		}
		else if(d.core_distance == 1000)
		{
			if(d.scint_tot < min1000.scint_tot)
			{
				min1000 = d;
			}
			else if(d.scint_tot > max1000.scint_tot)
			{
				max1000 = d;
			}
		}
	}
	min600.corrected_scint_tot = doCorrectionTwo(min600.scint_tot, min600.angle, min600.core_distance, 20000);
	max600.corrected_scint_tot = doCorrectionOne(max600.scint_tot, max600.angle, max600.core_distance, 20000);
	min800.corrected_scint_tot = doCorrectionTwo(min800.scint_tot, min800.angle, min800.core_distance, 20000);
	max800.corrected_scint_tot = doCorrectionOne(max800.scint_tot, max800.angle, max800.core_distance, 20000);
	min1000.corrected_scint_tot = doCorrectionTwo(min1000.scint_tot, min1000.angle, min1000.core_distance, 20000);
	max1000.corrected_scint_tot = doCorrectionOne(max1000.scint_tot, max1000.angle, max1000.core_distance, 20000);
	vector<DataPoint> otherData;
	otherData.push_back(min600);
	otherData.push_back(max600);
	otherData.push_back(min800);
	otherData.push_back(max800);
	otherData.push_back(min1000);
	otherData.push_back(max1000);
	return otherData;
}

/*
Makes a single histogram of fit slopes
params
	vector<DataPoint>& data All data to be filtered through
	vector<double> angles : the angles to filter by
	vector<double> energies : the energies to filter by
	bool corrected : whether to use the corrected scint_tot or regular
*/
TH2F* makeHistogram(vector<DataPoint>& data, vector<double> angles, vector<double> energies, bool corrected)
{
	//args: name, title, num_bins_x, min_x, max_x, num_bins_y, min_y, max_y
	TH2F *fit_slopes = new TH2F("slopes", "Slopes", energies.size(), 0, energies.size(), angles.size(), 0, angles.size());
	for (int i = 0; i < angles.size(); i++)
	{
		vector<double> slopes;
		if(!corrected)
			slopes = getFitSlopes(data, angles[i], energies);
		else
			slopes = getCorrectedFitSlopes(data, angles[i], energies);

		for (int j = 0; j < slopes.size(); j++)
		{
			cout << slopes[j] << endl;
			fit_slopes->Fill(j, i, slopes[j]);
		}
	}
	return fit_slopes;
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

/*
Plots and fits data filtered by angle and energy using the corrected scint_tot
params
	vector<DataPoint>& data All data to be filtered through
	double angle the angle to filter by
	vector<double> energies the energies to filter by
*/
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

/*
Makes a single plot of the data wcd_tot vs scint_tot
params
	vector<DataPoint> data : the data to filter
	double angle : the angle to filter by. Default is -1 if do not need to filter
	double energy : the energy to filter by. Default is -1 if do not need to filter
*/
TGraph* plotData(vector<DataPoint>& data, double angle, double energy, bool doCorrected)
{
	TGraph* graph = new TGraph();
	graph->SetMarkerSize(1);
	graph->SetMarkerStyle(20);
	graph->SetMarkerColor(kRed);
	vector<DataPoint> dataToPlot = filterData(data, angle, energy);
	int index = 0;
	for (DataPoint d : dataToPlot)
	{
		if (!doCorrected)
		{
			graph->SetPoint(index, d.wcd_tot, d.scint_tot);
		}
		else
		{
			graph->SetPoint(index, d.wcd_tot, d.corrected_scint_tot);
		}
		index++;
	}
	graph->GetXaxis()->SetTitle("WCD [VEM]");
	graph->GetYaxis()->SetTitle("SSD [MIP]");
	return graph;
}

/*
Filters data by angle and/or energy. Used by plotData
params
	vector<DataPoint> data : the data to filter
	double angle : the angle to filter by
	double energy : the energy to filter by
*/
vector<DataPoint> filterData(vector<DataPoint>& data, double angle, double energy)
{
	vector<DataPoint> filteredData;
	int index = 0;
	for (DataPoint d : data)
	{
		if(angle == -1 && energy == -1)
		{
			filteredData.push_back(d);
		}
		else if (angle == -1)
		{
			if (d.energy == energy)
			{
				filteredData.push_back(d);
			}
		}
		else if (energy == -1)
		{
			if (d.angle == angle)
			{
				filteredData.push_back(d);
			}
		}
		else if (d.angle == angle && d.energy == energy)
		{
			filteredData.push_back(d);
		}
	}
	return filteredData;
}



/*
Creates a graph showing mip/vem ratio for the various core distances
params
  vector<DataPoint>& data All data to be filtered through
  double angle the angle to filter by
  double energy the energy to filter by
*/
TGraph* getSlopesForCoreDistance(vector<DataPoint>& data, double angle, double energy)
{
  TGraph* graph = new TGraph();
  TGraph* averageGraph = new TGraph();
  
  map<double, vector<double>> points;

  int index = 0;

  //Filters into several graphs
  for (DataPoint d : data)
  {
    if(d.energy == energy && d.angle == angle){
          graph->SetPoint(index, d.core_distance, (d.scint_tot/d.wcd_tot));
          points[d.core_distance].push_back(d.scint_tot/d.wcd_tot);
          index++;
    }
  }

  // index = 0;
  // for (auto const& entry : points) {
  //   cout << entry.first << endl;
  //   double sum = 0;

  //   for(point : entry.second){
  //     sum += point;
  //   }

  //   averageGraph->SetPoint(index, entry.first, sum/entry.second.size());

  //   index++;
  // }

  return graph;
}

TH2F* getSlopesForCoreDistanceCandlePlot(vector<DataPoint>& data, double angle, double energy)
{
  int nx = 10;
  int ny = 1000;
  int minx = 500;
  int maxx = 1100;
  double miny = 0.;
  double maxy = 2.2;


  TH2F* graph = new TH2F("nameinmemory", "title", nx, minx, maxx, ny, miny, maxy);

  //Filters into several graphs
  for (DataPoint d : data)
  {
    if(d.energy == energy && d.angle == angle){
          graph->Fill(d.core_distance, (d.scint_tot/d.wcd_tot));       
    }
    
  }


  // index = 0;
  // for (auto const& entry : points) {
  //   cout << entry.first << endl;
  //   double sum = 0;

  //   for(point : entry.second){
  //     sum += point;
  //   }

  //   averageGraph->SetPoint(index, entry.first, sum/entry.second.size());

  //   index++;
  // }

  return graph;
}