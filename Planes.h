#pragma once
#ifndef PLANES_H
#define PLANES_H




class Vehicle_for_acf
{
	// count for all types 


	std::string type;
	std::string name;
	int assigned_vehicle_index;
	int index_of_this_type;


	// temp solutions for the placing of em in da waiting spot

	double x, z, hdg, height;

public:
	Vehicle_for_acf(std::string, std::string, double = 0, double = 0, double = 0, double = 0);
	Vehicle_for_acf();

	std::string gettype();
	std::string getname();

	void set_assigned_vehicle_index(int);
	int get_assigned_vehicle_index();
	int get_index_of_this_type();
	


	double get_x();
	double get_z();
	double get_hdg();
	double get_height();



};


//std::vector<Vehicle_for_acf> acf_profile_vehicles;


// plane positions




bool is_behind_wing(std::string, std::string);


// calculate a point relative from some known point. newX, newZ, posX, posZ, posHdg, offset from the distance (for stuff). And profile index number of the object
void calculate_relative_point(std::string, std::string, double&, double&, double, double, double, double = 0, bool is_for_prepare = 0);


// calculates a route for the bus, around a known point (the plane) (posX, posZ, posHdg)
std::vector<std::vector<double>> calculate_bus_route(double, double, double);

int get_amount_of_type_for_acf(std::string);


#endif

