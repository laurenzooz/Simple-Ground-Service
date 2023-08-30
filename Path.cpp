#define _USE_MATH_DEFINES// to use pi, rad deg conversions

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

#include "Vehicle.h"
#include "Path.h"
#include "temp_message.h"
#include "Planes.h"
#include "Profile_reader.h"


// heading between two points. x, z (in local), destination x, z (local)
double calculate_heading(double x1, double z1, double x2, double z2)
{
	double hdg_rad = atan2(z2 - z1, x2 - x1);
	double hdg_deg = (hdg_rad * 180 / M_PI) + 90;
	if (hdg_deg <= 0) { hdg_deg += 360; }

	return hdg_deg;
}


// Distance between two points. obj position x, z (in local), destination x, z (local)
double calculate_distance(double x1, double z1, double x2, double z2)
{
	double distance;
	distance = sqrt((pow((x2 - x1), 2)) + (pow((z2 - z1), 2)));

	
	// debug stuff
	/*
	XPLMDebugString("SGS -- The distance is: ");
	print_number_to_log(distance);
	XPLMDebugString("\n");
	*/

	return distance;
}



// opengl x, z; then double to be set to world lat, lon
void local_coordinates_to_world(double x_in, double z_in, double& lat_out, double& lon_out)
{
	double alt;
	XPLMLocalToWorld(x_in, 0, z_in, &lat_out, &lon_out, &alt);
}

// world lat, lon, then doubles to be set to opengl x and z
void world_coordinates_to_local(double lat_in, double lon_in, double& x_out, double& z_out)
{
	double y_out;
	XPLMWorldToLocal(lat_in, lon_in, 0, &x_out, &y_out, &z_out);

	/*
	XPLMDebugString("SGS[DEBUG] -- coordinates to be converted: \nLat: ");
	print_number_to_log(lat_in);
	XPLMDebugString(" Lon: ");
	print_number_to_log(lon_in);
	XPLMDebugString("\n");



	XPLMDebugString("SGS[DEBUG] -- converted: \nX: ");
	print_number_to_log(x_out);
	XPLMDebugString(" Z: ");
	print_number_to_log(z_out);
	XPLMDebugString("\n");
	*/
}



void Vehicle::change_route_direction()


{


	if (is_drive_off == 0) // changing from drive up to drive off
	{
		route_coordinates[0].resize(drive_off_route[0].size());
		route_coordinates[1].resize(drive_off_route[1].size());

		for (int i = 0; i < route_coordinates[0].size(); i++)
		{
			route_coordinates[0][i] = drive_off_route[0][i];
			route_coordinates[1][i] = drive_off_route[1][i];
		}

		is_drive_off = 1;
	}

	else // drive off back to drive up
	{
		//XPLMDebugString("SGS[DEBUG] -- Drive off is 1. Changing to drive up.\n");
		route_coordinates[0].resize(drive_up_route[0].size());
		route_coordinates[1].resize(drive_up_route[1].size());

		for (int i = 0; i < route_coordinates[0].size(); i++)
		{
			route_coordinates[0][i] = drive_up_route[0][i];
			route_coordinates[1][i] = drive_up_route[1][i];
		}



		is_drive_off = 0;

	}

	// XPLMDebugString("SGS[DEBUG] -- Route direction change succesful!\n");

	if (vehicle_type == "bus")
	{
		set_needed_waypoints(true); // bus connects straight away

	}
	else
	{
		set_needed_waypoints();

	}


	next_waypoint = 0;
	/*
	XPLMDebugString("SGS[DEBUG] -- Next waypoint is ");
	print_number_to_log(next_waypoint);
	XPLMDebugString("\t");
	print_number_to_log(route_coordinates[0][next_waypoint]);
	XPLMDebugString("\t");
	print_number_to_log(route_coordinates[1][next_waypoint]);
	XPLMDebugString("\n");
	*/

}

void Vehicle::set_needed_waypoints(bool is_for_connect)
{

	XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
	XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");
	XPLMDataRef heading = XPLMFindDataRef("sim/flightmodel/position/psi");


	//XPLMDebugString("SGS[DEBUG] -- Setting the waypoints to the route the vehicle actually follows\n");

	// when driving up, find the waypoint with shortest distance to destination, delete the extra points after that, add destination as last.
	if (is_drive_off == 0)
	{
		//XPLMDebugString("SGS[DEBUG] -- Drive on detected. Adding extra waypoints\n");
		double x_dest = 0, z_dest = 0, x_prep1 = 0, z_prep1 = 0, x_prep2 = 0, z_prep2 = 0, x_conn1 = 0, z_conn1 = 0, x_conn2 = 0, z_conn2 = 0;
		
		std::vector<std::vector<double>> stand_route_coordinates;
		if (!is_global) {stand_route_coordinates = get_stand_route_data(desired_stand);}


		//debug print
		/*
		XPLMDebugString("SGS[DEBUG] -- Stand route data x, y\n");
		if (stand_route_coordinates.size() == 3)
		{
			for (int i = 0; i < stand_route_coordinates[0].size(); i++)
			{
				XPLMDebugString(std::to_string(stand_route_coordinates[0][i]).c_str());
				XPLMDebugString("\t");
				XPLMDebugString(std::to_string(stand_route_coordinates[1][i]).c_str());
				XPLMDebugString("\n");

			}
		}
		*/


		double stand_x, stand_z, stand_hdg;
		if (!is_global) {get_stand_data(desired_stand, stand_x, stand_z, stand_hdg);}
		// stand data

		// prep points when not connecting. Bus has it's own route
		if (is_for_connect == 0 && stand_route_coordinates.size() != 2 && vehicle_type != "bus")
		{


			//XPLMDebugString("SGS[DEBUG] -- Stand data read, starting to calculate relative points.\n");


			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_prep1, z_prep1, stand_x, stand_z, stand_hdg, 15, true);
			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_prep2, z_prep2, stand_x, stand_z, stand_hdg, 5, true);

		}


		if (is_for_connect == 1) //  correction point when connecting. Calculated with the stand position (for waiting point), but when connecting use the plane pos for accuracy

		{
			//XPLMDebugString("SGS[DEBUG] -- Data read, starting to calculate relative points for connect.\n");


			// if behind wing, the first connection point is behind the wing.
			if (is_behind_wing(assigned_acf_vehicle_name, vehicle_type))
			{
				calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn1, z_conn1, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading), 35);
			}
			else
			{
				calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn1, z_conn1, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading), 15);
			}



			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn2, z_conn2, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading), 5);
			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_dest, z_dest, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading));

			


		}


		else if (is_for_connect == 0 && vehicle_type != "bus")
		{
			if (is_behind_wing(assigned_acf_vehicle_name, vehicle_type))
			{
				calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn1, z_conn1, stand_x, stand_z, stand_hdg, 35);
			}
			else
			{
				calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn1, z_conn1, stand_x, stand_z, stand_hdg, 15);
			}


			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_conn2, z_conn2, stand_x, stand_z, stand_hdg, 5);
			// calculate connection point anyway when not connecting, with stand data tho

		}






		// the route will be resized to this. For starters just use the current size, depends on the method used what it'll actually be resized to.
		int new_route_size = route_coordinates[0].size();


		// interpolate to add the waypoints between, then find the point where to deviate to the destination and delete not needed waypoints at the end. Not needed in global mode or when connecting 
		if (is_for_connect == 0 && vehicle_type != "bus")
		{
			/*

			XPLMDebugString("SGS[DEBUG] -- route of vehicle before interpolation ");
			XPLMDebugString(vehicle_type.c_str());
			XPLMDebugString("_");
			print_number_to_log(index);
			XPLMDebugString("\n");
			
			for (int i = 0; i < route_coordinates[0].size(); i++)
			{
				XPLMDebugString(std::to_string(route_coordinates[0][i]).c_str());
				XPLMDebugString("\t");

				XPLMDebugString(std::to_string(route_coordinates[1][i]).c_str());
				XPLMDebugString("\n");

			}
			*/
			route_coordinates = interpolate_midpoints(route_coordinates, 20);

			/*
			XPLMDebugString("SGS[DEBUG] -- Final, complete route of vehicle after interpolation ");
			XPLMDebugString(vehicle_type.c_str());
			XPLMDebugString("_");
			print_number_to_log(index);
			XPLMDebugString("\n");
			
			for (int i = 0; i < route_coordinates[0].size(); i++)
			{
				XPLMDebugString(std::to_string(route_coordinates[0][i]).c_str());
				XPLMDebugString("\t");

				XPLMDebugString(std::to_string(route_coordinates[1][i]).c_str());
				XPLMDebugString("\n");

			}
			*/
			// basically if there is a route found, then use that instead.
			if (stand_route_coordinates.size() == 2 && !is_global)
			{
				//XPLMDebugString("SGS[DEBUG] -- Done, looking for shortest deviation for custom route\n");
				new_route_size = find_shortest_distance_index(route_coordinates, stand_route_coordinates[0][0], stand_route_coordinates[1][0]) + 1; // special route for the stand
			}
			else if (!is_global)
			{
				//XPLMDebugString("SGS[DEBUG] -- Done, looking for shortest deviation point (normal)\n");
				new_route_size = find_shortest_distance_index(route_coordinates, x_prep1, z_prep1) + 1;
				// normal condition. No special stand route. Don't look for deviation point in global mode.
			}






			route_coordinates[0].resize(new_route_size);
			route_coordinates[1].resize(new_route_size);
			// set the size

		}




		// get the bus route, and then find the deviation point. Then add the dest
		if (vehicle_type == "bus" && !is_global)
		
		{

			route_coordinates = interpolate_midpoints(route_coordinates, 20);
			
			std::vector<std::vector<double>> route_around_plane = calculate_bus_route(XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading));



			new_route_size = find_shortest_distance_index(route_coordinates, route_around_plane[0][0], route_around_plane[1][0]);
			route_coordinates[0].resize(new_route_size);
			route_coordinates[1].resize(new_route_size);

			route_coordinates[0].insert(route_coordinates[0].end(), route_around_plane[0].begin(), route_around_plane[0].end());
			route_coordinates[1].insert(route_coordinates[1].end(), route_around_plane[1].begin(), route_around_plane[1].end());

			route_coordinates[0].push_back(x_dest);
			route_coordinates[1].push_back(z_dest);


		}








		//XPLMDebugString("SGS[DEBUG] -- Adding new points at the end\n");

		if (is_for_connect == 0 && vehicle_type != "bus")
		{


			if (stand_route_coordinates.size() == 2)
			{

				// get the custom waiting points
				std::vector<std::vector<double>> stand_waiting_positions = get_stand_waiting_positions(desired_stand);

				if (stand_waiting_positions[0].size() == 0) // if not any, calculate default relative points
				{

					//XPLMDebugString("SGS[DEBUG] -- Default relative points \n");

					double temp_x1, temp_y1, temp_x2, temp_y2;

					calculate_relative_point(assigned_acf_vehicle_name, vehicle_type,
						temp_x1, temp_y1,
						stand_route_coordinates[0][stand_route_coordinates[0].size() - 2], stand_route_coordinates[1][stand_route_coordinates[0].size() - 2],


						calculate_heading(stand_route_coordinates[0][stand_route_coordinates[0].size() - 1], stand_route_coordinates[1][stand_route_coordinates[0].size() - 1],
							stand_route_coordinates[0][stand_route_coordinates[0].size() - 2], stand_route_coordinates[1][stand_route_coordinates[0].size() - 2]),


						0, true);

					calculate_relative_point(assigned_acf_vehicle_name, vehicle_type,
						temp_x2, temp_y2,
						stand_route_coordinates[0][stand_route_coordinates[0].size() - 1], stand_route_coordinates[1][stand_route_coordinates[0].size() - 1],


						calculate_heading(stand_route_coordinates[0][stand_route_coordinates[0].size() - 1], stand_route_coordinates[1][stand_route_coordinates[0].size() - 1],
							stand_route_coordinates[0][stand_route_coordinates[0].size() - 2], stand_route_coordinates[1][stand_route_coordinates[0].size() - 2]),


						0, true);


					stand_route_coordinates[0][stand_route_coordinates[0].size() - 2] = temp_x1;
					stand_route_coordinates[1][stand_route_coordinates[0].size() - 2] = temp_y1;

					stand_route_coordinates[0][stand_route_coordinates[0].size() - 1] = temp_x2;
					stand_route_coordinates[1][stand_route_coordinates[0].size() - 1] = temp_y2;
				}




				else if (stand_waiting_positions[0].size() > 0)
				{
					// Find the nearest to the connection route start coordinate (x_conn and z_conn)

					int nearest_waiting_point = 0; // shortest distance from this indexed waypoint
					double shortest_distance = 9999; //  calculate_distance(stand_waiting_positions[0][0], stand_waiting_positions[1][0], x_conn, z_conn);



					// XPLMDebugString("SGS[DEBUG] -- Finding a waiting pos. index    distance    shortest so far\n");


					for (int i = 0; i < stand_waiting_positions[0].size(); i++)
					{
						// distance from every waypoint, if shorter than previously found shortest value, save it. Shortest value will remain
						double distance_from_i = calculate_distance(stand_waiting_positions[0][i], stand_waiting_positions[1][i], x_conn1, z_conn1);

						/*
						print_number_to_log(i);
						XPLMDebugString("\t");
						print_number_to_log(distance_from_i);
						XPLMDebugString("\t");
						print_number_to_log(shortest_distance);
						XPLMDebugString("\n");
						*/


						if (distance_from_i < shortest_distance && is_stand_waiting_pos_avail(i) == 1) // make sure the stand is free
						{
							shortest_distance = distance_from_i;
							nearest_waiting_point = i;
						}

					}

					/*
					XPLMDebugString("SGS[DEBUG] -- selected waiting pos index is ");
					print_number_to_log(nearest_waiting_point);
					XPLMDebugString("\n");
					*/
					reserved_waiting_pos = nearest_waiting_point;
					reserve_waiting_pos(reserved_waiting_pos); // reserve the spot

					// the waiting point itself

					double temp_x, temp_y;

					// make a point 15 meters away to get straight on the wpt
					temp_x = stand_waiting_positions[0][nearest_waiting_point] + (15 * sin((stand_waiting_positions[2][nearest_waiting_point]) * (M_PI / 180)));
					temp_y = stand_waiting_positions[1][nearest_waiting_point] - (15 * cos((stand_waiting_positions[2][nearest_waiting_point]) * (M_PI / 180)));




					stand_route_coordinates[0].push_back(temp_x);
					stand_route_coordinates[1].push_back(temp_y); // relative point, then the waiting point itself

					stand_route_coordinates[0].push_back(stand_waiting_positions[0][nearest_waiting_point]);
					stand_route_coordinates[1].push_back(stand_waiting_positions[1][nearest_waiting_point]);


				}








				// add the points to route
				//XPLMDebugString("SGS[DEBUG] -- stand route points that finally gets added: \n");
				for (int i = 0; i < stand_route_coordinates[0].size(); i++)
				{
					//print_number_to_log(stand_route_coordinates[0][i]);
					//XPLMDebugString("\t");
					//print_number_to_log(stand_route_coordinates[0][i]);
					//XPLMDebugString("\n");

					route_coordinates[0].push_back(stand_route_coordinates[0][i]);
					route_coordinates[1].push_back(stand_route_coordinates[1][i]);
				}




			}

			// default, no custom route to the stand

			else
			{
				//XPLMDebugString("SGS[DEBUG] -- Default waiting points pushed: \n");

				route_coordinates[0].push_back(x_prep1);
				route_coordinates[1].push_back(z_prep1);

				route_coordinates[0].push_back(x_prep2);
				route_coordinates[1].push_back(z_prep2);

			}





		}




		/// when connecting:
		else if (is_for_connect == 1 && (vehicle_type != "bus" || is_global))
		{
					
			route_coordinates[0].resize(0);
			route_coordinates[1].resize(0);


			// add the neewded points
			route_coordinates[0].push_back(x_conn1);
			route_coordinates[1].push_back(z_conn1);

			route_coordinates[0].push_back(x_conn2);
			route_coordinates[1].push_back(z_conn2);

			route_coordinates[0].push_back(x_dest);
			route_coordinates[1].push_back(z_dest);


			// test: one point over the target to avoid big corrections when close
			double x_over, z_over;
			calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_over, z_over, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading), -10);

			route_coordinates[0].push_back(x_over);
			route_coordinates[1].push_back(z_over);

		}






		//XPLMDebugString("SGS[DEBUG] -- Complete\n");

	}

	// when driving off, find the waypoint with shortest distance from current position, delete waypoints before that. No need to add extra waypoints in global mode
	else
	{

		double x_prep, z_prep;



		// drive off route 
		std::vector<std::vector<double>> stand_route_coordinates = get_stand_route_data(desired_stand, true);
		//XPLMDebugString("SGS[DEBUG] -- Stand data read\n");


		calculate_relative_point(assigned_acf_vehicle_name, vehicle_type, x_prep, z_prep, XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading), 30);
		//XPLMDebugString("SGS[DEBUG] -- Relative point calculated\n");

		// set the point to reverse to

		// For both: delete all the waypoints before the nearest point (if eg changing to drive off while middle of the drive up route, the vehicle can just turn



		// interpolate to add the midpoints, then find the deviation point
		route_coordinates = interpolate_midpoints(route_coordinates, 20);


		int route_start_index = find_shortest_distance_index(route_coordinates, x_prep, z_prep);

		// no deviation point for the bus

		if (vehicle_type == "bus")
		{
			route_start_index = find_shortest_distance_index(route_coordinates, draw_info.x, draw_info.z);
		}

		//XPLMDebugString("SGS[DEBUG] -- Shortest distance index found\n");

		else if (stand_route_coordinates.size() == 2)
		{
			route_start_index = find_shortest_distance_index(route_coordinates, stand_route_coordinates[0][stand_route_coordinates[0].size() - 1], stand_route_coordinates[1][stand_route_coordinates[0].size() - 1]);
			//XPLMDebugString("SGS[DEBUG] -- Shortest distance index found with custom stand coordinates\n");

		}

		//XPLMDebugString("SGS[DEBUG] -- Deleting extra points\n");

		for (int i = 0; i < route_start_index; i++)
		{
			route_coordinates[0].erase(route_coordinates[0].begin());
			route_coordinates[1].erase(route_coordinates[1].begin());
			// delete the route before where the vehicle joins it.
		}

		/* //temporarily disabled
		// set the new items at front
		if (stand_route_coordinates.size() == 2)
		{

		}
		else
		{
			auto startx = route_coordinates[0].begin();
			auto startz = route_coordinates[1].begin();
			route_coordinates[0].insert(startx, x_prep);
			route_coordinates[1].insert(startz, z_prep);
		}
		*/





		/*
		XPLMDebugString("SGS[DEBUG] -- First point is: \t");
		XPLMDebugString(std::to_string(route_coordinates[0][0]).c_str());
		XPLMDebugString("\t");
		XPLMDebugString(std::to_string(route_coordinates[1][0]).c_str());
		XPLMDebugString("\n");
		*/

		// XPLMDebugString("SGS[DEBUG] -- Inserting new points\n");

		if (stand_route_coordinates.size() == 2)
		{
			for (int i = stand_route_coordinates[0].size() - 1; i >= 0; i--) // insert the last one first (because inserted to the first place)
			{
				auto startx = route_coordinates[0].begin();
				auto startz = route_coordinates[1].begin();
				route_coordinates[0].insert(startx, stand_route_coordinates[0][i]);
				route_coordinates[1].insert(startz, stand_route_coordinates[1][i]);

				/*
				XPLMDebugString("SGS[DEBUG] -- Point inserted: \t");
				XPLMDebugString(std::to_string(route_coordinates[0][i]).c_str());
				XPLMDebugString("\t");
				XPLMDebugString(std::to_string(route_coordinates[1][i]).c_str());
				XPLMDebugString("\n");
				*/
			}
		}


		if (vehicle_type != "bus")
		{
			auto startx = route_coordinates[0].begin();
			auto startz = route_coordinates[1].begin();
			route_coordinates[0].insert(startx, x_prep);
			route_coordinates[1].insert(startz, z_prep);
		}
		
		


		/*
		XPLMDebugString("SGS[DEBUG] -- prep point inserted:\t");
		XPLMDebugString(std::to_string(x_prep).c_str());
		XPLMDebugString("\t");
		XPLMDebugString(std::to_string(z_prep).c_str());
		XPLMDebugString("\n");


		XPLMDebugString("SGS[DEBUG] -- Complete\n");

		*/
	}

	// case of already moving, just join the route the shortest way

	if (speed != 0)
	{
		int route_start_index = find_shortest_distance_index(route_coordinates, draw_info.x, draw_info.z);

		for (int i = 0; i < route_start_index; i++)
		{
			route_coordinates[0].erase(route_coordinates[0].begin());
			route_coordinates[1].erase(route_coordinates[1].begin());
			// delete the route before where the vehicle joins it.
		}

	}



	
	/*
	XPLMDebugString("SGS[DEBUG] -- Final, complete route of vehicle ");
	XPLMDebugString(vehicle_type.c_str());
	XPLMDebugString("_");
	print_number_to_log(index);
	XPLMDebugString("\n");
	
	for (int i = 0; i < route_coordinates[0].size(); i++)
	{
		XPLMDebugString(std::to_string(route_coordinates[0][i]).c_str());
		XPLMDebugString("\t");

		XPLMDebugString(std::to_string(route_coordinates[1][i]).c_str());
		XPLMDebugString("\n");

	}


	XPLMDebugString("SGS[DEBUG] -- Final, complete route of vehicle in world coordinates");
	XPLMDebugString(vehicle_type.c_str());
	XPLMDebugString("_");
	print_number_to_log(index);
	XPLMDebugString("\n");
	
	for (int i = 0; i < route_coordinates[0].size(); i++)
	{
		double x_in = route_coordinates[0][i];
		double z_in = route_coordinates[1][i];
		double lat_out, lon_out;

		local_coordinates_to_world(x_in, z_in, lat_out, lon_out);
		print_number_to_log(lat_out);

		//XPLMDebugString("\t");
		//print_number_to_log(lon_out);
		//XPLMDebugString("\n");

	}
	*/
}


int find_shortest_distance_index(std::vector<std::vector<double>> route, double x, double z, bool is_log)
{
	/*
		XPLMDebugString("SGS[DEBUG] -- looking for shortest distance index from \n");
		print_number_to_log(x);
		XPLMDebugString("\t");
		print_number_to_log(z);
		XPLMDebugString("\n");

		XPLMDebugString("SGS[DEBUG] -- in world coordinates\n");

		double temp_x, temp_z;

		local_coordinates_to_world(x, z, temp_x, temp_z);

		print_number_to_log(temp_x);
		XPLMDebugString("\t");
		print_number_to_log(temp_z);
		XPLMDebugString("\n");


		XPLMDebugString("SGS[DEBUG] -- index -- measured distance -- shortest distance \n");
	*/
	
		
	

	int shortest_distance_index = 0; // shortest distance from this indexed waypoint
	double shortest_distance = calculate_distance(route[0][0], route[1][0], x, z); // actual shortest distance found so far

	for (int i = 0; i < route[0].size(); i++)
	{
		// distance from every waypoint, if shorter than previously found shortest value, save it. Shortest value will remain
		double distance_from_i = calculate_distance(route[0][i], route[1][i], x, z);
		if (distance_from_i < shortest_distance)
		{
			shortest_distance = distance_from_i;
			shortest_distance_index = i;
		}

		
		
			
		

	}


	
	
	return shortest_distance_index;
}


std::vector<std::vector<double>> interpolate_midpoints (std::vector<std::vector<double>> route_coordinates, double spacing)
{

	
	// create temp vectors where to add the stuff(interpolated points), and once it's done, set the original route vector equal to the temp one 
	std::vector<std::vector<double>> route_coordinates_temp;
	route_coordinates_temp.resize(2);

	for (size_t i = 0; i < route_coordinates[0].size() - 1; i++)
	{
		double temp_x = route_coordinates[0][i];
		double temp_z = route_coordinates[1][i];
		// the current point. Set it in the temp vector first
		route_coordinates_temp[0].push_back(temp_x);
		route_coordinates_temp[1].push_back(temp_z);


		double temp_hdg = calculate_heading(temp_x, temp_z, route_coordinates[0][i + 1], route_coordinates[1][i + 1]);
		double temp_dist = calculate_distance(temp_x, temp_z, route_coordinates[0][i + 1], route_coordinates[1][i + 1]);


		// only add the temp waypoints if the distance is big enough, larger than the desires spacing
		while (temp_dist > spacing)
		{
			// XPLMDebugString("Adding the spacing thingies\n\n\n");
			temp_x += (spacing)*sin((temp_hdg * (M_PI / 180)));
			temp_z += (-spacing) * cos((temp_hdg * (M_PI / 180)));

			temp_dist -= spacing;

			route_coordinates_temp[0].push_back(temp_x);
			route_coordinates_temp[1].push_back(temp_z);

		}




	}


	// push the last wpt
	route_coordinates_temp[0].push_back(route_coordinates[0][route_coordinates[0].size() - 1]);
	route_coordinates_temp[1].push_back(route_coordinates[1][route_coordinates[1].size() - 1]);

	return route_coordinates_temp;
}

double Vehicle::calculate_distance_to_destination(bool is_drive_off_reverse)
{
	double distance = -1; // error
	//XPLMDebugString("SGS[DEBUG] -- Calculating remaining distance\n");
	if (route_coordinates[0].size() > 1)//(target_speed != 0 || speed != 0))
	{

		size_t last_wpt = route_coordinates[0].size() - 1;
		if (is_drive_off_reverse == 1)
		{

			last_wpt = route_coordinates[0].size() - 2;

		}

		distance = calculate_distance(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
		for (size_t i = next_waypoint; i < last_wpt; i++)
		{
			distance += calculate_distance(route_coordinates[0][i], route_coordinates[1][i], route_coordinates[0][i + 1], route_coordinates[1][i + 1]);
		}

		// if for connect, remove the distance of the last wpt (that's the over point)

		if (is_connect)
		{
			distance -= 10;
		}
	}

	/*
	if (is_drive_off_reverse == 1)
	{
		XPLMDebugString("SGS[DEBUG] -- Remaining distance to start the reversing ");
		XPLMDebugString(vehicle_type.c_str());
		XPLMDebugString("_");
		print_number_to_log(index);
		XPLMDebugString(" is: ");
		print_number_to_log(distance);
		XPLMDebugString("\n");
	}
	*/

	

	return distance;
}




// TODO: replace the vehicle specific method with this more common distance measurer

double measure_path_distance(std::vector<std::vector<double>> path)
{
	double distance = 0;
	if (path.size() == 2 && path[0].size() > 0)//(target_speed != 0 || speed != 0))
	{
		for (size_t i = 1; i < path[0].size(); i++)
		{
			distance += calculate_distance(path[0][i - 1], path[1][i - 1], path[0][i], path[1][i]);
		}

	
	}

	//XPLMDebugString("SGS[DEBUG] -- Distance of the path: ");
	//print_number_to_log(distance);
	//XPLMDebugString("\n");

	return distance;
}



std::vector<std::vector<double>> global_mode_route(std::string acf_vehicle_name, std::string vehicle_type, std::string desired_stand)
{
	//XPLMDebugString("Setting global route!: ");
	//XPLMDebugString(acf_vehicle_name.c_str());
	//XPLMDebugString("\n");

	std::vector<std::vector<double>> route;
	route.resize(2);


	XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
	XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");
	XPLMDataRef heading = XPLMFindDataRef("sim/flightmodel/position/psi");

	double pos_x, pos_z, pos_hdg;

	if (desired_stand.length() > 0) { // if desired stand is set, use that instead of plane pos
		get_stand_data(desired_stand, pos_x, pos_z, pos_hdg);
	} else { // if it is set
		pos_x = XPLMGetDataf(plane_x_pos_ref);
		pos_z = XPLMGetDataf(plane_z_pos_ref);
		pos_hdg = XPLMGetDataf(heading);
	}


	double x0, z0, x1, z1;

	if (vehicle_type != "bus")
	{
		calculate_relative_point(acf_vehicle_name, vehicle_type, x0, z0, pos_x, pos_z, pos_hdg, 10, true);
		calculate_relative_point(acf_vehicle_name, vehicle_type, x1, z1, pos_x, pos_z, pos_hdg, 0, true);

		route[0].push_back(x0);
		route[1].push_back(z0);
	
		route[0].push_back(x1);
		route[1].push_back(z1);
	} else 
	{ // bus doesn't move in global mode
		calculate_relative_point(acf_vehicle_name, vehicle_type, x0, z0, pos_x, pos_z, pos_hdg);
		calculate_relative_point(acf_vehicle_name, vehicle_type, x1, z1, pos_x, pos_z, pos_hdg, -5);

		route[0].push_back(x0);
		route[1].push_back(z0);

		route[0].push_back(x1);
		route[1].push_back(z1);
	}

	


	return route;
}
