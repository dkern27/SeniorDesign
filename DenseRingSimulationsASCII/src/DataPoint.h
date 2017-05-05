#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <climits>
#include <iostream>

#include "TMath.h"

#pragma once

using namespace std;

class DataPoint
{
	public:
		//Constructor
		DataPoint(int station_id, double core_distance, double energy, double angle, double wcd_tot, double scint_tot, double corrected_scint_tot);

		//Static function to read in data and filter it
		static vector<DataPoint> ReadFile(ifstream& input_file, string file);
		static vector<DataPoint> GetMinAndMaxData(vector<DataPoint>& data);
		static vector<DataPoint> filterData(vector<DataPoint>& data, double angle, double energy);

		//Attributes
		int station_id;
		double core_distance;
		double energy;
		double angle;
		double wcd_tot;
		double scint_tot;
		double corrected_scint_tot;

	private:
		static double doCorrectionOne(double scint_tot, double angle, double coreDistance, double height);
		static double doCorrectionTwo(double scint_tot, double angle, double coreDistance, double height);
};