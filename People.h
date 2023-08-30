#pragma once
#ifndef PEOPLE_H 
#define PEOPLE_H



/// temporary pax count for my own enjoyement



void reset_pax_count();

int get_pax_count();




class Human
{
protected:
	XPLMInstanceRef instance = NULL;
	XPLMObjectRef object = NULL;
	XPLMDrawInfo_t draw_info;

	// for terrain probe 
	XPLMProbeRef probe_ref = NULL;
	XPLMProbeResult probe_result = NULL;
	XPLMProbeInfo_t probe_info;


	std::string obj_path = "Resources/plugins/Simple Ground Service/objects/pax/pax_0.obj"; //  path for the object. Default, possible randomization when loading instance


	float terrain_probe();

	float anim_data[2];
	double anim_pace_offset = 1; // Random, lets see how this looks. Only applies when walking level, stair pace is unrealistically same for everyone (so much easier to execute this way.)
	//bool is_descend = 1; // descending or not

	double speed = 0; // scalar value
	double height = 0;
	double vertical_speed = 0;
	double pitch = 0; // for tilted stairs
	double stair_height = 3; // current stair height for the pax (pretty dumb to use this ig but whatevs)


	int next_waypoint = 1;


	bool is_initialized = 0;
	bool is_loaded = 0;

	// route coordinates the same way
	std::vector<std::vector<double>> route_coordinates;

	int status = 0; // 0 = normal walking, 1 transfer from walking to stair up, 2 = stair up, 3 = stair down

	double velocity_x = 0;
	double velocity_z = 0; // components of velocity vector


	double prev_dist = -1; // for wpt change check




	// testing for the height thingy
	double current_stair_height_change = 0; // the amount the human has climbed between the current stair and the next one its climbing to


	bool is_for_custom_stairs = 0;

	double custom_step_height = -1;
	double custom_step_length = -1;
	int custom_step_amount = -1;



public:
	Human(); // constructor

	void load_instance(int obj_variation = 0); // ICAO and default obj variation
	void initialize(std::vector<std::vector<double>>, double anim_pos_offset = 0, double anim_ace_offset = 1, bool is_descend = false, double step_height = -1, double step_length = -1, int amount_of_steps = -1);
	// offsets for the animation. Route offsets already calculated. Last three for custom stairs. If set to -1, there are no custom stairs
	
	void update_human(std::vector<Human*>, double, double stair_height = 2.20, double dist_human = 5.0); // deltaT, stair height(and default), distance to the human in front

	bool get_init_status();
	bool get_load_status();

	void destroy(); // unload the passenger


	double get_x();
	double get_z(); // pos for distance measurement
	bool is_stopped();

	double distance_to_closest(std::vector<Human*>); // distance to the closest human found in all the 


};


// copy pasted from vehicle pretty much


#endif
