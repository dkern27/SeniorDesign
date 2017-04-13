#include "Plotter.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// PUBLIC METHODS /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
Makes a single plot of the data wcd_tot vs scint_tot
params
	vector<DataPoint> data : the data to filter
	double angle : the angle to filter by. Default is -1 if do not need to filter
	double energy : the energy to filter by. Default is -1 if do not need to filter
*/
TGraph* Plotter::plotData(vector<DataPoint>& data, double angle, double energy, bool doCorrected)
{
	TGraph* graph = new TGraph();
	graph->SetMarkerSize(1);
	graph->SetMarkerStyle(20);
	graph->SetMarkerColor(kRed);
	vector<DataPoint> dataToPlot = DataPoint::filterData(data, angle, energy);
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
Makes a single histogram of fit slopes
params
	vector<DataPoint>& data All data to be filtered through
	vector<double> angles : the angles to filter by
	vector<double> energies : the energies to filter by
	bool corrected : whether to use the corrected scint_tot or regular
*/
TH2F* Plotter::make2DHistogram(vector<DataPoint>& data, vector<double> angles, vector<double> energies, bool corrected)
{
	//TH2F args: name in memory, title, num_bins_x, min_x, max_x, num_bins_y, min_y, max_y
	TH2F *fit_slopes = new TH2F("slopes", "Fit Slopes", energies.size(), 0, energies.size(), angles.size(), 0, angles.size());
	for (int i = 0; i < angles.size(); i++)
	{
		vector<double> slopes;
		if(!corrected)
			slopes = getFitSlopes(data, angles[i], energies);
		else
			slopes = getCorrectedFitSlopes(data, angles[i], energies);

		for (int j = 0; j < slopes.size(); j++)
		{
			//cout << slopes[j] << endl;
			fit_slopes->Fill(j, i, slopes[j]);
		}
	}
	//This has been set from looking at the graphs and choosing a range to use most colors
	fit_slopes->GetZaxis()->SetRangeUser(0.75, 1.6);
	return fit_slopes;
}

/*
Creates a graph showing mip/vem ratio for the various core distances
params
  vector<DataPoint>& data All data to be filtered through
  double angle the angle to filter by
  double energy the energy to filter by
*/
TGraph* Plotter::getSlopesForCoreDistance(vector<DataPoint>& data, double angle, double energy)
{
	TGraph* graph = new TGraph();

	int index = 0;
	for (DataPoint d : data)
	{
		if(d.energy == energy && d.angle == angle)
		{
			graph->SetPoint(index, d.core_distance, (d.scint_tot/d.wcd_tot));
			index++;
		}
	}

	graph->GetXaxis()->SetTitle("Core Distance");
	graph->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");

	return graph;
}

/*
Creates a candle plot showing mip/vem ratio for the various core distances, can filter by sation id
paramss
  vector<DataPoint>& data All data to be filtered through
  double angle the angle to filter by
  double energy the energy to filter by
  set<int> stationIds station Ids to plot
*/
TH2F* Plotter::getSlopeVsDistanceCandlePlot(vector<DataPoint>& data, double angle, double energy, set<string> stationIds)
{
	int nx = 10;
	int ny = 1000;
	int minx = 500;
	int maxx = 1100;
	double miny = 0.;
	double maxy = 2.2;


	string title = "Fit Slopes vs. Distance | Energy: " + to_string(energy) + " Angle: " + to_string(angle);

	TH2F* graph = new TH2F(title.c_str(), title.c_str(), nx, minx, maxx, ny, miny, maxy);

	vector<DataPoint> filteredData = DataPoint::filterData(data, angle, energy);

  //Filters into several graphs
	for (DataPoint d : data)
	{
		string id = to_string(d.station_id);
		//empty station ids is plotting all stations
		//otherwise look at last two digits of station id
		if(stationIds.empty() || stationIds.find(id.substr(id.length()-2)) != stationIds.end())
		{
			graph->Fill(d.core_distance, (d.scint_tot/d.wcd_tot));       
		}
	}

	graph->GetXaxis()->SetTitle("Core Distance");
	graph->GetYaxis()->SetTitle("SSD [MIP] / WCD [VEM]");

	return graph;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// PRIVATE METHODS ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
Plots and fits data filtered by angle and energy
params
	vector<DataPoint>& data All data to be filtered through
	double angle the angle to filter by
	vector<double> energies the energies to filter by
*/
vector<double> Plotter::getFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies)
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
vector<double> Plotter::getCorrectedFitSlopes(vector<DataPoint>& data, double angle, vector<double> energies)
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