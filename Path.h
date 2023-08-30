#pragma once
#ifndef PATH_H
#define PATH_H

#include <vector>
// heading between two points. x, z (in local), destination x, z
double calculate_heading(double, double, double, double);

// distance between two points. obj position x, z (in local), destination x, z
double calculate_distance(double, double, double, double);


// opengl x, z; then double to be set to world lat, lon
void local_coordinates_to_world(double, double, double&, double&);


// world lat, lon, then doubles to be set to opengl x and z
void world_coordinates_to_local(double, double, double&, double&);

// waypoint list, target point (local)
int find_shortest_distance_index(std::vector<std::vector<double>>, double, double, bool is_log = 0);

std::vector<std::vector<double>> interpolate_midpoints(std::vector<std::vector<double>>, double);

double measure_path_distance(std::vector<std::vector<double>>);

std::vector<std::vector<double>> global_mode_route(std::string, std::string);


#endif