
#define _USE_MATH_DEFINES
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"

#include <string>
#include <vector>
#include <cmath>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

#include "Planes.h"
#include "Path.h"
#include "Profile_reader.h"
#include "temp_message.h"



Vehicle_for_acf::Vehicle_for_acf(std::string type, std::string name, double x, double z, double hdg, double height)
{
	this->type = type;
	this->name = name;
	this->x = x;
	this->z = z;
	this->hdg = hdg;
	this->height = height;

	/*
	if (type == "stair_small")
	{
		index_of_this_type == stair_count;
		stair_count++;
		XPLMDebugString("SGS[DEBUG] -- Stair count increased and is now");
		print_number_to_log(stair_count);
		XPLMDebugString("\n");
	}
	else
	{
		index_of_this_type == loader_count;
		loader_count++;
		XPLMDebugString("SGS[DEBUG] -- Loader count increased and is now");
		print_number_to_log(loader_count);

		XPLMDebugString("\n");
	}
	*/
}
Vehicle_for_acf::Vehicle_for_acf()
{
	x = 0;
	z = 0;
	hdg = 0;
	height = 0;

}

std::string Vehicle_for_acf::gettype()
{
	return type;
}

std::string Vehicle_for_acf::getname()
{
	return name;
}

double Vehicle_for_acf::get_x()
{
	return x;
}

double Vehicle_for_acf::get_z()
{
	return z;
}

double Vehicle_for_acf::get_hdg()
{
	return hdg;
}

double Vehicle_for_acf::get_height()
{
	//XPLMDebugString("SGS[DEBUG] -- Getting height: ");
	//XPLMDebugString(std::to_string(height).c_str());
	//XPLMDebugString("\n");

	return height;
}


int Vehicle_for_acf::get_index_of_this_type()
{
	return index_of_this_type;
}


void Vehicle_for_acf::set_assigned_vehicle_index(int i)
{
	assigned_vehicle_index = i;
}

int Vehicle_for_acf::get_assigned_vehicle_index()
{
	return assigned_vehicle_index;
}
//int Vehicle_for_acf::stair_count = 0;
//int Vehicle_for_acf::loader_count = 0;


bool is_behind_wing(std::string name, std::string type)
{
	std::vector<Vehicle_for_acf> acf_profile_vehicles;
	read_aircraft_config(acf_profile_vehicles);

	int profile_index = -1;

	for (size_t i = 0; i < acf_profile_vehicles.size(); i++)
	{
		if (acf_profile_vehicles[i].getname() == name && acf_profile_vehicles[i].gettype() == type)
		{
			profile_index = i;
		}
	}
	if (profile_index == -1)
	{
		XPLMDebugString("SGS ERROR, unknown vehicle for relative point calculation\n");
	}
	double ac_z = acf_profile_vehicles[profile_index].get_z();


	// get the data. 

	// vittu t�� on kyl sekasotkua

	// if behind wing, z is smaller than 0.
	return ac_z < 0;
}


void calculate_relative_point(std::string name, std::string type, double& x, double& z, double pos_x, double pos_z, double posHdg, double distance, bool is_for_prepare)
{
	
	std::vector<Vehicle_for_acf> acf_profile_vehicles;
	read_aircraft_config(acf_profile_vehicles);

	int profile_index = -1;

	for (size_t i = 0; i < acf_profile_vehicles.size(); i++)
	{
		if (acf_profile_vehicles[i].getname() == name && acf_profile_vehicles[i].gettype() == type)
		{
			profile_index = i;
		}
	}
	if (profile_index == -1)
	{
		XPLMDebugString("SGS ERROR, unknown vehicle for relative point calculation\n");
		return;
	}
	
	
	// get the index of this certain vehicle type.
	// int index_of_this_type = acf_profile_vehicles[profile_index].get_index_of_this_type();
	// XPLMDebugString("\n\nIndex of this type: ");
	// print_number_to_log(index_of_this_type);

	double ac_x = acf_profile_vehicles[profile_index].get_x();
	double ac_z = acf_profile_vehicles[profile_index].get_z();
	double ac_hdg = acf_profile_vehicles[profile_index].get_hdg();

	// when preparing, different position
	if (is_for_prepare == 1)
	{
		ac_z = 15;
		ac_hdg = 180;
		// left or right side of the plane
		/*
		XPLMDebugString("SGS[DEBUG] -- index of this type =  ");
		XPLMDebugString((std::to_string(profile_index)).c_str());
		XPLMDebugString("\n");
		*/

		if (ac_x < 0)
		{
			ac_x = - 25 - (profile_index * 4);
		}
		else
		{
			ac_x =	 25 + ((profile_index - 2) * 4);

		}
		/*
		XPLMDebugString("SGS[DEBUG] -- ac_x =  ");
		XPLMDebugString((std::to_string(ac_x)).c_str());
		XPLMDebugString("\n");
		*/
	}


	double hdg_to_relative_point = calculate_heading(0, 0, ac_x, ac_z);
	double dist_to_relative_point = calculate_distance(0, 0, ac_x, ac_z);



	x = pos_x - (dist_to_relative_point * sin((posHdg - hdg_to_relative_point) * (M_PI / 180)));
	z = pos_z + (dist_to_relative_point * cos((posHdg - hdg_to_relative_point) * (M_PI / 180)));







	if (distance != 0)
	{
		x += (distance)*sin((posHdg + (ac_hdg + 180)) * (M_PI / 180));
		z -= (distance)*cos((posHdg + (ac_hdg + 180)) * (M_PI / 180));
	}


	/*
	x = plane_x + ((-30 - profile_x_offset) * sin((plane_hdg + profile_hdg_offset) * (M_PI / 180)));
	z = plane_z + (( 30 + profile_z_offset) * cos((plane_hdg + profile_hdg_offset) * (M_PI / 180)));
	*/
	// kun et�isyys nolla, ei headingilla ole v�li�. Vain preparation point

}



// calculates a route for the bus, around a known point (the plane) (posX, posZ, posHdg)
std::vector<std::vector<double>> calculate_bus_route(double x, double z, double hdg)
{
	std::vector<std::vector<double>> route;
	route.resize(2);

	// first point: right Front 
	// 2: wingtip, 3: right rear, 4: straight back, 5: left rear


	// 1:
	double pX = 35;
	double pZ = 25;

	double pHdg = calculate_heading(0, 0, pX, pZ);
	double pDist = calculate_distance(0, 0, pX, pZ);

	double tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	double tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);



	// 2:

	pX = 45;
	pZ = 0;

	pHdg = calculate_heading(0, 0, pX, pZ);
	pDist = calculate_distance(0, 0, pX, pZ);

	tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);




	// 3: 

	pX = 35;
	pZ = -35;

	pHdg = calculate_heading(0, 0, pX, pZ);
	pDist = calculate_distance(0, 0, pX, pZ);

	tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);


	// 4: 

	pX = 0;
	pZ = -40;

	pHdg = calculate_heading(0, 0, pX, pZ);
	pDist = calculate_distance(0, 0, pX, pZ);

	tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);



	// 5:

	pX = -35;
	pZ = -35;

	pHdg = calculate_heading(0, 0, pX, pZ);
	pDist = calculate_distance(0, 0, pX, pZ);

	tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);


	return route;



}


int get_amount_of_type_for_acf(std::string type)
{
	std::vector<Vehicle_for_acf> acf_profile_vehicles;
	read_aircraft_config(acf_profile_vehicles);

	int count = 0;
	for (int i = 0; i < acf_profile_vehicles.size(); i++)
	{
		if (acf_profile_vehicles[i].gettype() == "bus")
		{
			count++;
		}
	}

}