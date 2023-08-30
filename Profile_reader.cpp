#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <utility>




#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMNavigation.h"

#include "Profile_reader.h"
#include "Path.h"
#include "Planes.h"
#include "temp_message.h"

//#include "acfutils/assert.h"
//#include "acfutils/airportdb.h"

#include <acfutils/assert.h>
#include <acfutils/avl.h>
#include <acfutils/airportdb.h>



std::vector<std::vector<double>> stand_positions; // 0: X, 1: Z, 2: Hdg

std::vector<std::vector<double>> stand_waiting_positions; // 0: X, 1: Z, 2: Hdg
std::vector<bool> stand_waiting_position_availability; // 0: X, 1: Z, 2: Hdg

std::vector<std::string> stand_names;
std::string selected_airport = "KSEA Demo Area";



std::string read_file_to_string(std::string filename)
{
	/*
	XPLMDebugString("SGS[DEBUG] -- reading a file to string: ");
	XPLMDebugString(filename.c_str());
	XPLMDebugString("\n");
*/
	std::ifstream profile_file(filename);


	// TODO: case sensitivty to linux

	/*
	#if !defined(_WIN32)
    if (!profile_file.is_open())
    {
        char *r = alloca(strlen(filename.c_str()) + 2);
        if (casepath(filename.c_str(), r))
        {
            std::ifstream profile_file(r);

        }
    }
	#endif
*/

	std::string file_input = "";
	std::string row;
	if (profile_file.is_open())
	{
		while (std::getline(profile_file, row))
		{
			file_input += row;
			file_input.push_back('\n');
		}
	}
	else
	{
		XPLMDebugString("SGS[DEBUG] -- Oh noes, the file was not found\n");
	}
	

	return file_input;
}



std::string get_stands_for_route(int index, std::string vehicle_type)
{
	std::string whole_file = read_file_to_string(get_routes_txt_dir(selected_airport)); // very inefficent to read the same file again and again
	std::string stands = "";

	std::string vehicle_to_find = "#";
	vehicle_to_find += vehicle_type;
	vehicle_to_find += "_";
	vehicle_to_find += std::to_string(index);

	std::string::size_type route_start = whole_file.find(vehicle_to_find); // -1 if not found


	if (route_start != std::string::npos)
	{
		do
		{
			route_start++; //
		} while (whole_file[route_start] != '|'); // first until where the stand info starts

		do
		{
			route_start++;
			stands += whole_file[route_start];

		} while (whole_file[route_start] != '\n' && whole_file[route_start] != '|'); // Then save the stand info and iterate until row ends, thats where the coordinates start
	}
	return stands;

}

std::string get_livery_for_vehicle(int index, std::string vehicle_type)
{
	//XPLMDebugString("SGS[DEBUG] -- Looking for custom liveries\n");
	std::string whole_file = read_file_to_string(get_routes_txt_dir(selected_airport)); // very inefficent to read the same file again and again
	std::string livery = "";

	std::string vehicle_to_find = "#";
	vehicle_to_find += vehicle_type;
	vehicle_to_find += "_";
	vehicle_to_find += std::to_string(index);

	std::string::size_type livery_start = whole_file.find(vehicle_to_find); // -1 if not found


	//std::string correct_profile_input = whole_file;
	if (livery_start != std::string::npos)
	{
		do
		{
			livery_start++; //
		} while (whole_file[livery_start] != '|'); // stand info starts
		livery_start++;

		for (livery_start; livery_start < whole_file.size(); livery_start++)
		{
			if (whole_file[livery_start] == '\n')
			{
				return "default"; // means that no special livery used
			}

			if (whole_file[livery_start] == '|') // livery actually starts
			{

				for (int i = livery_start + 1; i < whole_file.size(); i++)
				{
					if (whole_file[i] == '\n')
					{
						break;
					}
					livery += whole_file[i];
				}


				break;
			}
		}


	}

	//XPLMDebugString("SGS[DEBUG] -- Custom livery found: ");
	//XPLMDebugString(livery.c_str());
	//XPLMDebugString("\n");

	return livery;

}


std::string find_closest_stand(double x, double z, int& index)
{

	

	double shortest_dist = 99999; // TODO: replace with smth sensible.
	std::string closest_stand; 
	for (int i = 0; i < stand_names.size(); i++)
	{
		double temp_dist = calculate_distance(x, z, stand_positions[0][i], stand_positions[1][i]);
		if (temp_dist < shortest_dist)
		{
			shortest_dist = temp_dist;
			closest_stand = stand_names[i];
			index = i;
		}
	}
	//XPLMDebugString("SGS[DEBUG] -- Closest stand is: ");
	//XPLMDebugString(closest_stand.c_str());
	//XPLMDebugString("\n");
	return closest_stand;
}


// inefficent reads the whole, long file multiple times. Maybe a struct for route?


void get_route_info(std::vector<std::vector<double>>& drive_up_route, std::vector <std::vector<double>>& drive_off_route, int index, std::string vehicle_type)
{
	//XPLMDebugString("SGS[DEBUG] -- Getting route info \n");
	drive_up_route.resize(2);
	drive_off_route.resize(2);
	// 0 and 1 for x and y, same stuff as before

	// hidas, koska tavallaan lukee koko filun jokaista objektia varten erikseen

// waypoints themselves

	//[0][N] on �nn�s latitude, [1][N] �nn�s longitude


	//profile_file.open("sgs.txt");

	std::string whole_file = read_file_to_string(get_routes_txt_dir(selected_airport));



	// etit��n kyseinen indexi
	std::string vehicle_to_find = "#";
	vehicle_to_find += vehicle_type;
	vehicle_to_find += "_";
	vehicle_to_find += std::to_string(index);

	int route_start = whole_file.find(vehicle_to_find); // -1 if not found


	//std::string correct_profile_input = whole_file;
	std::string correct_profile_input = "";
	if (route_start != -1)
	{
		do
		{
			route_start++;

		} while (whole_file[route_start] != '\n'); // Then save the stand info and iterate until row ends, thats where the coordinates start


		for (int i = route_start; i < whole_file.size(); i++)
		{
			if (whole_file[i] == '#') { break; }
			correct_profile_input.push_back(whole_file[i]);
		}
	}


	std::string tempstr;
	bool is_drive_off = 0;
	//XPLMDebugString("SGS[DEBUG] -- Parsing drive up and off routes\n");

	for (int i = 0; i < correct_profile_input.size(); i++)
	{


		if (correct_profile_input[i] == '\n') // rivinvaihto nollaa tempin
		{
			tempstr = "";
		}

		if (correct_profile_input[i] == '!') // return
		{
			is_drive_off = 1;
			continue; // jumps to next iteration
		}


		// lis�� objektivektoriin oikealla nimell�, lis�� obj counter.
		// ja jatka ylemp�� looppia sen j�lkeen mihin sisempi loppu.

		if (correct_profile_input[i] != '\n') // coordinates, not drive off character
		{
			while (correct_profile_input[i] != ',') // until comma for lat, after that until newline -> lon
			{
				tempstr += correct_profile_input[i];
				i++;
			}
			i++; // to next letter after comma
			tempstr.pop_back(); // erase the comma from tempstr

			if (is_drive_off == 0)
			{
				drive_up_route[0].push_back(std::stod(tempstr)); // tempstr to double that gets stored. Then get the lon
			}
			else
			{
				drive_off_route[0].push_back(std::stod(tempstr));
			}


			tempstr = "";

			while (correct_profile_input[i] != '\n') // until comma for lat, after that until newline -> lon
			{
				tempstr += correct_profile_input[i];
				i++;
			}

			if (is_drive_off == 0)
			{
				drive_up_route[1].push_back(std::stod(tempstr)); // tempstr to double that gets stored. Then get the lon
			}
			else
			{
				drive_off_route[1].push_back(std::stod(tempstr));
			}
			tempstr = "";
		}




	}



	// saves the waypoint to drive up unless a certain character ('-') is found; the waypoints after that are stored in drive off

	//XPLMDebugString("SGS[DEBUG] -- routes parsed -- converting to local from world: \n");

	// convert to local 
	for (int i = 0; i < drive_up_route[0].size(); i++)
	{
		


		double temp_x, temp_z;
		world_coordinates_to_local(drive_up_route[0][i], drive_up_route[1][i], temp_x, temp_z);
		drive_up_route[0][i] = temp_x;
		drive_up_route[1][i] = temp_z;

		


	}

	for (int i = 0; i < drive_off_route[0].size(); i++)
	{
		double temp_x, temp_z;
		world_coordinates_to_local(drive_off_route[0][i], drive_off_route[1][i], temp_x, temp_z);
		drive_off_route[0][i] = temp_x;
		drive_off_route[1][i] = temp_z;


	}

	/*
	XPLMDebugString("SGS[DEBUG] -- coordinates converted. Drive up route is: \n");
	
	for (int i = 0; i < drive_up_route[0].size(); i++)
	{
		XPLMDebugString(std::to_string(drive_up_route[0][i]).c_str());
		XPLMDebugString("\t");

		XPLMDebugString(std::to_string(drive_up_route[1][i]).c_str());
		XPLMDebugString("\n");

	}
	*/



}

std::vector <std::pair<std::string, std::string>> supported_airports; // ICAO, sceneryname



int get_amount_of_vehicle(std::string vehicle_type)
{
	int amount = 0;
	std::string whole_file = read_file_to_string(get_routes_txt_dir(selected_airport));

	size_t found_index = whole_file.rfind(vehicle_type); // returns sizeof size_t if not found
	if (found_index < whole_file.size())
	{
		std::string temp_str = "";

		for (int i = (found_index + vehicle_type.size() + 1); i < whole_file.size(); i++)
		{
			if (whole_file[i] == '\n') { break; }
			temp_str += whole_file[i];

		}
		amount = stoi(temp_str);
		amount++;


		// find the last occurance of the given type. Start saving the number from where the last occurance starts plus the length of it.
		// The length is one higher than the last index, +1 to get rid of the underscore
		// => thats where the highest number starts. Highest number plus one is the amount. 


	}




	return amount;
}


std::string get_airport_data(airportdb_t &airportdb) {

    XPLMDataRef lon_ref = XPLMFindDataRef("sim/flightmodel/position/longitude");
	XPLMDataRef lat_ref = XPLMFindDataRef("sim/flightmodel/position/latitude");


	float lon = XPLMGetDataf(lon_ref);
    float lat = XPLMGetDataf(lat_ref);

	char airport_id[50];

	closest_airport = NULL;

	unload_distant_airport_tiles(&airportdb, GEO_POS2(lat, lon)); 

	XPLMNavRef apt_ref = XPLMFindNavAid(NULL, NULL, &lat, &lon, NULL, xplm_Nav_Airport); // get the nearest airport 

	if (apt_ref != -1) {
        XPLMGetNavAidInfo(apt_ref, NULL, &lat, &lon, NULL, NULL, NULL, airport_id,
                NULL, NULL);
        
		closest_airport = adb_airport_lookup_by_ident(&airportdb, airport_id);
    }

	stand_positions.resize(3);
	stand_positions[0].resize(0);
	stand_positions[1].resize(0);
	stand_positions[2].resize(0);
	stand_names.resize(0);
	// empty the old stuff

	if (!closest_airport) {
		return "-1";
	}

	assert(closest_airport != NULL);

	// push all ramp starts to stand_positions and stand_names vectors

	ramp_start_t *ramp = (ramp_start_t*) (avl_first(&closest_airport->ramp_starts));
	for (*ramp; ramp != NULL; ramp = (ramp_start_t*) (AVL_NEXT( (&closest_airport->ramp_starts), ramp))) {

		double temp_x, temp_z;
		world_coordinates_to_local(ramp->pos.lat, ramp->pos.lon, temp_x, temp_z);
		stand_positions[0].push_back(temp_x);
		stand_positions[1].push_back(temp_z);
		// convert to local coordinates and add to vec
	

		stand_positions[2].push_back(ramp->hdgt);
		stand_names.push_back(ramp->name);
		// and name and hdg
	}


	// check if airport is supported; if yes, change the selected airport to the correct scenery name

	// there might be a method to find this, and prolly should of used a map. But eh
	for (int i = 0; i < supported_airports.size(); i++) {
		
		if (supported_airports[i].first == closest_airport->icao) {
			selected_airport = supported_airports[i].second;
			std::cout << "selected airport is now " << selected_airport << std::endl;
			break;
		}
	}

	return closest_airport->icao;
}



void get_stand_data(std::string name_to_find, double& x, double& z, double& hdg)
{
	//XPLMDebugString("SGS[DEBUG] -- Getting stand data\n");
	int index = 0;
	for (int i = 0; i < stand_names.size(); i++)
	{
		if (stand_names[i] == name_to_find)
		{
			index = i;
			break;
		}
	}
	//XPLMDebugString("SGS[DEBUG] -- completed, setting the data\n");

	x = stand_positions[0][index];
	z = stand_positions[1][index];
	hdg = stand_positions[2][index];
	//XPLMDebugString("SGS[DEBUG] -- completed\n");

}


std::vector<std::string> get_stand_name_list()
{
	return stand_names;
}



std::string get_acf_profile_dir()
{
	XPLMDataRef livery_dir = XPLMFindDataRef("sim/aircraft/view/acf_livery_path");
	//XPLMDataRef livery_dir = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");

	//std::string dir;
	char acf_file_name[1024];
	char acf_path[2048];


	XPLMGetNthAircraftModel(0, acf_file_name, acf_path);
	//strcpy(dir, (char*)& ByteVals);


	

	// remove the filename acf thingy

	std::string dir = acf_path;
	std::string::size_type start_pos = dir.rfind('/');

	if (start_pos != std::string::npos)
	{
		dir.erase(start_pos);

	}
	else
	{
		start_pos = dir.rfind('\\');
		if (start_pos != std::string::npos)
		{
			dir.erase(start_pos);
		}
	}

	dir += "/Simple Ground Service/sgs.txt";

	
	//XPLMDebugString("SGS[DEBUG] -- plane path: ");
	//XPLMDebugString(dir.c_str());
	//XPLMDebugString("\n");

	return dir;// dir;
}



void read_aircraft_config(std::vector<Vehicle_for_acf>& Acf_vehicles)
{
	//plane_counts_reset();
	// reset the counts


	std::string input = read_file_to_string(get_acf_profile_dir());

	for (int n = 0; n < input.size(); n++)
	{
		std::string temp_row = ""; // save each row individually
		for (int i = n; i < input.size(); i++)
		{
			if (input[i] == '\n')
			{
				break;
			}
			n++;

			temp_row += input[i];
		}

		if (temp_row.find("custom_stair") != std::string::npos)
		{
			//XPLMDebugString("SGS[DEBUG] -- Custom stairs found\n");

			for (int i = n; i < input.size(); i++)
			{
				

				if (input[i] == '\n')
				{
					break;
				}
				n++;
			}
			
			continue;

		}

		// save temp stuff
		std::string temp_type, temp_name;
		double temp_x = 0, temp_z = 0, temp_hdg = 0, temp_height = 0;

		std::string bfr_str = "";
		int amount_of_separators = 0; 

		for (int i = 0; i < temp_row.size(); i++)
		{

			if (temp_row[i] == '|')
			{
				amount_of_separators++;

				// each case individually
				if (amount_of_separators == 1) // type
				{

				

					temp_type = bfr_str;
					bfr_str = "";
					continue;
				}

				else if (amount_of_separators == 2) // name
				{
					temp_name = bfr_str;
					bfr_str = "";
					continue;
				}

				else if (amount_of_separators == 3) // x
				{
					temp_x = stod(bfr_str);
					bfr_str = "";
					continue;
				}

				else if (amount_of_separators == 4) // z
				{
					temp_z = stod(bfr_str);
					bfr_str = "";
					continue;

				}

				else if (amount_of_separators == 5) // hdg
				{
					temp_hdg = stod(bfr_str);
					bfr_str = "";
					continue;

				}

				// only thing remaining is the height

			}

			bfr_str += temp_row[i];
		}

		temp_height = stod(bfr_str);
		bfr_str = "";
		Acf_vehicles.push_back(Vehicle_for_acf(temp_type, temp_name, temp_x, temp_z, temp_hdg, temp_height));
		
		/*
		XPLMDebugString("SGS[DEBUG] -- vehicle for acf. type, name, x, z, hdg, height\n");
		XPLMDebugString(temp_type.c_str());
		XPLMDebugString("\t");
		XPLMDebugString(temp_name.c_str());
		XPLMDebugString("\t");
		XPLMDebugString((std::to_string(temp_x)).c_str());
		XPLMDebugString("\t");
		XPLMDebugString((std::to_string(temp_z)).c_str());
		XPLMDebugString("\t");
		XPLMDebugString((std::to_string(temp_hdg)).c_str());
		XPLMDebugString("\t");
		XPLMDebugString((std::to_string(temp_height)).c_str());
		XPLMDebugString("\n");
		*/
	}
}





std::vector<std::string> get_icaos() {
	
	std::vector<std::string> icaos;


	for (int i = 0; i < supported_airports.size(); i++) { // get the keys 
		icaos.push_back(supported_airports[i].first);
		std::cout << "ICAO: " << icaos[i] << std::endl;
	}

	return icaos;
}

std::vector<std::string> get_scenery_names() {
	
	std::vector<std::string> sceneries;


	for (int i = 0; i < supported_airports.size(); i++) { // get the keys 
		sceneries.push_back(supported_airports[i].second);
		std::cout << "Scenery name: " << sceneries[i] << std::endl;
	}

	return sceneries;
}




std::string get_icao(std::string path) {


	std::string input = read_file_to_string(path);
	std::string icao = "";



	// find the index of 'icao_code', read until end of line. Remove whitespaces.
	int i = input.find("icao_code");

	i += 10; // skip the text icao_code and the first whitespace

	if (i != std::string::npos) {
		for (i; i < input.length(); i++) {
			if (input[i] == '\n') { break; }
			if (input[i] != ' ') { icao.push_back(input[i]); }
		}
	}	
	return icao;
}





void get_supported_airports() // scans through the full custom scenery folder. slow af, crashes if non english character in any folder/file/texture name or anything. 
{
	namespace fs = std::filesystem;

	//std::string path = L"Custom Scenery/";
	std::string path = "Custom Scenery/";
	XPLMDebugString("SGS -- Scanning sceneries\n");
	for (const auto& entry : fs::recursive_directory_iterator(path))
	{
		std::string dir = entry.path().string();
		// XPLMDebugString(dir.c_str());
		//XPLMDebugString("\n");


		// if (dir.find(L"Simple Ground Service\\routes.txt") != std::wstring::npos) // backwards slashes on windows
		if (dir.find("Simple Ground Service/routes.txt") != std::string::npos)
		{


			// 'Custom Scenery/' is n letters. Search for the next / after that to know where to add the earthnavdata/apt.dat thungy

			int i = dir.find("/", 15); // this is where the name of the scenery ends, and after this add the apt.dat path

			int j = dir.find("/", i + 1); // find the next /, to get the scenery name

			std::string scenery_name =  dir.substr(15, i - 15);


			if (i == std::string::npos) { return; }

			dir = dir.substr(0, i + 1);


			// get the scenery name


			
			// TODO: recheck if wstring makes any sense, maybe a better way to deal with non-english character. 

			// find apt.dat
			dir += "Earth nav data/apt.dat";


			//std::cout << "dir to icao, scenery name:  |" << dir << "|\t|" << scenery_name << "|" << std::endl;


			supported_airports.push_back(std::make_pair(get_icao(dir), scenery_name));
		}
	}
}

bool is_supported(std::string icao) {
	
	std::vector<std::string> icaos = get_icaos();
	return (std::count(icaos.begin(), icaos.end(), icao)); // checks if icao exists within icaos vector
}



std::string get_routes_txt_dir(std::string scenery_name)
{
	std::string dir = "Custom Scenery/";
	dir += scenery_name;
	dir += "/plugins/Simple Ground Service/routes.txt";

	/*
	XPLMDebugString("SGS[DEBUG] -- Routes.txt directory: ");
	XPLMDebugString(dir.c_str());
	XPLMDebugString("\n");
	*/
	return dir;
}

std::string get_stands_txt_dir(std::string scenery_name)
{
	std::string dir = "Custom Scenery/";
	dir += scenery_name;
	dir += "/plugins/Simple Ground Service/stands.txt";

	/*
	XPLMDebugString("SGS[DEBUG] -- Routes.txt directory: ");
	XPLMDebugString(dir.c_str());
	XPLMDebugString("\n");
	*/
	return dir;
}

std::string get_pax_txt_dir(std::string scenery_name)
{
std::string dir = "Custom Scenery/";
	dir += scenery_name;
	dir += "/plugins/Simple Ground Service/pax.txt";

	/*
	XPLMDebugString("SGS[DEBUG] -- Routes.txt directory: ");
	XPLMDebugString(dir.c_str());
	XPLMDebugString("\n");
	*/
	return dir;
}




std::string get_current_scenery_livery_dir(std::string vehicle_type)
{
	std::string dir = "Custom Scenery/";
	dir += selected_airport;
	dir += "/plugins/Simple Ground Service/liveries/";
	dir += vehicle_type;
	dir += "/";

	//XPLMDebugString("SGS[DEBUG] -- custom livery directory for selected scenery: ");
	//XPLMDebugString(dir.c_str());
	//XPLMDebugString("\n");


	return dir;
}

std::string get_current_scenery_obj_dir(std::string vehicle_type)
{
	std::string dir = "Custom Scenery/";
	dir += selected_airport;
	dir += "/plugins/Simple Ground Service/objects/";
	dir += vehicle_type;
	dir += "/";

	//XPLMDebugString("SGS[DEBUG] -- custom object directory for selected scenery: ");
	//XPLMDebugString(dir.c_str());
	//XPLMDebugString("\n");

	return dir;
}


std::vector<std::vector<double>> get_stand_route_data(std::string stand_name, bool is_drive_off)
{

	// stand routes are stand-spesific routes for each stand -- for instance, the default route might clip through an obstace near the stand, this way you can make a custom route to avoid that


	namespace fs = std::filesystem;

	std::vector<std::vector<double>> route_to_be_added;
	std::string whole_file;

	if (fs::exists(get_stands_txt_dir(selected_airport)))
	{
		whole_file = read_file_to_string(get_stands_txt_dir(selected_airport));

		// file exists. Now, search if the desired stand is there. If yes, resize to two and push the coordinates etc. If the size stays at 0, we know that we are gonna use the "default" route.

		std::string str_to_find = "#";
		str_to_find += stand_name;

		std::string::size_type start_pos = whole_file.find(str_to_find);

		if (start_pos == std::string::npos)
		{
			//XPLMDebugString("\n\n\n\nNO STAND PROFILE FOUND\n\n\n\n");
			return route_to_be_added;
		}
		

		route_to_be_added.resize(2);
		do
		{
			start_pos++;
		} while (whole_file[start_pos] != '!');
		 // this is where the route starts, before that it's waiting positions

		if (is_drive_off == 1)
		{
			do
			{
				start_pos++;
			} while (whole_file[start_pos] != '!');
		}
		// and after the next ! starts the drive off route 

		for (size_t i = start_pos + 1 ; i < whole_file.size(); i++)
		{
			if (whole_file[i] == '#')
			{
				break;
			}

			if (is_drive_off == 0 && whole_file[i] == '!')
			{
				break;
			}


			// x coordinate
			std::string temp = "";
			while (whole_file[i] != ',') // until comma for lat, after that until newline -> lon
			{
				temp += whole_file[i];
				i++;
			}
			i++; // to next letter after comma

	
			route_to_be_added[0].push_back(std::stod(temp)); // tempstr to double that gets stored. Then get the lon
			




			temp = "";



			while (whole_file[i] != '\n')
			{
				temp += whole_file[i];
				i++;
			}

			//temp.pop_back(); // erase the comma from temp
			route_to_be_added[1].push_back(std::stod(temp));

			temp == "";



			


		}


		// convert to local
		for (int i = 0; i < route_to_be_added[0].size(); i++)
		{
			double temp_x, temp_z;
			world_coordinates_to_local(route_to_be_added[0][i], route_to_be_added[1][i], temp_x, temp_z);
			route_to_be_added[0][i] = temp_x;
			route_to_be_added[1][i] = temp_z;
		}
	}
	else
	{
		//XPLMDebugString("\n\n\n\nNO STAND PROFILE FOUND\n\n\n\n");
	}

	return route_to_be_added;
}



std::vector<std::vector<double>> get_stand_waiting_positions(std::string stand_name)
{


	stand_waiting_positions.resize(3);
	stand_waiting_positions[0].resize(0); // lat 
	stand_waiting_positions[1].resize(0); // lon
	stand_waiting_positions[2].resize(0); // hdg


	namespace fs = std::filesystem;

	std::string whole_file;

	if (fs::exists(get_stands_txt_dir(selected_airport)))
	{
		whole_file = read_file_to_string(get_stands_txt_dir(selected_airport));

		// file exists. Now, search if the desired stand is there. If yes, resize to two and push the coordinates etc. If the size stays at 0, we know that we are gonna use the "default" route.

		std::string str_to_find = "#";
		str_to_find += stand_name;

		std::string::size_type start_pos = whole_file.find(str_to_find);

		if (start_pos == std::string::npos)
		{
			//XPLMDebugString("\n\n\n\nNO STAND PROFILE FOUND\n\n\n\n");
			return stand_waiting_positions;
		}

		do
		{
			start_pos++;
		} while (whole_file[start_pos] != '\n');




		// this is where the waiting positions start

		for (size_t i = start_pos + 1; i < whole_file.size(); i++)
		{


			if (whole_file[i] == '!')
			{
				break;
			} 
			// routes would start


			// x coordinate
			std::string temp = "";
			while (whole_file[i] != ',') // until comma for lat, after that until newline -> lon
			{
				temp += whole_file[i];
				i++;
			}
			i++; // to next letter after comma


			stand_waiting_positions[0].push_back(std::stod(temp)); // tempstr to double that gets stored. Then get the lon






			temp = "";



			while (whole_file[i] != ',')
			{
				temp += whole_file[i];
				i++;
			}
			i++;

			stand_waiting_positions[1].push_back(std::stod(temp));



			temp = "";

			// then hdg [2]



			while (whole_file[i] != '\n')
			{
				temp += whole_file[i];
				i++;
			}

			stand_waiting_positions[2].push_back(std::stod(temp));



			temp = "";
		}

		// XPLMDebugString("SGS[DEBUG] -- Selected stand waiting positions\n");
		/*
		for (int i = 0; i < stand_waiting_positions[0].size(); i++)
		{
			print_number_to_log(stand_waiting_positions[0][i]);
			XPLMDebugString("\t");

			print_number_to_log(stand_waiting_positions[1][i]);
			XPLMDebugString("\t");

			print_number_to_log(stand_waiting_positions[2][i]);
			XPLMDebugString("\n");

		}
		*/

		// convert to local
		for (int i = 0; i < stand_waiting_positions[0].size(); i++)
		{
			double temp_x, temp_z;

			world_coordinates_to_local(stand_waiting_positions[0][i], stand_waiting_positions[1][i], temp_x, temp_z);

			stand_waiting_positions[0][i] = temp_x;
			stand_waiting_positions[1][i] = temp_z;
		}



		
	}


	 // set the reserve system

		stand_waiting_position_availability.resize(stand_waiting_positions[0].size());
	

	return stand_waiting_positions;

}

bool is_stand_waiting_pos_avail(int n)
{
	if (stand_waiting_position_availability[n] == 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


void reserve_waiting_pos(int n)
{
	if (n >= 0)
	{
		stand_waiting_position_availability[n] = 1;
	}
	
}


void free_waiting_pos(int n)
{
	if (n >= 0)
	{
		stand_waiting_position_availability[n] = 0;

	}
	/*
//	XPLMDebugString("SGS[DEBUG] -- waiting pos reserved: \n");
	for (int i = 0; i < stand_waiting_position_availability.size(); i++)
	{
		
		print_number_to_log(i);
		XPLMDebugString("\t");
		print_number_to_log(stand_waiting_position_availability[i]);
		XPLMDebugString("\n");

		
	}
	*/
}





std::vector<std::vector<double>>  get_pax_route_data(std::string stand_name)
{


	//XPLMDebugString("Mita vittua\n\n\n\n");

	namespace fs = std::filesystem;

	std::vector<std::vector<double>> route_to_be_added;
	std::string whole_file;

	if (fs::exists(get_pax_txt_dir(selected_airport)))
	{
		whole_file = read_file_to_string(get_pax_txt_dir(selected_airport));
		// XPLMDebugString(whole_file.c_str());
		// file exists. Now, search if the desired stand is there. If yes, resize to two and push the coordinates etc. If the size stays at 0, we know that we are gonna use the "default" route.

		std::string str_to_find = "#";
		str_to_find += stand_name;
		str_to_find += "\n"; // needs to end with a newline. So eg "Stand 11" doesn't get mixed with "Stand 1"

		//XPLMDebugString("SGS[DEBUG] -- Looking for entry:[");
		//XPLMDebugString(str_to_find.c_str());
		//XPLMDebugString("]\n");

		std::string::size_type start_pos = whole_file.find(str_to_find);

		if (start_pos == std::string::npos)
		{
			
			XPLMDebugString("SGS -- NO PAX ROUTE FOUND\n\n\n\n");
			return route_to_be_added; // if not found, return empty
		}
		

		route_to_be_added.resize(2);
		start_pos += str_to_find.length();

		for (size_t i = start_pos ; i < whole_file.size(); i++)
		{
			//std::string chr = whole_file[i];
			//XPLMDebugString(chr.c_str());
			if (whole_file[i] == '#') // next one starts
			{
				break;
			}


			// x coordinate
			std::string temp = "";
			while (whole_file[i] != ',') // until comma for lat, after that until newline -> lon
			{
				temp += whole_file[i];
				i++;
			}
			i++; // to next letter after comma

			//XPLMDebugString("Adding custom pax point:\n");
			//XPLMDebugString(temp.c_str());
			//XPLMDebugString("\t");

			route_to_be_added[0].push_back(std::stod(temp)); // tempstr to double that gets stored. Then get the lon
			




			temp = "";



			while (whole_file[i] != '\n')
			{
				temp += whole_file[i];
				i++;
			}

			//temp.pop_back(); // erase the comma from temp
			
			//XPLMDebugString(temp.c_str());
			//XPLMDebugString("\n");
			route_to_be_added[1].push_back(std::stod(temp));

			temp == "";



			


		}


		// convert to local
		for (int i = 0; i < route_to_be_added[0].size(); i++)
		{
			double temp_x, temp_z;
			world_coordinates_to_local(route_to_be_added[0][i], route_to_be_added[1][i], temp_x, temp_z);
			route_to_be_added[0][i] = temp_x;
			route_to_be_added[1][i] = temp_z;


		}


		



	}
	else
	{
		//XPLMDebugString("\n\n\n\nNO STAND PROFILE FOUND\n\n\n\n");

	}


	XPLMDebugString("SGS[DEBUG] -- route to be added to stand ");
	XPLMDebugString(stand_name.c_str());
	XPLMDebugString(":\n");

	if (route_to_be_added.size() == 2)
	{
		for (int i = 0; i < route_to_be_added[0].size(); i++)
		{


			print_number_to_log(route_to_be_added[0][i]);
			XPLMDebugString("\t");
			print_number_to_log(route_to_be_added[1][i]);

			XPLMDebugString("\n");


		}
	}
	
	XPLMDebugString("SGS[DEBUG] -- Completed\n\n");


	return route_to_be_added;
}


bool is_walkable_stand(std::string stand_name)
{
	namespace fs = std::filesystem;

	std::string whole_file;

	if (fs::exists(get_pax_txt_dir(selected_airport)))
	{
		whole_file = read_file_to_string(get_pax_txt_dir(selected_airport));
		// XPLMDebugString(whole_file.c_str());
		// file exists. Now, search if the desired stand is there. If yes, resize to two and push the coordinates etc. If the size stays at 0, we know that we are gonna use the "default" route.

		std::string str_to_find = "#";
		str_to_find += stand_name;

		//XPLMDebugString("SGS[DEBUG] -- Looking for entry:\n");
		//XPLMDebugString(str_to_find.c_str());

		std::string::size_type start_pos = whole_file.find(str_to_find);

		if (start_pos == std::string::npos)
		{
			
			//XPLMDebugString("NO PAX ROUTE FOUND\n\n\n\n");
			return false; // if not found, return empty
		}
		else
		{
			return true;
		}
	}
	return false;
}