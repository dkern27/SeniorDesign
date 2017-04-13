#include "DataPoint.h"

#include "TROOT.h"
#include "TF1.h"
#include "TH1F.h"   // 1D Histogram
#include "TH2F.h"   // 2D Histogram
#include "TGraph.h" // Basic scatter plot

#include <vector>
#include <set>

#pragma once

using namespace std;

//Could give this vector of data to hold and make functions not static
//Would have to differentiate between data and the min/max data we filter to get
//Possible solution is making data the default argument, and only passing in data if we need the min/max or other data plotted

class Plotter
{
	public:
		//Plotter();
		static TGraph* plotData(vector<DataPoint>& data, double angle = -1, double energy = -1, bool doCorrected = false);
		static TH2F* make2DHistogram(vector<DataPoint>& data, vector<double> angles, vector<double> energies, bool corrected);
		static TGraph* getSlopesForCoreDistance(vector<DataPoint>& data, double angle, double energy);

		static TH2F* getSlopeVsDistanceCandlePlot(vector<DataPoint>& data, double angle, double energy, set<string> stationIds);

		//To make non static
		//void addToData(DataPoint d);
		//vector<DataPoint> getData();
		//void addToCorrectedData(DataPoint d);
		//vector<DataPoint> getCorrectedData();

	private:
		static vector<double> getFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);
		static vector<double> getCorrectedFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies);

		//vector<DataPoint> data;
		//vector<DataPoint> correctedData;
};