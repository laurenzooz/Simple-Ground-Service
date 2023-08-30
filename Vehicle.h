#pragma once
#ifndef VEHICLE_H
#define VEHICLE_H

#include "People.h"

// loads the object, realpath with xplmlookupobject
static void load_cb(const char* real_path, void* ref);

/*
class Vec
{

public:

	double x = 0, y = 0, length = 0;
	Vec();
};
*/


class Vehicle
{

protected:

	int index = 0; // index of the vehicle
	int reserved_waiting_pos = -1; // for route

	bool is_global = 1; // global or local 

	bool is_child = 0; // for linked vehicles. Child is like a carriage or trailer.


	std::string vehicle_type = ""; // type of vehicle

	std::string virtual_path = ""; // virtual path for the object, set in library.txt

	// for instance and object creating/loading
	XPLMInstanceRef instance = NULL;
	XPLMObjectRef object = NULL;
	XPLMDrawInfo_t draw_info;

	// for terrain probe 
	XPLMProbeRef probe_ref = NULL;
	XPLMProbeResult probe_result = NULL;
	XPLMProbeInfo_t probe_info;

	// related to obj moving and turning etc
	double speed = 0; // actual current speed, in m/s

	double velocity_x = 0; // velocity (change of position per frame) along x axis
	double velocity_z = 0; // and along z (opengl naatit) 

	// given the deltaT, calculate the speed each frame. Member variable target speed is used 
	void update_speed(float);
	double target_speed = 0;

	double calculate_distance_to_destination(bool is_drive_off_reverse = 0);

	double accel_rate = 1.5;			// m/s^2
	double decel_rate = 2.5;			// m/s^2, both values must be > 0 
	// vehicle spesific, just some default values, can be changed in each object's constructor

	float terrain_probe();				// terrain height so the vehicle stays on the ground

	double wheel_base = 2.95;
	double max_steering_angle = 30;
	//vehicle spesific, just some default values, can be changed in each objects' constructor

	double steering_angle = 0;
	double angular_velocity = 0;

	// calculate the angular velocity and needed data to make nice turns to stay on course
	void update_heading(float);


	// route stuff
	bool is_connect = 0; // whether to connect to plane or just stay at preparation point.
	int next_waypoint = 1; // Where to head next (which index in the waypoint list) waypoint 0 is the initial position.
	double prev_dist = -1; // always save the distance to the waypoint previously measured. This way its possible to detect if already driven past the waypoint



	std::vector<std::vector<double>> drive_up_route; // menoreitti. World coordinates luettuna profiilista, converttautuu
	std::vector<std::vector<double>> drive_off_route; // paluu. --||--

	std::vector<std::vector<double>> route_coordinates; // The route the vehicle actually follows. Local coordinates

	std::string desired_stand = ""; // temporary

	// if driving up, changes to drive off and vice versa. 
	void change_route_direction();

	// sets only the needed waypoints to reach eg a spesific stand (deletes the ones that arent needed, adds a few where needed.
	void set_needed_waypoints(bool is_for_connect = 0);
	bool is_drive_off = 1; // if 0, driving up, if 1, driving off.

	// for animations
	float anim_data[10];
	double profile_height = 2; // from the acf profile

	// room for different kinds of animations depending on the vehicle type


	std::string assigned_acf_vehicle_name = "$unassigned"; // name in the aircraft profile where this vehicle is assigned to. (e.g front left stairs.)

	// bool is_ready = 0; // when stopped and connected.



	


public:
	Vehicle();
	// constructor. Plane X, plane Z, and the vector as arguments

	// Default livery by default lol. To change livery, basically just destroy the old object and create a new one, with the livery name as an argument when calling load_instance
	void load_instance(std::string livery = "default");
	void initialize();
	void reload_object(std::string);
	void set_object_position(float, int); // makes the object move. deltaT is time between frames for speed stuff, floop_count because not everything is needed to be done every frame(performance)

	void set_desired_stand(std::string);


	// data getters 
	double get_speed();
	std::string get_vehicle_type();
	int get_index();
	std::string get_desired_stand();

	// changes direction, starts moving if stopped
	void menu_connect_callback();
	void menu_callback();


	void set_assigned_acf_vehicle_name(std::string);
	std::string get_assigned_acf_vehicle_name();


	double get_x();
	double get_y();
	double get_z(); // position

	double get_heading(); 
	double get_turn_radius(); // linked vehicle needs this

	bool has_child();
	void set_child(bool does_have = true);



	

	void global_mode_select(bool); // true to switch to global mode, false to switch to local mode.
	/*
	double get_velocity_x();
	double get_velocity_z();

	double get_pos_x();
	double get_pos_z();

	bool get_status(); // connected or not
	*/





	
	/// "temporary" debug cone
	
	XPLMInstanceRef debug_instance = NULL;
	XPLMObjectRef debug_object = NULL;
	XPLMDrawInfo_t debug_draw_info;


	void load_debug_cone();
	void update_debug_cone();




	XPLMInstanceRef debug2_instance = NULL;
	XPLMObjectRef debug2_object = NULL;
	XPLMDrawInfo_t debug2_draw_info;

	
	void load_debug2_cone();
	void update_debug2_cone();

	XPLMInstanceRef debug3_instance = NULL;
	XPLMObjectRef debug3_object = NULL;
	XPLMDrawInfo_t debug3_draw_info;

	
	void load_debug3_cone();
	void update_debug3_cone();
	


	



};




// different object types:
class Stairtruck_small : public Vehicle
{
public:
	XPLMObjectRef object = NULL;
	Stairtruck_small();
	void delete_object();

	// Human pax1;

	std::vector<Human> Pax;

	void update_animations(float);// float for fps stuff, boolean: wheter some hard-on-performance stuff should be made or not. 
	// public because needs to be accessed from the flight loop, different stuff for different kinds of vehicles

	//pax_status(time_since_last_call, bus_x, bus_z, bus_status);
	//void pax_status(double, double, double, bool, std::vector<Human>); // just a checker that can be called in floop from main. Also the bus pos and status for the people. ALso the icao for now.(hope i change it)
	bool pax_status(double, std::vector<double>, std::vector<double>, std::vector<bool>, std::vector<Human*>); // just a checker that can be called in floop from main. Also the bus pos and status for the people. ALso the icao for now.(hope i change it)
	// return true when initialized

	void menu_callback(); // own menu callback, need to do some special stuff with stairs(destroy pax)

protected:
	// to keep track of amount of each oject types
	static int vehicle_count;
};



class Belt_loader : public Vehicle
{
public:
	XPLMObjectRef object = NULL;
	Belt_loader();
	void delete_object();


	void update_animations(float);// float for fps stuff, boolean: wheter some hard-on-performance stuff should be made or not. 
	// public because needs to be accessed from the flight loop, different stuff for different kinds of vehicles

protected:
	// to keep track of amount of each oject types
	static int vehicle_count;
};



class Bus : public Vehicle
{
public:
	XPLMObjectRef object = NULL;
	Bus();
	void delete_object();


	void update_animations(float);// float for fps stuff, boolean: wheter some hard-on-performance stuff should be made or not. 
	// public because needs to be accessed from the flight loop, different stuff for different kinds of vehicles


	// bus spesific stuff: status getter for people
	bool get_status();

protected:
	// to keep track of amount of each oject types
	static int vehicle_count;
};




// for linked vehicles

class Child : public Vehicle
{
protected:
	Vehicle* parent;
	double x_offset = 0;
	double z_offset = -8;

public:
	Child();
	void load_instance(std::string);
	void initialize(Vehicle*);
	void set_object_position(); // for linked vehicles

};







#endif
