------------------------------------------------------------
ABOUT THE PROGRAM
------------------------------------------------------------
To compile:
	make

DRS_Plots provides several different plotting functions to the data including, raw data, 2D histogram of fit slopes, candle plots by core distance, and average mip/vem by core distance. The candle plots and average mip/vem allows us to also filter by which stations we want to see.


ABOUT THE DATA
--------
CONTENTS
--------

The files contained within the subdirectories of this directory contain
processed simulation data produced using the 4 meter squared ASCII
scintillator detector simulation application within the black-tank branch of
Offline. These simulations were produced in the later portion of 2014. The
directory structure is given by:

lgenergy??.??/theta??/

where "lgenergy" stands for log10(energy/eV) and theta stands for the zenith
angle of the shower. Within each directory, filenames follow the format

event????????_en??.??_th??.dat

where the numbers following "event" are a unique event ID for the shower whose
data is contained in the file. Following "en" is the log10(energy/eV) and
following "th" is the shower zenith angle. This is repetitive information as
the directory structure already provides this information.

For each shower, 3 dense rings of 12 stations each were simulated. The
distances from the shower axis (within the shower detector plane, SDP) were
600, 800, and 1000 meters. The dense stations were spaced evenly in the
azimuthal angle within the SDP.


--------
WARNINGS
--------

The ASCII scintillator simulations were largely hacked together within the
black-tank branch (which was full of hack jobs). As a result, some known bugs
are present in these simulations. There may very well be other bugs, which
have not been discovered. Known bugs:

1. There is an issue in the simulated baseline. As a result, a number of quite
negative signals are present in the data set. This bug does not seem to
heavily affect signals above a few VEM/MIP.

2. There are systematics between signals calculated from the high gain and low
gain channels. In principle, there is a region in which both gains are
expected to perform rather well, but even within real data, systematics are
present. As a result, analysis on dense rings in which some station signals
were calculated using the low gain channel and some using the high gain
channel will yield biased results. Exercise caution.


------
FORMAT
------

Data files for individual events (event????????_en??.??_th??.dat) include
column headers. Here is the legend:

station_id        ID of the simulated station
r_mc              Distance from the shower axis in the SDP
scin_tot          Total scintillator signal
scin_em           Scintillator signal deriving from electromagnetic component
scin_mu           Scintillator signal deriving from muonic component
wcd_tot           Total WCD signal
wcd_em            WCD signal deriving from electromagnetic component
wcd_mu            WCD signal deriving from muonic component
scin_sat_status   0 (high gain channel)
                  1 (low gain channel)
                  2 (saturated low gain channel)
scin_sat_status   0 (high gain channel)
                  1 (low gain channel)
                  2 (saturated low gain channel)