#pragma once
#ifndef PROFILEREADER_H
#define PROFILEREADER_H

#include <vector>
#include <string>
#include "Planes.h"

#include <acfutils/assert.h>
#include <acfutils/avl.h>
#include <acfutils/airportdb.h>

static const airport_t* closest_airport;


// provide the filename, returns the contents as a string
std::string read_file_to_string(std::string);

// pass drive up and drive off vectors(passed by reference), route index and vehicle type. Modifies the routes.
void get_route_info(std::vector<std::vector<double>>&, std::vector <std::vector<double>>&, int, std::string);
std::string get_stands_for_route(int, std::string);

// enter position, returns the name of the closest stand. Additionally an int if the index of the nearest stand is needed
std::string find_closest_stand(double, double, int&);


int get_amount_of_vehicle(std::string);

// read the apt dat file and save the stand positions and headings. Also possible to only read the first stand for scenery positions only.

std::string get_airport_data(airportdb_t &); // populates the stand data vectors, returns the ICAO of closest airport.

// stand name, doubles to be changed to X, Z, heading
void get_stand_data(std::string, double&, double&, double&); // for route
std::vector<std::string> get_stand_name_list(); // name list duh



std::string get_acf_profile_dir();
void read_aircraft_config(std::vector<Vehicle_for_acf>&);



std::vector<std::string> get_icaos();
std::vector<std::string> get_scenery_names();


// get the values

// Look through custom sceneries, airports which have a profile, get their icao and add to list

std::string get_icao(std::string); // given a apt_dat, return the ICAO 

void get_supported_airports();
bool is_supported(std::string icao = ""); // check wheter the ICAO can be found on the supported scenery list

std::string get_routes_txt_dir(std::string);
std::string get_stands_txt_dir(std::string);
std::string get_pax_txt_dir(std::string);


std::string get_current_scenery_livery_dir(std::string);
std::string get_current_scenery_obj_dir(std::string);





std::string get_livery_for_vehicle(int, std::string);

std::vector<std::vector<double>> get_stand_route_data(std::string, bool is_drive_off = 0); // stand name, is drive off
std::vector<std::vector<double>> get_stand_waiting_positions(std::string);
std::vector<std::vector<double>> get_pax_route_data(std::string); // custom pax routes
bool is_walkable_stand(std::string);

bool is_stand_waiting_pos_avail(int);
void reserve_waiting_pos(int);
void free_waiting_pos(int);



/// TODO: get_supported_icaos  to get icaos (or some other ids) of airports that have a sgs profile. Only needed to be done once on startup, then 
//	can be compared to nearest ifr airport if there is a profile or not.
/// TODO: get_icao / or other type of airport id
#endif