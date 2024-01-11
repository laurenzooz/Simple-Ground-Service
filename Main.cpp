#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"
#include "XPLMPlugin.h"
#include "XPLMMenus.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>


#include "Vehicle.h"
#include "Path.h"
#include "Profile_reader.h"
#include "Menu.h"
#include "Liveries.h"
#include "Custom_stairs.h"

#include "acfutils/assert.h"
#include "acfutils/airportdb.h"

//#include <GLFW/glfw3.h>

#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif





void set_menu_item_names(bool);
void set_closest_stand();
void set_desired_stand(std::string);

bool is_global = true;

airportdb_t airportdb;

// imgui stuff


std::shared_ptr<SGS_menu> menu;
ImFontAtlas* fontAtlas = nullptr;

static void setupImGuiFonts() {
	// bind default font
	unsigned char* pixels;
	int width, height;

	fontAtlas = new ImFontAtlas();
	fontAtlas->GetTexDataAsAlpha8(&pixels, &width, &height);

	// slightly stupid dance around the texture number due to XPLM not using GLint here.
	int texNum = 0;
	XPLMGenerateTextureNumbers(&texNum, 1);

	// upload texture.
	XPLMBindTexture2d(texNum, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
		GL_UNSIGNED_BYTE, pixels);
	fontAtlas->TexID = (void*)(uintptr_t)texNum;
}

static void deleteImGuiFonts() {
	auto t = (GLuint)(uintptr_t)fontAtlas->TexID;
	glDeleteTextures(1, &t);
}










void print_number_to_log(int n)
{
	char str[50];
	int stri;
	stri = sprintf(str, "%i", n);
	XPLMDebugString(str);
}

void print_number_to_log(bool n)
{
	if (n == 1)
	{
		XPLMDebugString("True");
	
	}
	else
	{
		XPLMDebugString("False");

	}
}

void print_number_to_log(float n)
{
	char str[50];
	int stri;
	stri = sprintf(str, "%f", n);
	XPLMDebugString(str);
}

void print_number_to_log(double n)
{
	char str[50];
	int stri;
	stri = sprintf(str, "%f", n);
	XPLMDebugString(str);
}



XPLMDataRef tire_turn_ref = NULL;
XPLMDataRef steering_angle_ref = NULL;
XPLMDataRef height_ref = NULL;
XPLMDataRef leg_extend_ref = NULL;
XPLMDataRef pax_anim_ref = NULL;

std::vector<Stairtruck_small> Stairs_small(0);
std::vector<Belt_loader> Loaders(0);
std::vector<Bus> Busses(0);

/// for pax purposes
std::vector<Bus*> assigned_buses;
std::vector<Human*> all_pax;

/// for temporary test TODO: remove
//Child linked;
//bool child_init = false;


// testis custom porras
Custom_stairs custom_stair;

std::vector<Vehicle_for_acf> acf_profile_vehicles; // global so always stays the same


std::string livery = "default"; // temp global var for the livery
// initialize the vectors, get the amount later


XPLMDataRef planezd = XPLMFindDataRef("sim/flightmodel/position/local_y");


int isReady = -1;
bool is_custom_stair_initialized = 0;


std::string old_nearest_airport;



void initialize_all_objects()

{

	for (int i = 0; i < Stairs_small.size(); i++)
		{

			Stairs_small[i].load_instance(livery);
			Stairs_small[i].initialize();
		}

		for (int i = 0; i < Loaders.size(); i++)
		{
			Loaders[i].load_instance(livery);
			Loaders[i].initialize();
		}

		for (int i = 0; i < Busses.size(); i++)
		{
			Busses[i].load_instance(livery);
			Busses[i].initialize();
		}

		// check for custom stairs
		if (is_custom_stair() == 1)
		{
			//XPLMDebugString("SGS[DEBUG] -- Setting custom stairs\n");
			std::string name_temp;
			custom_stair.initialize(get_custom_stair_data(name_temp));
			is_custom_stair_initialized = 1;
		}


		/*/// TODO: remove this temporary child testing thingy
		if (Loaders.size() > 0)
		{
			linked.load_instance(livery);
			linked.initialize(&Loaders[0]);
			child_init = true;
		}
*/

}


void destroy()
{
	for (int i = 0; i < Stairs_small.size(); i++)
	{
		Stairs_small[i].delete_object();
	}

	for (int i = 0; i < Loaders.size(); i++)
	{
		Loaders[i].delete_object();
	}


	for (int i = 0; i < Busses.size(); i++)
	{
		Busses[i].delete_object();
	}

	for (int i = 0; i < all_pax.size(); i++)
	{
		all_pax[i]->destroy();
	}

	is_custom_stair_initialized = 0;

	all_pax.resize(0);

	XPLMDebugString("SGS -- destroyed\n");

}


/// TODO: remove the console output debug code


/// only if below 3000m and closer than 50km 
XPLMDataRef altitude_agl = XPLMFindDataRef("sim/flightmodel/position/y_agl");
XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");

bool flight_phase = 0; // 0 not taken off, 1: taken off.

static float update_nearest_airport(float time_since_last_call, float time_since_last_floop, int floop_counter, void* inRefcon)
{ /// TODO: now with airport db only need one function call, the return value will be the nearest ICAO/apt id. Then compare to supported apts list
	// if match => local mode, else global mode.


	// no need to check for the airpors up high
	if (isReady == 0 && XPLMGetDataf(altitude_agl) < 1000)
	{
		std::string new_nearest_airport = get_airport_data(airportdb);

		
		if (new_nearest_airport != old_nearest_airport) { 

			// TODO: check if the new_nearest_airport is in the list of supported sceneries.

			is_global = !is_supported(new_nearest_airport);


			old_nearest_airport = ""; // if changed, need to re-initialize everything

			destroy();
			Stairs_small.resize(0);
			Loaders.resize(0);
			Busses.resize(0);

			set_objects_for_each_livery("stair_small", 1); // set the liveries
			set_objects_for_each_livery("belt_loader", 1);
			set_objects_for_each_livery("bus", 1);

			
			if (is_global) {
				Stairs_small.resize(4);
				Loaders.resize(4);
				Busses.resize(4);
			} else {
				Stairs_small.resize(get_amount_of_vehicle("stair_small"));
				Loaders.resize(get_amount_of_vehicle("belt_loader"));
				Busses.resize(get_amount_of_vehicle("bus"));
			}

			// TODO: in global mode get the proper amount

			// TODO: if not in global mode, assign as many local vehicles as possibles, but if can't do all (the amount of vehicles in acf-in scenery)
			// assign rest in global mode

				
			for (int i = 0; i < Stairs_small.size(); i++) {
				Stairs_small[i].global_mode_select(is_global);
			}

			for (int i = 0; i < Loaders.size(); i++) {
				Loaders[i].global_mode_select(is_global);
			}

			for (int i = 0; i < Busses.size(); i++) {
				Busses[i].global_mode_select(is_global);
			}

			if (is_custom_stair()) { custom_stair.global_mode_select(is_global); }
			set_closest_stand();


			set_menu_item_names(0); // set the menu items again, the amount of vehicles might change per airport (in local mode). for prepare
			set_menu_item_names(1); // for connect

			old_nearest_airport = new_nearest_airport;

			initialize_all_objects();
		}
		//else if (is_global && (flight_phase == 1 || new_nearest_airport != old_nearest_airport)) // taken off, then gotten back below 1000 aka landing in global mode
	}

	else {
		
		// when above 1000m
		destroy();
		Stairs_small.resize(0);
		Loaders.resize(0);
		Busses.resize(0);
		old_nearest_airport = "";

		if (isReady == 0) {flight_phase = 1;}
	}
	return 10;
}



void reserve_pax() // sets the all pax vector
{
	// set the pax

	all_pax.resize(0); // delete old stuff

	for (int i = 0; i < Stairs_small.size(); i++)
	{
		for (int j = 0; j < Stairs_small[i].Pax.size(); j++)
		{
			all_pax.push_back(&Stairs_small[i].Pax[j]);
		}	
	}


	

	// and set pax for custom stairs as well
	if (custom_stair.get_status() == 1)
	{
		for (int i = 0; i < custom_stair.Pax.size(); i++)
		{
			all_pax.push_back(&custom_stair.Pax[i]);
		}
	}	
}




static float vehicle_floop(float time_since_last_call, float time_since_last_floop, int floop_counter, void* inRefcon) // vehicles
{
	// XPLMDebugString("floop\n\n\n\n\n");

	if ((XPLMGetDataf(planezd) != 0) && isReady == -1 && time_since_last_call < 1) // toi pienempi ku 1 vaan temporary homma kunnes keksii jotain siistimp��. Alussa heti sen j�lkeen kun on ready niin on tosi pitk� aika
	{
		// XPLMDebugString("SGS -- gonna resad the acf config\n");
		isReady = 1;

		read_aircraft_config(acf_profile_vehicles);


		// XPLMDebugString("SGS -- gonna find the nearest airport\n");

		old_nearest_airport = "";



		// XPLMDebugString("SGS -- gonna set the objs for liveries\n");
		//set_objects_for_each_livery("stair_small");		// global
		set_objects_for_each_livery("stair_small", 1);	// airport spesific

		//set_objects_for_each_livery("belt_loader");
		set_objects_for_each_livery("belt_loader", 1);

		set_objects_for_each_livery("bus", 1);

		// XPLMDebugString("SGS -- gonna read apt dat\n");
		//read_apt_dat();
		get_airport_data(airportdb);

		// XPLMDebugString("SGS -- gonna resize the vectors\n");

		// TODO: size to match the acf profile amounts.

		if (is_global)
		{
			Stairs_small.resize(4);
			Loaders.resize(4);
			Busses.resize(4);
		} else 
		{
			Stairs_small.resize(get_amount_of_vehicle("stair_small"));
			Loaders.resize(get_amount_of_vehicle("belt_loader"));
			Busses.resize(get_amount_of_vehicle("bus"));
		}
		
		
		// XPLMDebugString("SGS -- gonna set the closest stand\n");
		set_closest_stand();

		// XPLMDebugString("SGS -- gonna set the item menu names\n");
		set_menu_item_names(0); // recheck the menu items 
		set_menu_item_names(1); // prepare and connect


		

	}



	else if (isReady == 1 && time_since_last_call < 1)
	{
		isReady = 0;
		//get_acf_profile_dir();

		XPLMDebugString("SGS -- plugin ready\n");
	}


	if (isReady == 1 && time_since_last_call < 1) {



		set_objects_for_each_livery("stair_small", 1);	// airport spesific liveries
		set_objects_for_each_livery("belt_loader", 1);
		set_objects_for_each_livery("bus", 1);

		initialize_all_objects();


		

		
	}


	if (isReady == 0 && time_since_last_call < 0.1)
	{


		std::vector<double> bus_x;
		std::vector<double> bus_z;
		std::vector<bool> bus_status;

		for (int k = 0; k < assigned_buses.size(); k++)
		{
			bus_x.push_back(assigned_buses[k]->get_x());
			bus_z.push_back(assigned_buses[k]->get_z());
			bus_status.push_back(assigned_buses[k]->get_status());

			
		}


		//double bus_x = 0, bus_z = 0; 
		//bool bus_status = 0; 

		// busses first to get its pos. For now only the last bus gets saved. In future find the nearest bus to stairs or smth idk

		for (int i = 0; i < Busses.size(); i++)
		{
			Busses[i].set_object_position(time_since_last_call, floop_counter);
			Busses[i].update_animations(time_since_last_call);
		}

		
		int bus_count = get_amount_of_type_for_acf("bus");

		
		for (int i = 0; i < Stairs_small.size(); i++)
		{
			Stairs_small[i].set_object_position(time_since_last_call, floop_counter);
			Stairs_small[i].update_animations(time_since_last_call);

			//XPLMDebugString("Looking for assigned buses\n");
/*
			// assigned buses to a vector
			for (int k = 0; k < Busses.size(); k++)
			{
				
				XPLMDebugString("Bus ");
				print_number_to_log(k);
				XPLMDebugString(" assigned acf vehicle: ");
				XPLMDebugString(Busses[k].get_assigned_acf_vehicle_name().c_str());
				XPLMDebugString("\n");

				if (Busses[k].get_assigned_acf_vehicle_name() != "$unassigned")
				{
					
				}
			}

*/

			//XPLMDebugString("Assigned bus vector set up, amount of items: ");
			//print_number_to_log((int)assigned_buses.size());
			//XPLMDebugString("");

			
			//XPLMDebugString("bus x z and status vectors done, calling pax status\n");


			



			if (is_walkable_stand(Stairs_small[i].get_desired_stand()) && !is_global) // walkable stands
			{
				//Stairs_small[i].pax_status(time_since_last_call, 0, 0, true, Stairs_small[i].Pax);
				
				if (Stairs_small[i].pax_status(time_since_last_call, {0}, {0}, {true}, all_pax))
					{
						reserve_pax();
					}
			}

			else if (Stairs_small[i].pax_status(time_since_last_call, bus_x, bus_z, bus_status, all_pax)) // bus stands
			{
				
				reserve_pax();
			}


			
		}

		

		for (int i = 0; i < Loaders.size(); i++)
		{
			Loaders[i].set_object_position(time_since_last_call, floop_counter);
			Loaders[i].update_animations(time_since_last_call);
			
		}

		// TODO: remove 
		//if (child_init) {linked.set_object_position();}

		
		if (custom_stair.get_status() == 1)
		{
			
			if (is_walkable_stand(custom_stair.get_desired_stand()) && !is_global) // walkable stands
			{
				//Stairs_small[i].pax_status(time_since_last_call, 0, 0, true, Stairs_small[i].Pax);
				
				if (custom_stair.pax_status(time_since_last_call, {0}, {0}, {true}, all_pax))
					{
						reserve_pax();
					}
			}

			else if (custom_stair.pax_status(time_since_last_call, bus_x, bus_z, bus_status, all_pax)) // bus stands
			{
				
				reserve_pax();
			}
			
			
		}
		



	}

	return -1;
}






int stand_selection = 0; // index of the selected stand for the menu


// get plane pos, go through the stands and set whicever is closest
void set_closest_stand() {

	get_airport_data(airportdb);

	XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
	XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");
	
	std::string desired_stand;
	desired_stand = find_closest_stand(XPLMGetDataf(plane_x_pos_ref), XPLMGetDataf(plane_z_pos_ref), stand_selection);


	set_desired_stand(desired_stand); 
	
}





///////// menu stuff

static void menu_open_cb(
	void* inMenuRef,
	void* inItemRef)
{
	setupImGuiFonts();
	menu = std::make_shared<SGS_menu>(fontAtlas);
	menu->SetVisible(true);
	menu->SetWindowTitle("Simple Ground Service v1.0");

}


void set_desired_stand(std::string desired_stand)
{



	// Integration with AutoDGS: check if the autodgs dref exists
	// (aka it's installed) and then set the stand via it's API (write to ramp_change dataref), and then activate

	XPLMDataRef autodgs_ramp_name = XPLMFindDataRef("AutoDGS/ramp_override_str");
	XPLMCommandRef autodgs_activate = XPLMFindCommand("AutoDGS/activate");

	if (autodgs_ramp_name && desired_stand.length() > 0 && flight_phase) { 
		// ctd with empty string. Also, check that landed

		char stand_cstr [desired_stand.length() + 1];
		strcpy(stand_cstr, desired_stand.c_str());
		XPLMSetDatab(autodgs_ramp_name, &stand_cstr, 0, desired_stand.length() + 1);

		XPLMCommandOnce(autodgs_activate);
		
	}
	

	if (is_custom_stair() == 1)
	{
		std::string name_temp;
		custom_stair.initialize(get_custom_stair_data(name_temp)); // initialize custom stairs again

		//XPLMDebugString("Custom Stairs are initialized. Gonna call for the set desired stand\n");
		custom_stair.set_desired_stand(desired_stand);			
	}

	for (int i = 0; i < Stairs_small.size(); i++)
	{
		Stairs_small[i].set_assigned_acf_vehicle_name("$unassigned"); // unassign old stuff
		Stairs_small[i].set_desired_stand(desired_stand);

		all_pax.resize(0);
	}

	for (int i = 0; i < Loaders.size(); i++)
	{
		Loaders[i].set_assigned_acf_vehicle_name("$unassigned");
		Loaders[i].set_desired_stand(desired_stand);
	}

	for (int i = 0; i < Busses.size(); i++)
	{
		Busses[i].set_assigned_acf_vehicle_name("$unassigned");
		Busses[i].set_desired_stand(desired_stand);

		assigned_buses.resize(0);
	}

	// assign airport vehicles to the aircraft (stuff defined in acfs profile)
	for (int i = 0; i < acf_profile_vehicles.size(); i++)
	{

		//std::cout << "assigning\n";

		acf_profile_vehicles[i].set_assigned_vehicle_index(-1); // unassign the old stuff

		
		std::string type = acf_profile_vehicles[i].gettype();

		if (type == "stair_small")
		{
			// TODO: check that can only assign to stands specified in airport profile (if not all) 
			for (int j = 0; j < Stairs_small.size(); j++)
			{	
				//if (Stairs_small[j].get_assigned_acf_vehicle_name() == "$unassigned" && ((get_stands_for_route(j, type).find("all") != std::string::npos || get_stands_for_route(j, type).find(desired_stand) != std::string::npos) || is_global))
				if (Stairs_small[j].get_assigned_acf_vehicle_name() == "$unassigned") // if an unassigned vehicle is found, assign it
				{
					acf_profile_vehicles[i].set_assigned_vehicle_index(j); 
					Stairs_small[j].set_assigned_acf_vehicle_name(acf_profile_vehicles[i].getname());
					break;
				}
			}
		}

		else if (type == "belt_loader")
		{
			for (int j = 0; j < Loaders.size(); j++)
			{
				//if ((Loaders[j].get_assigned_acf_vehicle_name() == "$unassigned") && ((get_stands_for_route(j, type).find("all") != std::string::npos || get_stands_for_route(j, type).find(desired_stand) != std::string::npos) || is_global))
				if ((Loaders[j].get_assigned_acf_vehicle_name() == "$unassigned"))
				{
					acf_profile_vehicles[i].set_assigned_vehicle_index(j); 
					Loaders[j].set_assigned_acf_vehicle_name(acf_profile_vehicles[i].getname());
					// assign for the acf profile as well

					break;
				}
			
			}
		}

		else if (type == "bus")
		{

			for (int j = 0; j < Busses.size(); j++)
			{
				//if ((Busses[j].get_assigned_acf_vehicle_name() == "$unassigned") && ((get_stands_for_route(j, type).find("all") != std::string::npos || get_stands_for_route(j, type).find(desired_stand) != std::string::npos) || is_global))
				if ((Busses[j].get_assigned_acf_vehicle_name() == "$unassigned"))
				{
					acf_profile_vehicles[i].set_assigned_vehicle_index(j); 
					Busses[j].set_assigned_acf_vehicle_name(acf_profile_vehicles[i].getname());
					// assign for the acf profile as well

					assigned_buses.push_back(&Busses[j]); // and keep track of the assigned stuff

					//XPLMDebugString

					break;
				}
			}
		}

	}

	
	if (!is_global) {
		get_stand_waiting_positions(desired_stand);
	}
}




char search_string[1024] = "";

SGS_menu::SGS_menu(ImFontAtlas* fontAtlas) : ImgWindow(fontAtlas) {
	Init(640, 480, 640, 640);
}




// Vaikutta perfiiin liikaa k�sitell� n�it� painalluksia monimutkasesti ton menu buildinterfacen sis�ll�; se py�ritt�� sit� looppia uudestaan ja uudestaan niin kauan kun menu on auki. 
// luodaan globaali muuttuja jossa nimet on, sitten joku funktio indeksill� jota menusta kutsutaan. Lis�ks voinee tehd� sillain, ett� jos kent�ll� ei oo tietty� vehiclee, sit� ei n�y menussa?

std::vector<std::string> menu_prepare_item_names;
std::vector<std::string> menu_connect_item_names;
void set_menu_item_names(bool is_connect)
{

	// reset to 0 first. This is only called when loading in (and maybe when chaning acf or smth?)
	if (is_connect == 0)
	{
		menu_prepare_item_names.resize(0);
	}

	else
	{
		menu_connect_item_names.resize(0);
	}

	for (int i = 0; i < acf_profile_vehicles.size(); i++)
	{
		XPLMDebugString(acf_profile_vehicles[i].getname().c_str());

		// if there is an airport vehicle assigned to the index i acf vehicle
		if (acf_profile_vehicles[i].get_assigned_vehicle_index() != -1)
		{ 
			if (is_connect == 0)
			{
				
				menu_prepare_item_names.push_back(acf_profile_vehicles[i].getname());
				menu_prepare_item_names.back() += " Prepare";

			}

			else if (is_connect == 1 && acf_profile_vehicles[i].gettype() != "bus") // no connect for bus
			//else if (is_connect == 1) // no connect for bus
			{
				menu_connect_item_names.push_back(acf_profile_vehicles[i].getname());
				menu_connect_item_names.back() += " Connect";
			}
			else
			{
				menu_connect_item_names.push_back("$DISABLED");

			}

		}
		else
		{
			if (is_connect == 0)
			{

				menu_prepare_item_names.push_back("$DISABLED");

			}

			else if (is_connect == 1) // no connect for bus
			{
				menu_connect_item_names.push_back("$DISABLED");

			}
		}


		//// tyhm� systeemi addata noi disabled jutut. Hidastaa my�s kun joutuu menussa koko ajan kattoon ett� ei n�yt� jos on disabled. 
		//// T�ll�in toistaseks, koska muuten j�rjestys on vituillaan ja nimilista ei vastaa oikeeta listaa vehicleist�.
	}

	

}


// sama homma, ei tarvi handlata menu callbackissa, vaan se kutsuu erikseen t�t�
void handle_menu_callbacks(int n, bool is_connect)
{
	// get the vehicle type and which airport vehicle index (of the type) is assigned to this acf vehicle
	std::string type = acf_profile_vehicles[n].gettype();
	int acf_profile_index = acf_profile_vehicles[n].get_assigned_vehicle_index();

	/*
	XPLMDebugString("SGS[DEBUG] -- menu callback handling -- type -- index: ");
	XPLMDebugString(type.c_str());
	print_number_to_log(acf_profile_index);
	XPLMDebugString("\n");
	*/

	// for now, unfortunately each vehicle individually. Double check its assigned, although there shouldnt be unassigned stuff in the menu
	if (type == "stair_small" && acf_profile_index != -1)
	{
		if (is_connect == 0)
		{	
			Stairs_small[acf_profile_index].menu_callback();
		}

		else
		{
			Stairs_small[acf_profile_index].menu_connect_callback();
		}
	}

	if (type == "belt_loader" && acf_profile_index != -1)
	{
		if (is_connect == 0)
		{
			Loaders[acf_profile_index].menu_callback();
		}

		else
		{
			Loaders[acf_profile_index].menu_connect_callback();
		}
	}

	if (type == "bus" && acf_profile_index != -1)
	{
		if (is_connect == 0)
		{
			Busses[acf_profile_index].menu_callback();
		}

	}
	
}




void SGS_menu::BuildInterface()
{
	for (int i = 0; i < menu_prepare_item_names.size(); i++)
	{
		if (menu_prepare_item_names[i] != "$DISABLED" && ImGui::Button(menu_prepare_item_names[i].c_str())) // prepare
		{
			handle_menu_callbacks(i, 0);
		}

	}

	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	// empty spaces
	
	for (int i = 0; i < menu_connect_item_names.size(); i++)
	{
		if (menu_connect_item_names[i] != "$DISABLED" && ImGui::Button(menu_connect_item_names[i].c_str())) // connect
		{
			handle_menu_callbacks(i, 1);
		}
	}




	// stands. Let's try the old method but without the search function first
	std::vector<std::string> stands = get_stand_name_list();

	if ( ImGui::BeginCombo("Select stand", "", ImGuiComboFlags_NoPreview)) {
		for (int i = 0; i < stands.size(); i++)
		{
			//if (stands[i].find(search_string) != std::string::npos)
			//{
				if (ImGui::Selectable(stands[i].c_str(), stand_selection == i)) {
					stand_selection = i;
					set_desired_stand(stands[i]);
				}
			//}


		}

		ImGui::EndCombo();
	}

	if ( ImGui::Button("Select nearest"))
	{
		set_closest_stand();
	}


	

	// temporary menu button for custom stair status change:

	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");



	if (is_custom_stair_initialized == 1)
	{
		
		std::string name_temp;
		get_custom_stair_data(name_temp);
		if (ImGui::Button(name_temp.c_str()))
		//if (ImGui::Button("Custom Stairs"))
		{
			// XPLMDebugString("VOI VITTU\n\n\n");
			// set_closest_stand();

			std::string name_temp; // initialize again to make sure custom stairs have the present position, then resize pax to 0 to set the routes again.
			custom_stair.initialize(get_custom_stair_data(name_temp)); 
		
			for (int i = 0; i < custom_stair.Pax.size(); i++)
			{
				custom_stair.Pax[i].destroy();
			}

			custom_stair.Pax.resize(0); // to set the routes again.
		
			custom_stair.menu_callback();
		}

	}
	




	// temporary pax count for my own enjoyement:

	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("");
	ImGui::TextUnformatted("Pax count: ");
	ImGui::TextUnformatted(std::to_string(get_pax_count()).c_str());

	if (ImGui::Button("Pax reset"))
	{

		for (int i = 0; i < all_pax.size(); i++)
		{
			all_pax[i]->destroy();
		}

		all_pax.resize(0);

		for (int i = 0; i < Stairs_small.size(); i++)
		{
			Stairs_small[i].Pax.resize(0);
		}

		if (is_custom_stair())
		{
			custom_stair.Pax.resize(0);
		}

		
		// destroy passengers, then initialize them again and set the count to 0 
		flight_phase = 0; // restart
		reset_pax_count(); // count to 0.


	}


}




PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	strcpy(outName, "Simple Ground Service");
	strcpy(outSig,	"Laurenzo.Simple Ground Service");
	strcpy(outDesc, "A freeware ground handling addon for X-Plane 11 and X-Plane 12");

	log_init(XPLMDebugString, "SGS");

	// initialize the libacfutil airport db stuff
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);			/* Get paths in posix format under X-Plane 10+ */

	char xpdir[512];
	XPLMGetSystemPath(xpdir);

	XPLMDebugString("Dir got");

	char cache_path[512];
	snprintf(cache_path, sizeof(cache_path), "%sOutput/caches/Simple-Ground-Service.cache", xpdir);
	fix_pathsep(cache_path);                        /* libacfutils requires a canonical path sep */
	airportdb_create(&airportdb, xpdir, cache_path);
	airportdb.ifr_only = B_TRUE;

	XPLMDebugString("Creating cache\n");

	if (!recreate_cache(&airportdb)) {
		XPLMDebugString("Recreating airport cache failed\n");
		return 0;
	}
	XPLMDebugString("Cache created\n");
	
	
	
	
	
	
	get_supported_airports();
	//XPLMDebugString("SGS[DEBUG] -- registering datarefs\n");
	

	// register datarefs for animation
	tire_turn_ref = XPLMRegisterDataAccessor("SGS/animations/tire_rotation",
		2, 1, // float, writable
		NULL, NULL,
		0, 0, // float, NULLs are just accessors for other datatypes if needed
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);


	steering_angle_ref = XPLMRegisterDataAccessor("SGS/animations/steering_angle",
		2, 1, // float, writable
		NULL, NULL,
		0, 0, // float, NULLs are just accessors for other datatypes if needed
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);




	height_ref = XPLMRegisterDataAccessor("SGS/animations/height",
		2, 1, // float, writable
		NULL, NULL,
		0, 0, // float, NULLs are just accessors for other datatypes if needed
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);




	leg_extend_ref = XPLMRegisterDataAccessor("SGS/animations/leg_extend",
		2, 1, // float, writable
		NULL, NULL,
		0, 0, // float, NULLs are just accessors for other datatypes if needed
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);





	//// human anim

	pax_anim_ref = XPLMRegisterDataAccessor("SimpleGroundService/animations/pax",
		2, 1, // float, writable
		NULL, NULL,
		0, 0, // float, NULLs are just accessors for other datatypes if needed
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);


	pax_anim_ref = XPLMRegisterDataAccessor("SimpleGroundService/animations/pax/hide",
		1, 1, // int, writable
		0, 0, // int, NULLs are just accessors for other datatypes if needed
		NULL, NULL, 
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL);


	///////////////////////



	int my_slot = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Simple Ground Service", NULL, 0);
	XPLMMenuID m = XPLMCreateMenu("Simple Ground Service", XPLMFindPluginsMenu(), my_slot, menu_open_cb, NULL);
	XPLMAppendMenuItem(m, "Open menu", NULL, 0);

	// set global liveries
	set_objects_for_each_livery("stair_small");
	set_objects_for_each_livery("belt_loader");
	set_objects_for_each_livery("bus");


	XPLMRegisterFlightLoopCallback(vehicle_floop, -1, NULL);
	// XPLMRegisterFlightLoopCallback(human_floop, 0.01, NULL);

	XPLMRegisterFlightLoopCallback(update_nearest_airport, 10, NULL);


	return 1;
}



PLUGIN_API void XPluginDisable(void) {
	isReady = -1;
}
PLUGIN_API int  XPluginEnable(void)  
{ 
	isReady = -1;
	XPLMDebugString("SGS --  Plugin enabled\n");


	return 1; 
}


PLUGIN_API void	XPluginStop(void)
{
	isReady = -1;

	XPLMUnregisterFlightLoopCallback(vehicle_floop, NULL);
	// XPLMUnregisterFlightLoopCallback(human_floop, NULL);
	XPLMUnregisterFlightLoopCallback(update_nearest_airport, NULL);



}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }
