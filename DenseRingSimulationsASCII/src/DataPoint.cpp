#include "DataPoint.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// PUBLIC METHODS /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

//Constructor
DataPoint::DataPoint(int station_id, double core_distance, double energy, double angle, double wcd_tot, double scint_tot, double corrected_scint_tot)
{
	this->station_id = station_id;
	this->core_distance = core_distance;
	this->energy = energy;
	this->angle = angle;
	this->wcd_tot = wcd_tot;
	this->scint_tot = scint_tot;
	this->corrected_scint_tot = corrected_scint_tot;
}

/*
Reads in a file that has already been opened
params
	ifstream input_file file that has been opened to be read
	string file name of file being read
*/
vector<DataPoint> DataPoint::ReadFile(ifstream& input_file, string file)
{
	vector<DataPoint> data;
	int station_id;
	int scin_sat_status, wcd_sat_status;
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

	//If you want to see it print out hundreds of lines, uncomment the cout
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
		DataPoint newPoint(station_id, r_mc, energy, theta, wcd_tot, scint_tot, doCorrectionOne(scint_tot, theta, r_mc, 20000));
		data.push_back(newPoint);
	}
	return data;
}

/*
NOTE: NOT SURE IF WE NEED THIS ANYMORE SINCE WE KNOW THE GEOMETRY AND STATIONS 00 AND 06 SHOULD BE MAX AND MIN
Takes a vector of data from a single file and gets the 2 points per core distance 
that have the min and max scint_tot value
*/
vector<DataPoint> DataPoint::GetMinAndMaxData(vector<DataPoint>& data)
{
	DataPoint max600 (0,0,0,0,0,INT_MIN,0);
	DataPoint min600 (0,0,0,0,0,INT_MAX,0);
	DataPoint max800 (0,0,0,0,0,INT_MIN,0);
	DataPoint min800 (0,0,0,0,0,INT_MAX,0);
	DataPoint max1000 (0,0,0,0,0,INT_MIN,0);
	DataPoint min1000 (0,0,0,0,0,INT_MAX,0);
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
Filters data by angle and/or energy.
params
	vector<DataPoint> data : the data to filter
	double angle : the angle to filter by
	double energy : the energy to filter by
*/
vector<DataPoint> DataPoint::filterData(vector<DataPoint>& data, double angle, double energy)
{
	vector<DataPoint> filteredData;
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

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// PRIVATE METHODS ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
Applies a correction to a scint_tot value, based on geomtry of incoming shower
Used for max scint_tot
*/
double DataPoint::doCorrectionOne(double scint_tot, double angle, double coreDistance, double height)
{
	if(angle == 0) return scint_tot;
	double phi = atan(height/((height/tan(angle * TMath::Pi()/180)) - coreDistance/sin(angle * TMath::Pi()/180)));
	return scint_tot/abs(cos(phi));
}

/*
Applies a correction to a scint_tot value, based on geomtry of incoming shower
Used for min scint_tot
*/
double DataPoint::doCorrectionTwo(double scint_tot, double angle, double coreDistance, double height)
{
	if(angle == 0) return scint_tot;
	double phi = atan(height/((height/tan(angle * TMath::Pi()/180)) + coreDistance/sin(angle * TMath::Pi()/180)));
	return scint_tot/abs(cos(phi));
}