#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"
#include "XPLMDataAccess.h"


#include <string>
#include <vector>
#include <cmath>

#include "People.h"
#include "Path.h"
#include "temp_message.h"


//// Human:

double Human::distance_to_closest(std::vector<Human*> people) // distance to the closest human found in all the humans given(excluding itself)
{
	//std::vector<std::vector<double>> positions;
	//positions.resize(2);
	
	double shortest = 999;

/*
	// iterate through every human vector's every human and add them to the positions vector. Then find the shortest distance from this human.
	for (int i = 0; i < people.size(); i++)
	{
	
		positions[0].push_back(people[i].get_x());
		positions[1].push_back(people[i].get_z());

		
	}
*/

	//bool is_stopped = false; // set to true if another human within half a meter from this one has already stopped. 

	//XPLMDebugString("Doing it\n");

	for (int i = 0; i < people.size(); i++)
	{
		double temp_dist = calculate_distance(get_x(), get_z(), people[i]->get_x(), people[i]->get_z());
		
		//XPLMDebugString("Doing it\n");

		if (people[i]->is_stopped() && temp_dist > 0 && temp_dist < 2)
		//if (people[i].is_stopped()) // if another human has stopped (must sill be initalized though!)
		{
			//is_stopped = true; 
			//XPLMDebugString("There was a stopped human\n");
			return 999; // if someone half a meter from this obj has stopped, then keep going 
		}
		
		
		double hdg = calculate_heading(get_x(), get_z(), people[i]->get_x(), people[i]->get_z()); // calculate heading as well. The ones that are behind you, can be ignored
		
		// use heading to next waypoint rather than the current hdg

		double hdg_to_wpt = calculate_heading(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);

		double hdg_diff = fmod((hdg_to_wpt - hdg + 540), 360) - 180;

/*
		XPLMDebugString("temp_dist\thdg_diff\n");
		print_number_to_log(temp_dist);
		XPLMDebugString("\t");
		print_number_to_log(hdg_diff);
		XPLMDebugString("\n");
		*/
		if (temp_dist < shortest && hdg_diff > -90 && hdg_diff < 90 && temp_dist > 0)
		{
			shortest = temp_dist;
		}
	}

	//XPLMDebugString("Shortest distance to another human: ");
	//print_number_to_log(shortest);
	//XPLMDebugString("\n");

	return shortest;

}