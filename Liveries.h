#pragma once
#ifndef LIVERIES_H
#define LIVERIES_H

#include <vector>
#include <string>


void set_objects_for_each_livery(std::string, bool is_airport_spesific = 0);

std::vector<std::string> get_global_liveries(std::string);
std::vector<std::string> get_airport_spesific_liveries(std::string);



#endif