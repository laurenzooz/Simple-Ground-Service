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
#include <algorithm>

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <random>
// for offsets

#include "People.h"
#include "Path.h"
#include "temp_message.h"
#include "Profile_reader.h"







int pax_count = 0;
bool is_descend = 0;

int trigger_time = 45; // minutes

int timer_reset = 0;


void reset_pax_count()
{
	XPLMDataRef elapsed_time = XPLMFindDataRef("sim/time/total_flight_time_sec");
			
	timer_reset = XPLMGetDataf(elapsed_time);

	is_descend = 0;

	pax_count = 0;
}

int get_pax_count()
{
	return pax_count;
}


// mostly copy pasted from vehicle



static void load_cb(const char* real_path, void* ref)
{
	//XPLMDebugString("SGS -- Loading object callback\n");
	XPLMObjectRef* dest = (XPLMObjectRef*)ref;

	if (*dest == NULL)
	{

		*dest = XPLMLoadObject(real_path);
		/*
		XPLMDebugString("SGS -- Real path: ");
		XPLMDebugString(real_path);
		XPLMDebugString("\n");
		*/
	}

}


// constructor
Human::Human()
{

	//XPLMDebugString("SGS -- Creating human\n");
	route_coordinates.resize(2);




	anim_data[0] = 0;		// the animation
	//XPLMDebugString("SGS -- Anim data initialized\n");



	draw_info.structSize = sizeof(draw_info);
	probe_info.structSize = sizeof(probe_info);
	//XPLMDebugString("SGS -- Structsizes set\n");

	draw_info.heading = 0;
	draw_info.pitch = 0;
	draw_info.roll = 0;
	// initialize draw and probe info


	


	//XPLMDebugString("SGS -- Human created\n");


}


void Human::load_instance(int obj_variation)
{

	// different variations
	if (obj_variation != 0)
	{
		obj_path = "Resources/plugins/Simple Ground Service/objects/pax/pax_";
		obj_path += std::to_string(obj_variation);
		obj_path += ".obj";
	}

	if (!object)
	{
		//XPLMDebugString("SGS -- Loading object for human\n");
		object = XPLMLoadObject(obj_path.c_str());

	}

	if (object && !instance)
	{
		//XPLMDebugString("SGS -- Loading instance for human\n");
		// default object is in different path than custom ones.

		const char* datarefs[] = { "SimpleGroundService/animations/pax", "SimpleGroundService/animations/pax/hide", NULL };

		/*
		XPLMDebugString("SGS -- Object path: ");
		XPLMDebugString(obj_path.c_str());
		XPLMDebugString("\n");
		*/
		instance = XPLMCreateInstance(object, datarefs);
		//XPLMDebugString("SGS -- Instance created\n");
		is_loaded = 1;

	}

}


void Human::initialize(std::vector<std::vector<double>> pax_route, double anim_pos_offset, double anim_pace_offset, bool is_descend, double step_height, double step_length, int amount_of_steps)
{
	//XPLMDebugString("SGS -- Initializing human\n");

	//this->is_descend = is_descend;
	
	if (step_height != -1) // if set to some value, meaning custom stairs are there
	{
		is_for_custom_stairs = 1;

		custom_step_amount = amount_of_steps;
		custom_step_height = step_height;
		custom_step_length = step_length;
		//stair_height = amount_of_steps * step_height + 1;// 1m marginal at least for now when testing

		/*
		XPLMDebugString("Using custom stairs with following parameters: step_amount, step_height, step_length, stair_height\n");
		print_number_to_log(custom_step_amount);
		XPLMDebugString("\t");
		print_number_to_log(custom_step_height);
		XPLMDebugString("\t");
		print_number_to_log(custom_step_length);
		XPLMDebugString("\t");
		print_number_to_log(stair_height);
		XPLMDebugString("\t");
		*/
	}

	/*
	XPLMDebugString("SGS[DEBUG] -- Pax are using stairs with following parameters: step_amount, step_height, step_length, stair_height\n");
	print_number_to_log(amount_of_steps);
	XPLMDebugString("\t");
	print_number_to_log(step_height);
	XPLMDebugString("\t");
	print_number_to_log(step_length);
	XPLMDebugString("\t");
	print_number_to_log(stair_height);
	XPLMDebugString("\t");
	*/

	// if descending, start at the top of the stairs
	//if (is_descend)
	///{
	//	height = 3.52;
	//}

	

	// heading offset 180;

	// route temporarily in global coordinates.

	route_coordinates = pax_route;

	if (is_descend)
	{
		next_waypoint = route_coordinates[0].size() - 1;
		draw_info.x = route_coordinates[0][route_coordinates[0].size() - 2];
		draw_info.z = route_coordinates[1][route_coordinates[0].size() - 2];
		
	}
	else
	{
		draw_info.x = route_coordinates[0][0];
		draw_info.z = route_coordinates[1][0];
	}

	//if (is_descend)
	//{
		//height = stair_height;
	//}
	// start with the 2nd to last point when descending (to gain separation)

	draw_info.y = terrain_probe() + height + 0.4;
	draw_info.heading = calculate_heading(route_coordinates[0][0], route_coordinates[1][0], route_coordinates[0][1], route_coordinates[1][1]) + 180;
//	XPLMDebugString("SGS -- Initial draw info set\n");

	//if (instance) { XPLMInstanceSetPosition(instance, &draw_info, anim_data); }
	
	
	is_initialized = 1;

	// XPLMDebugString("SGS -- Human initialization done\n");

	/*
	XPLMDebugString("SGS -- Route of human:\n");

	for (int i = 0; i < route_coordinates[0].size(); i++)
	{
		XPLMDebugString(std::to_string(route_coordinates[0][i]).c_str());
		XPLMDebugString("\t");

		XPLMDebugString(std::to_string(route_coordinates[1][i]).c_str());
		XPLMDebugString("\n");

	}
	*/
	anim_data[0] = anim_pos_offset;
	this->anim_pace_offset = anim_pace_offset;

}



void Human::destroy()
{
	is_initialized = 0;
	is_loaded = 0;
	status = 0;

	XPLMDestroyInstance(instance);
	XPLMUnloadObject(object);

}



//// get pos for distance to human in front:

double Human::get_x()
{
	return draw_info.x;
}

double Human::get_z() 
{
	return draw_info.z;
}

bool Human::is_stopped()
{

	return (speed == 0 && is_initialized);  // is stopped and initialized
}




float Human::terrain_probe()
{
	if (!probe_ref)
	{
		probe_ref = XPLMCreateProbe(0);
	}

	if (probe_ref)
	{
		probe_result = XPLMProbeTerrainXYZ(probe_ref, draw_info.x, 0, draw_info.z, &probe_info);


		return probe_info.locationY;

	}
	return -1;
}



void Human::update_human(std::vector<Human*> other_people, double deltaT, double stair_height, double dist_human)
{
	
	// don't do always to save frames
	//if ( ( (int)(floor(deltaT * 100))) % 5 == 0  && other_people.size() > 0) // don't do too often
	if (other_people.size() > 0)
	{
		dist_human = distance_to_closest(other_people);

	}

	speed = 1.25;
	vertical_speed = 0;


	//XPLMDebugString("stair_height = ");
	//print_number_to_log(stair_height);
	//XPLMDebugString("\n");

	// anim_data[1] = 1 // means its hidden, 0 means its shown


	if (anim_data[0] >= 0 && anim_data[0] <= 1.75)
	{
		anim_data[0] += deltaT * anim_pace_offset;
		//anim_data[0] += deltaT * 1.25;
	}
	else
	{
		anim_data[0] += deltaT; // 
	}

	// cycle the anim forward. Apply random pace when level


	// animation phases


	double tilted_surface_y_change = -sin(draw_info.pitch * (M_PI / 180)); // tilted surface


	if (anim_data[0] >= 0 && anim_data[0] <= 1.75)
	{
		speed = 1;
		vertical_speed = 0; // height stays the same
	}

	// porras animaatio hitaampi eteenp�in. T�ll�nen fixed juttu ny alkuun

	/*
	// 4cm nousua per frame, yhteen frameen ajateltu menev�n 0.05s.
	vanha ^^
	Nyt nousua 20cm sijaan 17.33cm. eli 17.33cm / 5 = 3.466cm

	Koko sykli kest��  2.00. Koko homman aikana 60cm eteenp�in. Eli 0.6m/2s = 0.3m/s. Oikein
	Vertikaalinopeus. Nousup�tk�t yhteens� 0.2*2 = 0.4. Sin� aikana 17.33 * 2 = 34.66cm yl�sp�in eli 0.3466m
	0.3466m / 0.4s = 0.8665 m/s



	absoluuttinen korkeudenmuutos 0.2 sekunnissa vertikaalinopeudella 0.8657 on 0.17314

	// lienee v�h� trial and error?? olkoon speed ny 0.3




	pitch angle:
	-3.1 kun korkeus 2.2
	0 kun korkeus 2.6
	7.75 * stair height - 20.15

	0 kun korkeus on 4.7
	6.5 kun korkeus on 5.9

	5.417 * stair_height - 25.45



	V�litasannekorkeus =
	2.6, kun korkeus 4.7
	3.07, kun korkeus 5.9

	platform height = -0.494 * stair_height + 5.98


	*/



	if (anim_data[0] >= 2.25)
	{
		// speed = 0.296;
		// speed = 0.29;
		speed = 0.28;

		if (anim_data[0] >= 4.35)
		{
			speed = 0.165; // little slower when descending
			//XPLMDebugString("slow descend");
		}
		
		
		if (is_for_custom_stairs == 1)
		{
			speed = custom_step_length; // needs tweaking
			
			
			//XPLMDebugString("SGS[DEBUG] -- Ascending stairs\n");			
		}

	}


	// handle the pitch
	// no pitch changes for custom stairs for now
	if (status == 2 && is_for_custom_stairs == 0)
	{
		if (stair_height < 2.6)
		{
			draw_info.pitch = -(7.75 * stair_height - 20.15);

		}
		else if (stair_height > 4.7)
		{
			draw_info.pitch = -(5.417 * stair_height - 25.45);
		}
	}

	if (is_for_custom_stairs == 0)
	{
		height += speed * tilted_surface_y_change * deltaT; // tilted surface compensation

	}
	else
	{
		draw_info.pitch = 0;
		tilted_surface_y_change = 0;
	}


	
	




	/*
	if ((anim_data[0] >= 2.85 && anim_data[0] <= 3.10) || (anim_data[0] >= 3.90 && anim_data[0] <= 4.15))
	{
		vertical_speed = 0.69259;

	}
	*/

	// lasketaan virhemarginaali 10fps. T�ll�in framejen v�lill� on 0.1s. Eli otetaan virhemarginaaliks 0.1, sit� isompaa muutosta ei varmasti kerralla tapahdu. Toivotaan ett� toimii suht j�rkev�sti.

	// normaalisti siis t�n jalan askel loppuis 3.10, virhemarginaalin kanssa otetaan 3.20

	//double step_height = 0.173 * cos(-draw_info.pitch * (M_PI / 180)); // need the absolute height change
	 double step_height = 0.1736;
	//double step_height = 0.1741;

	if (is_for_custom_stairs == 1) { step_height = custom_step_height; }

	
	
	// ascend

	if (!is_descend)
	{
		if (anim_data[0] >= 2.85 && anim_data[0] <= 3.50 && current_stair_height_change < step_height) 
		{
		//vertical_speed = 0.69259;
		vertical_speed = 0.75;
		//vertical_speed = 0.75;
		//XPLMDebugString("CLIMBING!!\n");
		}	
	
	else if (anim_data[0] >= 3.90 && anim_data[0] <= 4.25 && current_stair_height_change < step_height)
		{
		//vertical_speed = 0.69259;
		vertical_speed = 0.75;
		//vertical_speed = 0.72;
		//XPLMDebugString("CLIMBING!!\n");
		
		}
	}

	else
	{
		if (anim_data[0] >= 4.60 && anim_data[0] <= 5.80 && current_stair_height_change > -step_height) 
		{
			//vertical_speed = 0.69259;
			vertical_speed = -0.7;
			//vertical_speed = 0.75;
			//XPLMDebugString("Descending!!\n");
		}	
	
		else if (anim_data[0] >= 6.10 && anim_data[0] <= 7.10 && current_stair_height_change > -step_height)
		{
			//vertical_speed = 0.69259;
			vertical_speed = -0.7;
			//vertical_speed = 0.72;
			//XPLMDebugString("Descending!!\n");
		
		}
	}


	

	/*
	
	else if (anim_data[0] >= 4.30 && anim_data[0] <= 4.65)
	{
		vertical_speed = -0.7;
		vertical_speed = -0.7;
	}

	else if (anim_data[0] >= 4.65 && anim_data[0] <= 5.00)
	{
		vertical_speed = -0.7;
	}
	*/

	// descend disabled for now


	// nyk. portaan nousun laskun nollaus. Vasta kun yli marginaalin. T�ss� voi ottaa vaikka v�h�n isomman marginaalin varoiks, ei niink��n v�li�, kunhan marginaali ei mee ristiin toisen porrasnousun kanssa
	if (!is_descend && ((anim_data[0] > 3.50 && anim_data[0] <= 3.90) || anim_data[0] >= 4.25))
	{
		current_stair_height_change = 0; 
	}
	//)
	else if (is_descend && ((anim_data[0] > 6.00 && anim_data[0] < 6.10) || anim_data[0] >= 7.05)) 
	{
		current_stair_height_change = 0; 
		//XPLMDebugString("nollattu");
	} 


	// jonkun verran silti framekohtasta heittoo; ei aina voi loppua tasan 0.17314


	

	// the limits for each case
	if (status == 0 && anim_data[0] > 1.75 ) // normal walk loop
	{

		anim_data[0] = 0;
	}



	// transfer from walk to stair up. Speed up animation.
	/*if (status == 2 && anim_data[0] < 2.25)
	{
		speed = 0;
		anim_data[0] += deltaT * 5;
	}*/
	


	if (!is_descend && status == 2 && (anim_data[0] > 4.30 || anim_data[0] < 2.25 )) // cycle the stair up
	{
		anim_data[0] = 2.25;
	} 
	else if (is_descend && status == 2 && (anim_data[0] > 7.10 || anim_data[0] < 4.35 )) // cycle the stair down
	{
		anim_data[0] = 4.35;
	}




	// platform start height depending on tilt
	double platform_start_height = stair_height; // case of no tilt
	if (stair_height > 4.7)
	{
		platform_start_height = 0.7083 * stair_height + 1.37;
	}


	/*
	kun stair_height = 3.7, on platform start_height 4.7.
	Kun stair_height = 5.9, on platform start_height 5.55.

	0.7083 * stair_height + 1.37
	*/



	double ground_height_threshold = -0.5;
	// temporary; 

	if (is_descend) {ground_height_threshold = 0;} else {ground_height_threshold = -0.2;}

	// change to normal walking when reaching the platform start height
	if (height > platform_start_height && !is_descend)
	{
		status = 0;
		//XPLMDebugString("Status changed to 0 because of reaching platform start height\n");

	}
	
	else if (height < ground_height_threshold ) // small margin (temporarily 0.5) because when tilted, the 
	// height gets below 0 for a short moment before starting ascend.
	// TODO: replace -0.5 with precise calculation of the angle 
	{
		height = 0;
		status = 0;
		//XPLMDebugString("Status changed to 0 because of height less than 0 ??? height is: ");
		//print_number_to_log(height);
		//XPLMDebugString("\n");

	}



	// height limiter (the stair update thingy feeds it here) ei varmaan j�rkee joka kerta t�t�kin saada
	if (height > stair_height && !is_descend)
	{
		height = stair_height;
		status = 0;
		//XPLMDebugString("Status changed to 0 because of reaching stair height\n");
	}

	
	//XPLMDebugString("vertical speed, current stair height change, : ");
	//print_number_to_log(vertical_speed);
	//XPLMDebugString("\t");
	//print_number_to_log(current_stair_height_change);
	//XPLMDebugString("\n");





	// change statuses as necessary
	double dist = calculate_distance(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);

	if (dist > prev_dist && prev_dist >= 0 && dist < 3 )// || (is_drive_off == 1 && next_waypoint <= 1) ))
	{
		dist = 0;
	}
	
	/*
	XPLMDebugString("SGS[DEBUG] -- Human speed and distance to next waypoint ");
	print_number_to_log(next_waypoint);
	XPLMDebugString(" is ");
	print_number_to_log(speed);
	XPLMDebugString("\t");
	print_number_to_log(dist);
	XPLMDebugString("\n");
*/

	//XPLMDebugString("SGS[DEBUG] -- Human height: ");
	//print_number_to_log(height);
	//XPLMDebugString("\n");
	
	
	//15 cm margin
	if (dist < 0.15 && next_waypoint < route_coordinates[0].size() - 1 && dist >= 0 )
	{
		current_stair_height_change = 0; // will be different when nxt wpt is 6. To compensate the different height possibilities
		next_waypoint++;
		//XPLMDebugString("Next waypoint is ");
		//print_number_to_log(next_waypoint);
		//XPLMDebugString("\n");

		prev_dist = -1;
		dist = -1;



/*
				[0] == people spawn point. Hidden, because "behind" the bus. For spaces between people
				[1] == Bus, point where the human becomes visible
				[2] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				[3] == stairs start
				[4] == middle platform starts
				[5] == middle platform ends
				[6] == "inside" the plane (6 -> 7) the platform on the top


					/*
				[0] == people spawn point. Hidden, because "behind" the bus. For spaces between people
				[1] == Bus, point where the human becomes visible
				[2] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				[3] == stairs start
				[4] == "inside" the plane, back to start


				

				*/



		// status depending on which wpt next this for ascend
		if (!is_descend)
		{
			if ((next_waypoint == route_coordinates[0].size() - 3 && !is_for_custom_stairs) ||
			(next_waypoint == route_coordinates[0].size() - 1)) // (3 -> 4), stair part starts
			{
				status = 2;
				//XPLMDebugString("Status changed to 2 because reaching stairs\n");
			}
		

			else if (next_waypoint == route_coordinates[0].size() - 2 && height < stair_height && !is_for_custom_stairs)
			{
				status = 0;
				//XPLMDebugString("Status changed to 0 because of waypoint change\n");
			}

			else if (next_waypoint == route_coordinates[0].size() - 1 && height < stair_height && !is_for_custom_stairs) // middle platform ends. Correct the height. No middle platform for MD88
			{
				status = 2;
				//XPLMDebugString("Status changed to 2 because middle platform ends\n");


				if (stair_height < 4.7)
				{
					double offset = remainder((height - 2.6), 0.17356) - 0.02; // when negative, we can just subtract it from 0, aka the height change is same than the offset
					//(because we need to go a longer distance) when positive, minus from the step height.
					if (offset < 0)
					{
						current_stair_height_change = offset;
					}
					else
					{
						current_stair_height_change = 0.17356 - offset;
					}
				}


			}
		}
		
		else // descend
		{
			//XPLMDebugString("MITA BITTUAUAUUA\n\n\n\n\n\n\n");
			//if (next_waypoint >= 0 && height > 0)
			//{
			//	status = 2;
			//}

			
			
			if (next_waypoint == 1  || (next_waypoint == 3 && !is_for_custom_stairs)) // top to middle part, and from middle part down
			{
				status = 0; // middle platform or top platform
			}

			else if(next_waypoint == 2 || (next_waypoint == 4 && !is_for_custom_stairs) )
			{
				status = 2; // to be changed to 3 (descend anim)
			}

			if (stair_height < 4.7)
				{
					double offset = remainder((height - 2.6), 0.17356) - 0.02; // when negative, we can just subtract it from 0, aka the height change is same than the offset
					//(because we need to go a longer distance) when positive, minus from the step height.
					if (offset < 0)
					{
						current_stair_height_change = offset;
					}
					else
					{
						current_stair_height_change = 0.17356 - offset;
					}
				}
			
		}
		/*

				[0] == "inside" the plane
				[1] == Up platform ends, stairs start
				[2] == middle platform starts
				[3] == middle platform end
				[4] == stairs end
				[5] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				the rest is bus/custom route

				custom stair:
				[0] == "inside" the plane
				[1] == stairs start
				[2] == stairs end
				[3] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				the rest is bus/custom route
				
				
				
			
				
		*/
		
		

		// huom! toistaseks tarkka vaan 2.60-4.70 v�lill�, kun ei kallistusta. Luotava funktio jonka mukaan arvot muuttuu kallistuksen muuttuessa

		




		/*
		XPLMDebugString("SGS[DEBUG] -- Wpt change ");
		print_number_to_log(next_waypoint);
		XPLMDebugString("\n");
		*/

	}
	else
	{
		prev_dist = dist;

	}

	// temporary way to hide, objs appear at the bus

	
	//if (next_waypoint == 1 && !is_descend)
	if (next_waypoint == 1)
	{
		//XPLMDebugString("Route reset and ");
		status = 0;
		if (is_descend)
		{
			height = stair_height;
			anim_data[1] = 0;
			//status = 2;
			//XPLMDebugString("height set to stair height and human is shown\n");

		}
		else
		{
			height = -10;
			anim_data[1] = 1;
			
			//XPLMDebugString("height set to -10 and human is hidden\n");

		}
		


	}
	else if (next_waypoint == 2 && !is_descend)
	//else if (next_waypoint == 2)
	{
		status = 0;
		height = 0;
		anim_data[1] = 0;
	}
	else if (next_waypoint == route_coordinates[0].size() - 1 && is_descend)
	{
		height = -10;
		anim_data[1] = 0; 
	}
	//else if (is_descend && next_waypoint)
	

	// update the draw info itself




	// collision stuff temporarily disabled to test the descending stuff
	// for now, if too close to the human in front, just stop everything. Distance also depends on status, shorter distance on stairs is ok


	if ((dist_human < 0.8 && next_waypoint >= route_coordinates[0].size() - 4 && dist_human > 0) || (dist_human < 1.1 && dist_human > 0))
	{
		speed = 0;
		vertical_speed = 0;
	}



	// vertical speed straight up from the human, if human is tilted then the vertical speed is too and hence its divided to components
	double vertical_speed_x = - (vertical_speed * (sin(draw_info.pitch * (M_PI / 180))));
	double vertical_speed_y =    vertical_speed * (cos(draw_info.pitch * (M_PI / 180)));



	if (status == 0) // normal walking on a tilted surface
	{
		vertical_speed_x = cos(draw_info.pitch * (M_PI / 180));

		if (is_for_custom_stairs == 1)
		{
			vertical_speed_x = 1;
		}

		velocity_x =  (speed * vertical_speed_x) * ((deltaT) * (sin((draw_info.heading - 180) * (M_PI / 180))));
		velocity_z = -(speed * vertical_speed_x) * ((deltaT) * (cos((draw_info.heading - 180) * (M_PI / 180))));

	}
	else
	{
		velocity_x =  (speed - vertical_speed_x) * ((deltaT) * (sin((draw_info.heading - 180) * (M_PI / 180))));
		velocity_z = -(speed - vertical_speed_x) * ((deltaT) * (cos((draw_info.heading - 180) * (M_PI / 180))));
		
		
		double temp_old_height = height;
		height += vertical_speed_y * deltaT; 
		current_stair_height_change += vertical_speed_y * deltaT;
	
		//XPLMDebugString("> Human status -- vertical speed y  --  height_change\n");
		//print_number_to_log(vertical_speed_y);
		//XPLMDebugString("\t");

		//print_number_to_log(current_stair_height_change);
		//XPLMDebugString("\n");
			
	}

	/*

	XPLMDebugString("> Human status -- deltaT, v_x, v_y, scalar speed, vert speed\n");
	print_number_to_log(deltaT);
	XPLMDebugString("\t");
	print_number_to_log(velocity_x);
	XPLMDebugString("\t");
	print_number_to_log(velocity_z);
	XPLMDebugString("\t");
	print_number_to_log(speed);
	XPLMDebugString("\t");
	print_number_to_log(vertical_speed_y * deltaT );
	XPLMDebugString("\n");

	XPLMDebugString("> Divided by deltaT -- v_x, v_y, vert_speed\n");
	print_number_to_log(velocity_x / deltaT);
	XPLMDebugString("\t");
	print_number_to_log(velocity_z / deltaT);
	XPLMDebugString("\t");
	print_number_to_log(vertical_speed_y);
	XPLMDebugString("\n");

	// vertical speed y is the actual change of height, vertical speed x needs to be substracted from the level speed
	*/
	

	

	draw_info.x += velocity_x;
	draw_info.z += velocity_z;





	double height_offset = 0.4; // TODO: fix the model in blender?
	if (anim_data[0] > 4.30) { height_offset = 0.2; }
	draw_info.y = terrain_probe() + height + height_offset; // terrain probe to get ground elevation, then add the height of the object human from ground.


	// last point, jump to the first point again. 
	if (next_waypoint == route_coordinates[0].size() - 1 && dist < 0.15 && dist >= 0)
	{
		draw_info.pitch = 0; // reset the pitch

		next_waypoint = 1;
		
		height = -10;
		//height = 0;
		//anim_data[1] = 1;

		if (is_descend)
		{
			pax_count--;
		}
		else
		{
			pax_count++;
		}
		

		//if (instance) { XPLMInstanceSetPosition(instance, &draw_info, anim_data); }

		prev_dist = -1;
		draw_info.x = route_coordinates[0][1];
		draw_info.z = route_coordinates[1][1];
		 //XPLMDebugString("SGS human reset");

		status = 0;

		if (is_descend)
		{
			draw_info.x = route_coordinates[0][0];
			draw_info.z = route_coordinates[1][0];
			height = stair_height;
			anim_data[0] = 1;
			
			//XPLMDebugString(" at stair height\n");
		}
		else
		{
			//status = 0;
			draw_info.x = route_coordinates[0][1];
			draw_info.z = route_coordinates[1][1];
			height = 0;
			//XPLMDebugString(" at ground height\n");

		}



	}


	// smoothen the turns


	double amount_to_turn_to_reach = 0;
	double corrected_heading_to_reach = calculate_heading(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]) + 180;

	amount_to_turn_to_reach = fmod((corrected_heading_to_reach - draw_info.heading + 540), 360) - 180;

	
	
	draw_info.heading += amount_to_turn_to_reach * deltaT * 2.5;






	//draw_info.heading = calculate_heading(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]) + 180; // 180 offset always 










	if (draw_info.heading >= 360)
	{
		draw_info.heading -= 360;
	}

	if (draw_info.heading < 0)
	{
		draw_info.heading += 360;
	}
	// hdg between 0 and 360


	if (instance) { XPLMInstanceSetPosition(instance, &draw_info, anim_data); }

	
	//XPLMDebugString("SGS[DEBUG] -- Current human height: ");
/*
	print_number_to_log(speed);
	XPLMDebugString("\t");

	print_number_to_log(vertical_speed_y);
	XPLMDebugString("\t");

	print_number_to_log(vertical_speed);
	XPLMDebugString("\n");

	
	print_number_to_log(velocity_z);
	*/
	//print_number_to_log(height);
	//XPLMDebugString("\n");


/*
	XPLMDebugString("SGS[DEBUG] -- Current human position local x, y, z\n");
	print_number_to_log(draw_info.x);
	XPLMDebugString("\t");

	print_number_to_log(draw_info.y);
	XPLMDebugString("\t");

	print_number_to_log(draw_info.z);
	XPLMDebugString("\n");
	*/
}





bool Human::get_init_status()
{
	return is_initialized;
}

bool Human::get_load_status()
{
	return is_loaded;
}


// just a little checker that can be called from main. 


#include "Vehicle.h"





// the data is of the nearest bus item in the aircraft profile. Jos on yks bussi acf profiilissa, niin se on v�kisinkin kaikille portaille sama. Jos on useempi bussi, niin sitten aina l�hin. Ongelmana, 
// ett� jos on esim. molemmat portaat mutta haluaa vaan yhden bussin niin ei onnistu. Lis�ksi, kaikki menee vaan l�himm�st� bussista l�himpiin portaisiin(realismi)
bool Stairtruck_small::pax_status(double deltaT, std::vector<double> busses_x, std::vector<double> busses_z, std::vector<bool> busses_status, std::vector<Human*> other_people)
{
	srand((unsigned)time(0));


	//////// check flight time. If over 45 mins, switch to unload


	

	// do this somewhere else? Don't need to be checked every loop?






	//XPLMDebugString("Pax status called\n");
	
	bool bus_status = true;


	// temporary: requuire all assigned buses to be set?

	for (int i = 0; i < busses_status.size(); i++)
	{

		if (busses_status[i] == false)
		{
			bus_status = false;
			break;
		} 
	}
	//XPLMDebugString("Busses status: ");
	//print_number_to_log(bus_status);
	//	XPLMDebugString("\n");

	//if (bus_status == true)
	//{
	//	XPLMDebugString("Bus and human vector sizes: ");
	//	print_number_to_log((int)(busses_status.size()));
	//}

	//XPLMDebugString("\n\n\n\n");



	// resize the pax vector if not done already. But only when ready to initialize the pax
	if (Pax.empty() && target_speed == 0 && speed == 0 && is_connect == 1 && bus_status == 1)
	//if (Pax.size() != 12)
	{
		XPLMDebugString("Changing the size of pax vector \n\n");

		std::vector<std::vector<double>> custom_route;

		if (!is_global) {custom_route = get_pax_route_data(desired_stand);} // get the route distance} 
		if (custom_route.size() == 2)
		{
			XPLMDebugString("Custom route it is!! With a dist of ");

			double dist = (measure_path_distance(custom_route)) + (calculate_distance(draw_info.x, draw_info.z,  custom_route[0][custom_route[0].size()- 1], custom_route[1][custom_route[1].size()- 1]));
			//print_number_to_log(dist);
			//XPLMDebugString("\n");
			Pax.resize((int) ((dist / 5)));
			//Pax.resize(1);
		}
		else
		{
			XPLMDebugString("Nope, no custom route. Dist: ");
			int dist = (int)(calculate_distance(draw_info.x, draw_info.z, busses_x[0], busses_z[0]));


			
			Pax.resize((int) ((dist / 5) * busses_status.size()) );
			//Pax.resize(1);
		}

		// have some upper limit for the passengers, say 100. 
		if (Pax.size() > 100) {Pax.resize(100);}

	}
	/*
	else
	{
		print_number_to_log(target_speed);
		XPLMDebugString("\t");
		print_number_to_log(speed);
		XPLMDebugString("\t");
		print_number_to_log(is_connect);
		XPLMDebugString("\t");
		print_number_to_log(bus_status);
		XPLMDebugString("\t");
		print_number_to_log((int)Pax.size());
		XPLMDebugString("\n");


	}
*/
	for (int i = 0; i < Pax.size(); i++)
	{
		if (Pax[i].get_load_status() == 0)
		{
			int pax_obj_variation = rand() % 13;
			Pax[i].load_instance(pax_obj_variation); // integer as the pax number (different "variations") Load instance at startup
		}

	}

	// load them if not loaded already




	// initalization (set routes, actually draw it etc)

	if (target_speed == 0 && speed == 0 && is_connect == 1 && Pax.size() > 0 && Pax[0].get_init_status() == 0 && Pax[0].get_load_status() == 1 && bus_status == 1)
		// check if pax 0 is initialized, stairs and bus are ready. Only initialize then
	{
		/*
		XPLMDebugString("SGS -- Initializing pax with Bus position");
		print_number_to_log(bus_x);
		XPLMDebugString("\t");
		print_number_to_log(bus_z);
		XPLMDebugString("\n");
		*/

		if (!is_descend)
		{
			XPLMDataRef elapsed_time = XPLMFindDataRef("sim/time/total_flight_time_sec");
			
			if ((XPLMGetDataf(elapsed_time) - timer_reset) > (trigger_time * 60))
			{
				is_descend = 1;
			}
		
		}


		//bool is_descend = 0;
		bool is_custom_route = 0;

		// XPLMDebugString("SGS[DEBUG] -- Looking for custom pax route\n");
		std::vector<std::vector<double>> custom_route;
		if (!is_global)
		{
			custom_route = get_pax_route_data(desired_stand);
			if (custom_route.size() == 2) {is_custom_route = 1;}


		}


		

		for (size_t i = 0; i < Pax.size(); i++)
		{
			if (Pax[i].get_init_status() == 0)
			{

				int rand_bus = rand() % busses_status.size();

				//rand_bus = 1;

				double bus_x = busses_x[rand_bus];
				double bus_z = busses_z[rand_bus];


				


				//XPLMDebugString("Bus index for this lucky passenger: ");
				//print_number_to_log(rand_bus);
				//XPLMDebugString("\n");


				double rand_x_offset = rand() % 200; // +- Much more of this, like 2m
				rand_x_offset -= 100;
				rand_x_offset = rand_x_offset / 100;

				double rand_z_offset = rand() % 1000; // +- 100cm = 1m. Testing more now as there's very simple collision detect system. Pit�is olla 4m v�lit. Eli no testis 200cm
				rand_z_offset -= 500;
				rand_z_offset = rand_z_offset / 100;


				double rand_anim_pos_offset = (rand() % 175);
				rand_anim_pos_offset = rand_anim_pos_offset / 100;

				double rand_anim_pace_offset = (rand() % 20);
				rand_anim_pace_offset += 90;
				rand_anim_pace_offset = rand_anim_pace_offset / 100; // tasasella k�velyyn nopeus muutoksia, 0.90...1.10 kertanen
				
				// rand antaa kokonaisluvun, jaa 100lla -> desimaaleiks

				double bus_x_w_offset = (bus_x - (rand_x_offset / 2) * sin((draw_info.heading + 90) * (M_PI / 180)));
				double bus_z_w_offset = (bus_z + (rand_x_offset / 2) * cos((draw_info.heading + 90) * (M_PI / 180))); // for the first point, larger randomization possibility

				double pos_x_w_offset = (draw_info.x - (rand_x_offset / 5) * sin((draw_info.heading + 90) * (M_PI / 180)));
				double pos_z_w_offset = (draw_info.z + (rand_x_offset / 5) * cos((draw_info.heading + 90) * (M_PI / 180))); // getting closer to stairs, smaller the rand margin

				// for sideways offset

				std::vector<std::vector<double>> pax_route;
				pax_route.resize(2);




				/*
				[0] == people spawn point. Hidden, because "behind" the bus. For spaces between people
				[1] == Bus, point where the human becomes visible
				[2] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				[3] == stairs start
				[4] == middle platform starts
				[5] == middle platform ends
				([6] == stair part end (vertical speed to 0) disabled for now. Vertical speed to 0 and anim change when reaching wanted height
				[6] == "inside" the plane (6 -> 7) the platform on the top


				*/

				// bus position only when not using custom route for pax

				if (!is_custom_route)
				{
					pax_route[0].push_back(bus_x_w_offset - (rand_z_offset + (5 * i)) * sin((draw_info.heading) * (M_PI / 180)));  // points in relation to the stairs  []
					pax_route[1].push_back(bus_z_w_offset + (rand_z_offset + (5 * i)) * cos((draw_info.heading) * (M_PI / 180)));


					pax_route[0].push_back(bus_x_w_offset);  // bus position (to know when to show the objs again
					pax_route[1].push_back(bus_z_w_offset);

				}
				else
				{
					double start_x_w_offset = (custom_route[0][0] - rand_z_offset * sin((draw_info.heading + 90) * (M_PI / 180)));
					double start_z_w_offset = (custom_route[1][0] + rand_z_offset * cos((draw_info.heading + 90) * (M_PI / 180)));

					pax_route[0].push_back(start_x_w_offset - (rand_z_offset + (5 * i)) * sin((draw_info.heading) * (M_PI / 180)));  // points in relation to the stairs  []
					pax_route[1].push_back(start_z_w_offset + (rand_z_offset + (5 * i)) * cos((draw_info.heading) * (M_PI / 180)));
					// rand offsets for the terminal thingy as well

					// custom route, apply the randomization
					for (int i = 0; i < custom_route[0].size(); i++)
					{
						pax_route[0].push_back(custom_route[0][i] - rand_x_offset * sin((draw_info.heading + 90) * (M_PI / 180)));
						pax_route[1].push_back(custom_route[1][i] + rand_x_offset * cos((draw_info.heading + 90) * (M_PI / 180)));
					}
				}

				
			
				// if behind the wing, make people walk around the wing, and custom route(cuz buses should be properly positioned)
				if (is_behind_wing(assigned_acf_vehicle_name, vehicle_type) && is_custom_route)
				{
					
					XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
					XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");
					XPLMDataRef heading = XPLMFindDataRef("sim/flightmodel/position/psi");

				/*
				XPLMGetDataf(plane_x_pos_ref)
				
				// 1:
	double pX = 35;
	double pZ = 25;

	double pHdg = calculate_heading(0, 0, pX, pZ);
	double pDist = calculate_distance(0, 0, pX, pZ);

	double tempX = x - (pDist * sin((hdg - pHdg) * (M_PI / 180)));
	double tempZ = z + (pDist * cos((hdg - pHdg) * (M_PI / 180)));

	route[0].push_back(tempX);
	route[1].push_back(tempZ);

	XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), XPLMGetDataf(heading)
	
	*/
					double pX = -22;
					double pZ = -4;

					double pHdg = calculate_heading(0, 0, pX, pZ);
					double pDist = calculate_distance(0, 0, pX, pZ);

					double tempX = XPLMGetDataf(plane_x_pos_ref) - (pDist * sin((XPLMGetDataf(heading) - pHdg) * (M_PI / 180)));
					double tempZ = XPLMGetDataf(plane_z_pos_ref) + (pDist * cos((XPLMGetDataf(heading) - pHdg) * (M_PI / 180)));

					pax_route[0].push_back(tempX);  
					pax_route[1].push_back(tempZ);

				}
				
				pax_route[0].push_back(pos_x_w_offset - 8.0 * sin((draw_info.heading) * (M_PI / 180)));  // points in relation to the stairs
				pax_route[1].push_back(pos_z_w_offset + 8.0 * cos((draw_info.heading) * (M_PI / 180)));

				

				
				pax_route[0].push_back(draw_info.x - 4.15 * sin((draw_info.heading) * (M_PI / 180))); // stairs start
				pax_route[1].push_back(draw_info.z + 4.15 * cos((draw_info.heading) * (M_PI / 180)));


				
				if (profile_height > 4.7) // middle platform offset when height > 4.7 @todo: below 2.6
				{
					double platform_start_pos = -0.2375 * profile_height + 1.09125;
					double platform_end_pos = -0.24167 * profile_height + 1.6358;

					pax_route[0].push_back(draw_info.x + platform_start_pos * sin((draw_info.heading) * (M_PI / 180))); // small platform on the middle  [4]
					pax_route[1].push_back(draw_info.z - platform_start_pos * cos((draw_info.heading) * (M_PI / 180)));


					pax_route[0].push_back(draw_info.x + platform_end_pos * sin((draw_info.heading) * (M_PI / 180))); // stairs continue again   [5]
					pax_route[1].push_back(draw_info.z - platform_end_pos * cos((draw_info.heading) * (M_PI / 180)));
				}
				else
				{
					pax_route[0].push_back(draw_info.x - 0.15 * sin((draw_info.heading) * (M_PI / 180))); 
					pax_route[1].push_back(draw_info.z + 0.15 * cos((draw_info.heading) * (M_PI / 180)));


					pax_route[0].push_back(draw_info.x + 0.475 * sin((draw_info.heading) * (M_PI / 180))); 
					pax_route[1].push_back(draw_info.z - 0.475 * cos((draw_info.heading) * (M_PI / 180))); // middle platform with no offsets
				}
				
				
				


				// upper platform end when descending. (not needed when ascending as ascending stops when certain height is reached)
				if (is_descend)
				{
					double edge_dist = 1.5952 * profile_height - 3.6976;
					// 2.6 - 4.7: 1.5952 * profile_height - 3.6976;
					// < 2.6: -0.25 * profile_height + 1.1
					// > 4.7: -0.5 * profile_height + 6.15

					if (profile_height < 2.6)
					{
						edge_dist = -0.25 * profile_height + 1.1;
					}
					else if (profile_height > 4.7)
					{
						edge_dist = -0.5 * profile_height + 6.15;
					}

					//edge_dist += 0.2;

					XPLMDebugString("Edge dist: ");
					print_number_to_log(edge_dist);
					XPLMDebugString("\n");


					//pax_route[0].push_back(draw_info.x - ((edge_dist) * sin((draw_info.heading) * (M_PI / 180)))); 
					//pax_route[1].push_back(draw_info.z + ((edge_dist) * cos((draw_info.heading) * (M_PI / 180)))); // middle platform with no offsets

					pax_route[0].push_back(draw_info.x + edge_dist * sin((draw_info.heading) * (M_PI / 180))); // the door is precise so get to middle at the last two
					pax_route[1].push_back(draw_info.z - edge_dist * cos((draw_info.heading) * (M_PI / 180)));
				}




				


				pax_route[0].push_back(draw_info.x + 7.00 * sin((draw_info.heading) * (M_PI / 180))); // the door is precise so get to middle at the last two
				pax_route[1].push_back(draw_info.z - 7.00 * cos((draw_info.heading) * (M_PI / 180)));

				// dis inside the plane

					/*
					toistaseks vaan > 4.7 keissit.


					Kun profile height = 4.7, on tason alkuwpt - 0.025
					Kun profile height = 5.9, on tason alkuwpt - 0.31

					alkuwpt = 0.2375 * profile_height - 1.04125;

					Kun profile height = 4.7, on tason loppuwpt 0.50
					Kun profile height = 5.9, on tason loppuwpt 0.21

					- 0.24583 * profile_height + 1.68;

					
					
				
				*/

				XPLMDebugString("SGS[DEBUG] --  pax route:\n");

				for (int i = 0; i < pax_route[0].size(); i++)
				{
					print_number_to_log(pax_route[0][i]);
					XPLMDebugString("\t");
					print_number_to_log(pax_route[1][i]);
					XPLMDebugString("\n");
				}
					
			
					// if descending, same route but reversed
				if (is_descend)
				{
					std::reverse(pax_route[0].begin(), pax_route[0].end());
					std::reverse(pax_route[1].begin(), pax_route[1].end());

					/*
					XPLMDebugString("SGS[DEBUG] -- Reversing the pax route!!!\n");

					for (int i = 0; i < pax_route[0].size(); i++)
					{
						print_number_to_log(pax_route[0][i]);
						XPLMDebugString("\t");
						print_number_to_log(pax_route[1][i]);
						XPLMDebugString("\n");
					}
					*/
				}


				Pax[i].initialize(pax_route, rand_anim_pos_offset, rand_anim_pace_offset, is_descend); // reitti, animaatio paikka offset, animaatio nopeus offset


			}
			
		}


	}
	else if (target_speed == 0 && speed == 0 && is_connect == 1 && Pax.size() > 0 && Pax[0].get_init_status() == 1 && Pax[0].get_load_status() == 1 && bus_status == 1) // start updating the position if already initialized.
	{

		for (size_t i = 0; i < Pax.size(); i++)
		{
			/*
			double dist_next = 5;
			if (i != 0)
			{
				dist_next = calculate_distance(Pax[i].get_x(), Pax[i].get_z(), Pax[i - 1].get_x(), Pax[i - 1].get_z()); // distance to previous pax
			}
			else // if it's the first pax, then the one in front is the last index
			{
				dist_next = calculate_distance(Pax[i].get_x(), Pax[i].get_z(), Pax[Pax.size() - 1].get_x(), Pax[Pax.size() - 1].get_z()); // distance to previous pax

			}

			if (Pax.size() == 0){dist_next = -1;}
*/
			if (Pax[i].get_init_status() == 1)
			{
				Pax[i].update_human(other_people ,deltaT, anim_data[2], 999); // stair height

			}
		}

	}

}












/////////////// voi vitun copypaste







#include "Custom_stairs.h"
// practically the same thing for custom pax. Not the best to just copy paste almost identical function but eh
bool Custom_stairs::pax_status(double deltaT, std::vector<double> busses_x, std::vector<double> busses_z, std::vector<bool> busses_status, std::vector<Human*> other_people)
//bool Stairtruck_small::pax_status(double deltaT, std::vector<double> busses_x, std::vector<double> busses_z, std::vector<bool> busses_status, std::vector<Human*> other_people)

{

	
	//XPLMDebugString("SGS -- Custom stair pax status called\n");
	//XPLMDebugString("Custom stair status currently is ");
	//print_number_to_log(status);
	//XPLMDebugString("\n");




	srand((unsigned)time(0));


	bool bus_status = true;


	// temporary: requuire all assigned buses to be set?

	for (int i = 0; i < busses_status.size(); i++)
	{

	//	XPLMDebugString("Bus ");
	//	print_number_to_log(i);
	//	XPLMDebugString(" status: ");
	//	print_number_to_log(busses_status[i]);
	//	XPLMDebugString("\n");


		if (busses_status[i] == false)
		{
			bus_status = false;
			break;
		} 
	}

	// resize the pax vector if not done already
	//if (Pax.size() == 0)
	
	if (Pax.empty() && status == 1 && bus_status == 1)

	{
		XPLMDebugString("SGS -- Custom stair pax resized\n");
		std::vector<std::vector<double>> custom_route;

		if (!is_global) {custom_route = get_pax_route_data(desired_stand);} // get the route distance} 
		if (custom_route.size() == 2)
		{
			//XPLMDebugString("Custom route it is!! With a dist of ");

			double dist = (measure_path_distance(custom_route)) + (calculate_distance(pos_x, pos_z, custom_route[0][custom_route[0].size()- 1], custom_route[1][custom_route[1].size()- 1]));
			//print_number_to_log(dist);
			//XPLMDebugString("\n");
			Pax.resize((int)(dist / 6));
		}
		else
		{
			//XPLMDebugString("Nope, no custom route. Dist: ");
			int dist = (int)(calculate_distance(pos_x, pos_z, busses_x[0], busses_z[0]));
			//print_number_to_log(dist);
			//XPLMDebugString("\n");
			Pax.resize((int) ((dist / 6) * busses_status.size()));

			//Pax.resize(3);
		}

		// have some upper limit for the passengers, say 100. 
		if (Pax.size() > 100) {Pax.resize(100);}

	}

		


	

	for (int i = 0; i < Pax.size(); i++)
	{
		if (Pax[i].get_load_status() == 0)
		{

			int pax_obj_variation = rand() % 8;
			Pax[i].load_instance(pax_obj_variation); // integer as the pax number (different "variations") Load instance at startup
		}

	}

	// load them if not loaded already

	

	// initalization (set routes, actually draw it etc)
/*
	XPLMDebugString("SGS -- Custom stair status | pax 0 init status | pax 0 load status | bus status\n");
	print_number_to_log(status);
	XPLMDebugString("\t");
	//print_number_to_log(Pax.size());
	//XPLMDebugString("\t");
	print_number_to_log(Pax[0].get_init_status());
	XPLMDebugString("\t");
	print_number_to_log(Pax[0].get_load_status());
	XPLMDebugString("\t");
	print_number_to_log(bus_status);
	XPLMDebugString("\n");
*/

	if (status == 1 && Pax.size() > 0 && Pax[0].get_init_status() == 0 && Pax[0].get_load_status() == 1 && bus_status == 1)
		// check if pax 0 is initialized, stairs and bus are ready. Only initialize then
	{

		if (!is_descend)
		{
			XPLMDataRef elapsed_time = XPLMFindDataRef("sim/time/total_flight_time_sec");
			
			if ((XPLMGetDataf(elapsed_time) - timer_reset) > (trigger_time * 60))
			{
				is_descend = 1;
			}
		
		}
		
		

		

		//bool is_descend = 0;

		std::vector<std::vector<double>> custom_route;
		bool is_custom_route = 0;

		if (is_global)
		{
			//XPLMDebugString("These stairs loading passengers rn are in global mode\n");
		}
		

		if (!is_global)
		{
			//XPLMDebugString("Looking for custom route\n");
			custom_route = get_pax_route_data(desired_stand);
			if (custom_route.size() == 2) {is_custom_route = 1;}
		}
		


		if (custom_route.size() == 2) {is_custom_route = 1;}

		for (size_t i = 0; i < Pax.size(); i++)
		{

			// ascend
			if (Pax[i].get_init_status() == 0)
			{

				int rand_bus = rand() % busses_status.size();

				//rand_bus = 1;

				double bus_x = busses_x[rand_bus];
				double bus_z = busses_z[rand_bus];

				//XPLMDebugString("SGS -- Initializing pax for custom stairs with bus position\n");
				//print_number_to_log(bus_x);
				//XPLMDebugString("\t");
				//print_number_to_log(bus_z);
				//XPLMDebugString("\n");



				double srand_x_offset = rand() % 300; // +- Much more of this, like 4m
				srand_x_offset -= 150;
				srand_x_offset = srand_x_offset / 100;

				double srand_z_offset = rand() % 800; // +- 100cm = 1m. Testing more now as there's very simple collision detect system. Pit�is olla 4m v�lit. Eli no testis 200cm
				srand_z_offset -= 400;
				srand_z_offset = srand_z_offset / 100;


				double srand_anim_pos_offset = (rand() % 175);
				srand_anim_pos_offset = srand_anim_pos_offset / 100;

				double srand_anim_pace_offset = (rand() % 20);
				srand_anim_pace_offset += 90;
				srand_anim_pace_offset = srand_anim_pace_offset / 100; // tasasella k�velyyn nopeus muutoksia, 0.90...1.10 kertanen

				// rand antaa kokonaisluvun, jaa 100lla -> desimaaleiks

				double bus_x_w_offset = (bus_x - srand_x_offset * sin((hdg + 90) * (M_PI / 180)));
				double bus_z_w_offset = (bus_z + srand_z_offset * cos((hdg + 90) * (M_PI / 180))); // for the first point, larger randomization possibility

				double pos_x_w_offset = (pos_x - (srand_x_offset / 5) * sin((hdg + 90) * (M_PI / 180)));
				double pos_z_w_offset = (pos_z + (srand_z_offset / 5) * cos((hdg + 90) * (M_PI / 180))); // getting closer to stairs, smaller the rand margin

				// for sideways offset

				std::vector<std::vector<double>> pax_route;
				pax_route.resize(2);




				/*
				[0] == people spawn point. Hidden, because "behind" the bus. For spaces between people
				[1] == Bus, point where the human becomes visible
				[2] == stair direction, 8m from the stairs. To have the people at straight line to stairs
				[3] == stairs start
				[4] == "inside" the plane, back to start


				*/


				if (!is_custom_route)
				{
					pax_route[0].push_back(bus_x_w_offset - (srand_z_offset + (4 * i)) * sin((hdg) * (M_PI / 180)));  // points in relation to the stairs  []
					pax_route[1].push_back(bus_z_w_offset + (srand_z_offset + (4 * i)) * cos((hdg) * (M_PI / 180)));


					pax_route[0].push_back(bus_x_w_offset);  // bus position (to know when to show the objs again
					pax_route[1].push_back(bus_z_w_offset);

				}
				else
				{
					//XPLMDebugString("Custom stairs, custom route.\n");
					double start_x_w_offset = (custom_route[0][0] - srand_z_offset * sin((hdg + 90) * (M_PI / 180)));
					double start_z_w_offset = (custom_route[1][0] + srand_z_offset * cos((hdg + 90) * (M_PI / 180)));

					pax_route[0].push_back(start_x_w_offset - (srand_z_offset + (5 * i)) * sin((hdg) * (M_PI / 180)));  // points in relation to the stairs  []
					pax_route[1].push_back(start_z_w_offset + (srand_z_offset + (5 * i)) * cos((hdg) * (M_PI / 180)));
					// rand offsets for the terminal thingy as well

					// custom route, apply the randomization
					for (int i = 0; i < custom_route[0].size(); i++)
					{
						pax_route[0].push_back(custom_route[0][i] - srand_x_offset * sin((hdg + 90) * (M_PI / 180)));
						pax_route[1].push_back(custom_route[1][i] + srand_x_offset * cos((hdg + 90) * (M_PI / 180)));
					}
				}

					pax_route[0].push_back(pos_x_w_offset - 5.0 * sin((hdg) * (M_PI / 180)));  // points in relation to the stairs. Further away for custom route (to avoid wing)
					pax_route[1].push_back(pos_z_w_offset + 5.0 * cos((hdg) * (M_PI / 180)));
/*
				if (!is_custom_route)
				{
					pax_route[0].push_back(pos_x_w_offset - 8.0 * sin((hdg) * (M_PI / 180)));  // points in relation to the stairs. Further away for custom route (to avoid wing)
					pax_route[1].push_back(pos_z_w_offset + 8.0 * cos((hdg) * (M_PI / 180)));
				}
				else
				{
					pax_route[0].push_back(pos_x_w_offset - 15.0 * sin((hdg) * (M_PI / 180)));  // points in relation to the stairs. Further away for custom route (to avoid wing)
					pax_route[1].push_back(pos_z_w_offset + 15.0 * cos((hdg) * (M_PI / 180)));
				}

*/
				pax_route[0].push_back(pos_x); // stairs start exactly at pos
				pax_route[1].push_back(pos_z);

				if (is_descend)
				{
					pax_route[0].push_back(pos_x + ((step_amount * step_length)) * sin((hdg) * (M_PI / 180))); 
					pax_route[1].push_back(pos_z - ((step_amount * step_length)) * cos((hdg) * (M_PI / 180)));
				}


				pax_route[0].push_back(pos_x + ((step_amount * step_length) + 3) * sin((hdg) * (M_PI / 180))); // walk "inside" the plane for a bit (3 ish meters)
				pax_route[1].push_back(pos_z - ((step_amount * step_length) + 3) * cos((hdg) * (M_PI / 180)));


				/*
				XPLMDebugString("SGS[DEBUG] -- Pax route:\n");

				for (int i = 0; i < pax_route[0].size(); i++)
				{
					print_number_to_log(pax_route[0][i]);
					XPLMDebugString("\t");
					print_number_to_log(pax_route[1][i]);
					XPLMDebugString("\n");
				}
				*/

			XPLMDebugString("SGS[DEBUG] -- Pax route:\n");

					for (int i = 0; i < pax_route[0].size(); i++)
					{
						print_number_to_log(pax_route[0][i]);
						XPLMDebugString("\t");
						print_number_to_log(pax_route[1][i]);
						XPLMDebugString("\n");
					}

				if (is_descend)
				{
					std::reverse(pax_route[0].begin(), pax_route[0].end());
					std::reverse(pax_route[1].begin(), pax_route[1].end());

/*
					XPLMDebugString("SGS[DEBUG] -- Pax route reversed:\n");

					for (int i = 0; i < pax_route[0].size(); i++)
					{
						print_number_to_log(pax_route[0][i]);
						XPLMDebugString("\t");
						print_number_to_log(pax_route[1][i]);
						XPLMDebugString("\n");
					}
*/
				}

				
				

				Pax[i].initialize(pax_route, srand_anim_pos_offset, srand_anim_pace_offset, is_descend, step_height, step_length, step_amount); // reitti, animaatio paikka offset, animaatio nopeus offset
				//XPLMDebugString("SGS[DEBUG] --  pax route:\n");
/*
				for (int i = 0; i < pax_route[0].size(); i++)
				{
					print_number_to_log(pax_route[0][i]);
					XPLMDebugString("\t");
					print_number_to_log(pax_route[1][i]);
					XPLMDebugString("\n");
				}
				*/

			}

		}


	}
	else if (status == 1 && Pax.size() > 0 && Pax[0].get_init_status() == 1 && Pax[0].get_load_status() == 1 && bus_status == 1) // start updating the position if already initialized.
	{

		//XPLMDebugString("SGS -- Custom stair pax status starting to update the humans\n");


		for (size_t i = 0; i < Pax.size(); i++)
		{
			/*
			double dist_next = 5;
			if (i != 0)
			{
				dist_next = calculate_distance(Pax[i].get_x(), Pax[i].get_z(), Pax[i - 1].get_x(), Pax[i - 1].get_z()); // distance to previous pax
			}
			else // if it's the first pax, then the one in front is the last index
			{
				dist_next = calculate_distance(Pax[i].get_x(), Pax[i].get_z(), Pax[Pax.size() - 1].get_x(), Pax[Pax.size() - 1].get_z()); // distance to previous pax

			}

			if (Pax.size() == 0){dist_next = -1;}
*/

			if (Pax[i].get_init_status() == 1)
			{
				Pax[i].update_human(other_people, deltaT, step_height * step_amount, 999);

			}
		}

	}




}
