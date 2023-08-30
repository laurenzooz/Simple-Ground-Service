#pragma once
#ifndef CUSTOM_STAIRS_H
#define CUSRTOM_STAIRS_H

#include "People.h"

class Custom_stairs
{

protected:

	bool status = 0; // 0 not prepared, 1 = prepared. Later add a possibility to trigger this with a dataref.
	bool is_global = 1;



	float pos_x = 0, pos_z = 0, hdg = 0;

	// this from the acf profile
	double step_height = 0.218;
	double step_length = 0.23;
	int step_amount = 13;

	std::string desired_stand = "";

	


public:
	void initialize(std::vector<double>); // constructor and initializer basically

	void menu_callback();
	void unload(); // status back to 0 etc. (Kinda uninitalize, do when destroying the other objects)

	bool get_status();
	std::string get_desired_stand();
	void set_desired_stand(std::string);
	std::vector<Human> Pax;


	void global_mode_select(bool); // true to switch to global mode, false to switch to local mode.

	
	//void pax_status(double, double, double, bool, std::vector<Human*>); // just a checker that can be called in floop from main. Also the bus pos and status for the people. ALso the icao for now.(hope i change it)
	bool pax_status(double, std::vector<double>, std::vector<double>, std::vector<bool>, std::vector<Human*>); // just a checker that can be called in floop from main. Also the bus pos and status for the people. ALso the icao for now.(hope i change it)
};



bool is_custom_stair(); // is there custom stair in the acf profile

// if found, this reads the data and sets it to the provided arguments

std::vector<double> get_custom_stair_data(std::string&);
// [0] = x, [1] = z, [2] = hdg, [3] = step length, [4] = step height, [5] = step amount

#endif