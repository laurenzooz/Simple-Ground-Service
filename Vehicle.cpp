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

#include "Vehicle.h"
#include "Path.h"
#include "Liveries.h"
#include "temp_message.h"
#include "Profile_reader.h"


///// temporary. for debug cone.

double debug_x = 0, debug_z = 0, debug_x2 = 0, debug_z2 = 0, debug_x3 = 0, debug_z3 = 0;

bool is_debug = 0;



/////// Vehicle intializing, loading and creating stuff 

// load the object 
static void load_cb(const char* real_path, void* ref)
{
	//XPLMDebugString("SGS -- Loading object callback\n");
	XPLMObjectRef* dest = (XPLMObjectRef*)ref;

	if (*dest == NULL)
	{

		*dest = XPLMLoadObject(real_path);
		//XPLMDebugString("SGS -- Real path: ");
		//XPLMDebugString(real_path);
		//XPLMDebugString("\n");

	}

}


Vehicle::Vehicle()
{



	anim_data[0] = 0;		// tire rotation
	anim_data[1] = 0;		// steering angle
	anim_data[2] = 2.2;		// height
	anim_data[3] = 0;		// leg extend


	draw_info.structSize = sizeof(draw_info);
	probe_info.structSize = sizeof(probe_info);

	draw_info.heading = 0;
	draw_info.pitch = 0;
	draw_info.roll = 0;
	// initialize draw and probe info



	route_coordinates.resize(2);
	drive_up_route.resize(2);
	drive_off_route.resize(2);
	// [0][n] is x (lat) of point n, [1][n] is z (lon)




	//XPLMDebugString("SGS -- Object created\n");


}

Child::Child() // for linked vehicles
{
	draw_info.structSize = sizeof(draw_info);

	draw_info.heading = 0;
	draw_info.pitch = 0;
	draw_info.roll = 0;
}





void Vehicle::load_instance(std::string livery)
{

	bool is_airport_spesific = 0;
	if (get_livery_for_vehicle(index, vehicle_type) != "default")
	{
		livery = get_livery_for_vehicle(index, vehicle_type); // airport spesific livery
		is_airport_spesific = 1;
	}

	//XPLMDebugString("SGS -- Starting the instance loading process\n");

	if (is_airport_spesific == 0)
	{
		virtual_path = "Resources/plugins/Simple Ground Service/objects/";
		virtual_path += vehicle_type;
		virtual_path += "/";
		if (livery == "default")
		{
			virtual_path += livery;
			virtual_path += ".obj";
		}
		else
		{
			virtual_path += "custom/";
			virtual_path += livery;
			virtual_path += ".obj";
		}
	}
	else
	{
		//XPLMDebugString("SGS[DEBUG] -- Setting the custom obj path\n");
		virtual_path = get_current_scenery_obj_dir(vehicle_type);
		virtual_path += livery;
		virtual_path += ".obj";
	}



	// default object is in different path than custom ones.

	//XPLMDebugString("SGS -- Object path: ");
	//XPLMDebugString(virtual_path.c_str());
	//XPLMDebugString("\n");





	const char* datarefs[] = {
		"SGS/animations/tire_rotation",
		"SGS/animations/steering_angle",
		"SGS/animations/height",
		"SGS/animations/leg_extend",  NULL };


	if (!object)
	{
		//XPLMLookupObjects(virtual_path, 0, 0, load_cb, &object);
		object = XPLMLoadObject(virtual_path.c_str());

	}

	// error, livery not installed

	if (!object)
	{
		XPLMDebugString("SGS -- Error loading the object. Possibly a missing livery, will revert to default.\n");
		livery = "default";
		virtual_path = "Resources/plugins/Simple Ground Service/objects/";
		virtual_path += vehicle_type;
		virtual_path += "/";
		virtual_path += "default.obj";

		object = XPLMLoadObject(virtual_path.c_str());

	}

	if (object)
	{
		if (!instance)
		{
			instance = XPLMCreateInstance(object, datarefs);
			//XPLMDebugString("SGS -- Instance created\n");
		}

	}

}


// for linked vehicles
void Child::load_instance(std::string livery)
{
	/*
	bool is_airport_spesific = 0;
	if (get_livery_for_vehicle(index, vehicle_type) != "default")
	{
		livery = get_livery_for_vehicle(index, vehicle_type); // airport spesific livery
		is_airport_spesific = 1;
	}

	//XPLMDebugString("SGS -- Starting the instance loading process\n");

	if (is_airport_spesific == 0)
	{
		virtual_path = "Resources/plugins/Simple Ground Service/objects/";
		virtual_path += vehicle_type;
		virtual_path += "/";
		if (livery == "default")
		{
			virtual_path += livery;
			virtual_path += ".obj";
		}
		else
		{
			virtual_path += "custom/";
			virtual_path += livery;
			virtual_path += ".obj";
		}
	}
	else
	{
		//XPLMDebugString("SGS[DEBUG] -- Setting the custom obj path\n");
		virtual_path = get_current_scenery_obj_dir(vehicle_type);
		virtual_path += livery;
		virtual_path += ".obj";
	}
*/

	XPLMDebugString("Loading linked instance\n");

	vehicle_type = "belt_loader";

	virtual_path = "Resources/plugins/Simple Ground Service/objects/";
	virtual_path += vehicle_type;
	virtual_path += "/default.obj";

	const char* datarefs[] = { NULL };


	if (!object)
	{
		//XPLMLookupObjects(virtual_path, 0, 0, load_cb, &object);
		object = XPLMLoadObject(virtual_path.c_str());

	}

	// error, livery not installed
/*
	if (!object)
	{
		XPLMDebugString("SGS -- Error loading the object. Possibly a missing livery, will revert to default.\n");
		livery = "default";
		virtual_path = "Resources/plugins/Simple Ground Service/objects/";
		virtual_path += vehicle_type;
		virtual_path += "/";
		virtual_path += "default.obj";

		object = XPLMLoadObject(virtual_path.c_str());

	}
*/
	if (object)
	{
		if (!instance)
		{
			instance = XPLMCreateInstance(object, datarefs);
			XPLMDebugString("SGS -- Instance created\n");
		}

	}
}



void Vehicle::reload_object(std::string livery)
{
	XPLMDestroyInstance(instance);
	XPLMUnloadObject(object);
	instance = NULL;
	object = NULL;
	load_instance(livery);
}




// when instance is loaded, prepare and draw the first time

void Vehicle::initialize()
{
	XPLMDebugString("SGS -- Initializing ");
	XPLMDebugString(vehicle_type.c_str());
	XPLMDebugString(std::to_string(index).c_str());
	XPLMDebugString("\n");


	if (is_global == false) {
		get_route_info(drive_up_route, drive_off_route, index, vehicle_type); // reads the profile file and creates the base routes
	} 
	else {
		drive_up_route = global_mode_route(assigned_acf_vehicle_name, vehicle_type);
		drive_off_route = global_mode_route(assigned_acf_vehicle_name, vehicle_type);
	}
	
	

	speed = 0;
	target_speed = 0;

	draw_info.x = drive_up_route[0][0];
	draw_info.z = drive_up_route[1][0];


	if (!is_global) {draw_info.y = terrain_probe();} else {draw_info.y = -1000;} // hide (underground) if global mode and not prepared
	draw_info.heading = calculate_heading(draw_info.x, draw_info.z, drive_up_route[0][1], drive_up_route[1][1]);
	
	
	//XPLMDebugString("SGS -- Initial draw info set\n");

	if (instance) 
	{ 
		XPLMInstanceSetPosition(instance, &draw_info, anim_data); 

		if (is_debug)
		{
			load_debug_cone();
			load_debug2_cone();
			load_debug3_cone();
		}
		
		
	}
	XPLMDebugString("SGS -- Initialization done\n");

}

void Child::initialize(Vehicle* new_parent) // linked vehicle
{
	parent = new_parent; 

	XPLMDebugString("SGS -- Initializing a linked vehicle\n");
	//XPLMDebugString(vehicle_type.c_str());

	speed = 0;

	draw_info.x = parent->get_x();
	draw_info.y = parent->get_y();
	draw_info.z = parent->get_z();

	float child_anim_data;

	if (instance) 
	{ 
		XPLMInstanceSetPosition(instance, &draw_info, &child_anim_data);

	}

	XPLMDebugString("SGS -- Initialization done\n");

}


// get the terrain height
float Vehicle::terrain_probe()
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





////////// moving, turning


// vector stuff for the steering calculation

class Vec
{
public:
	double x = 0, y = 0, length = 0;

	Vec operator + (Vec);
	Vec operator - (Vec);
	// operator overload



	void set_data(double x, double y)
	{
		this->x = x;
		this->y = y;
		length = sqrt(pow(x, 2) + pow(y, 2));
	}
	


	void set_vector_length(double new_length)
	{
		// divide to unit vector first, then multiply by the new desired length

		// debug stuff
		/*
		XPLMDebugString("SGS[DEBUG] -- vector before setting length x, y, length\n");
		print_number_to_log(x);
		XPLMDebugString("\t");

		print_number_to_log(y);
		XPLMDebugString("\t");

		print_number_to_log(length);
		XPLMDebugString("\n");
		*/
		double temp_x = x;
		double temp_y = y;

		//XPLMDebugString("To be set to new length of ");// \n");
		//print_number_to_log(new_length);
		//XPLMDebugString("\n");

		/*

		XPLMDebugString("The formula for x and y are: \n");


		print_number_to_log(temp_x);
		XPLMDebugString("/");
		print_number_to_log(length);
		XPLMDebugString("*");
		print_number_to_log(new_length);
		XPLMDebugString("\n");


		print_number_to_log(temp_y);
		XPLMDebugString("/");
		print_number_to_log(length);
		XPLMDebugString("*");
		print_number_to_log(new_length);
		XPLMDebugString("\n");

		*/

		x = (temp_x / length) * new_length;
		y = (temp_y / length) * new_length;

		length = sqrt(pow(x, 2) + pow(y, 2)); // need to recalculate this

		/*

		XPLMDebugString("SGS[DEBUG] -- vector length set, its now  x, y, length\n");
		print_number_to_log(x);
		XPLMDebugString("\t");

		print_number_to_log(y);
		XPLMDebugString("\t");

		print_number_to_log(length);
		XPLMDebugString("\n");
		*/
	}
};


Vec Vec::operator + (Vec a)
{
	Vec temp;
	temp.x = x + a.x;
	temp.y = y + a.y;
	return temp;
}

Vec Vec::operator - (Vec a)
{
	Vec temp;
	temp.x = x - a.x;
	temp.y = y - a.y;
	return temp;
}


Vec create_vector(double x1, double y1, double x2, double y2)
{
	Vec vect;

	vect.x = x2 - x1;
	vect.y = y1 - y2;



	vect.length = sqrt(pow(vect.x, 2) + pow(vect.y, 2)); // need to recalculate this


	return vect;
}


double scalar_projection(Vec path, Vec pos)
{
	double scalar_projection = ((path.x * pos.x) + (path.y * pos.y)) / path.length;

	return scalar_projection;
}




void Vehicle::update_heading(float deltaT)
{


	// for the heading predictions, create a point to the front axle of the car. (So front tires are not much further and so that its not the centerpoint that follows the path)
	double front_axle_x = draw_info.x + ((wheel_base / 2) * sin((draw_info.heading * (M_PI / 180))));
	double front_axle_z = draw_info.z - ((wheel_base / 2) * cos((draw_info.heading * (M_PI / 180))));

	Vec path;
	double path_hdg;
	if (next_waypoint != 0 && next_waypoint != route_coordinates[0].size() - 1)
	{
		path = create_vector(route_coordinates[0][next_waypoint - 1], route_coordinates[1][next_waypoint - 1], route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
		
		path_hdg = calculate_heading(route_coordinates[0][next_waypoint - 1], route_coordinates[1][next_waypoint - 1], route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
	}

	else
	{
		path = create_vector(front_axle_x, front_axle_z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
		
		path_hdg = calculate_heading(front_axle_x, front_axle_z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
	}


	double di = 1.5;

	Vec prediction_global; // where the vehicle is in x (time/frames/distance)
	prediction_global.x = front_axle_x + ((velocity_x / deltaT) * di);
	prediction_global.y = front_axle_z + ((velocity_z / deltaT) * di);
	// jos jatkaa nykyist� headingia. Projektio sen mukaan.

	// kyll�h�n predictioon pit�s ottaa huomioon nyk. k��ntyminen? kai? testis

	if (steering_angle != 0)
	{
		prediction_global.x = front_axle_x + ((speed * di) * sin((draw_info.heading + ((angular_velocity))) * (M_PI / 180)));
		prediction_global.y = front_axle_z - ((speed * di) * cos((draw_info.heading + ((angular_velocity))) * (M_PI / 180)));
	}



	


	Vec prediction_from_start;


	if (next_waypoint != 0)
	{
		prediction_from_start = create_vector(route_coordinates[0][next_waypoint - 1], route_coordinates[1][next_waypoint - 1], prediction_global.x, prediction_global.y);
		
	}
	else
	{
		prediction_from_start = create_vector(front_axle_x, front_axle_z, prediction_global.x, prediction_global.y);

	}


	Vec target; // predictions position projected on the route.
	double scalar_proj = scalar_projection(path, prediction_from_start);
	// minimums
	
	
	if (scalar_proj < 5)
	{
		scalar_proj = 5;
	}
	


	float corrected_heading = calculate_heading(front_axle_x, front_axle_z, target.x, target.y);
	//float corrected_heading = calculate_heading(prediction_global.x, prediction_global.y, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
	// the direction from the prediction to the target


	/*
	if (speed < 0 && is_drive_off == 1 && next_waypoint != 1) { corrected_heading += 180; }
	// things are reversed when reversing. Also handling the case where changing from reversing to forward(when driving off the plane) to prevent turning in wrong direction
	*/


	float heading_to_waypoint = calculate_heading(front_axle_x, front_axle_z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
	double distance_to_waypoint = calculate_distance(front_axle_x, front_axle_z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);

	float angle_to_be_turned = fmod((corrected_heading - draw_info.heading + 540), 360) - 180;
	float angle_to_be_turned_to_waypoint = fmod((heading_to_waypoint - draw_info.heading + 540), 360) - 180;

	if (next_waypoint > 0)
	{
		target.x = route_coordinates[0][next_waypoint - 1] + (scalar_proj * sin((path_hdg) * (M_PI / 180)));
		target.y = route_coordinates[1][next_waypoint - 1] - (scalar_proj * cos((path_hdg) * (M_PI / 180)));
	}
	else
	{
		target.x = front_axle_x + (scalar_proj * sin((path_hdg) * (M_PI / 180)));
		target.y = front_axle_z - (scalar_proj * cos((path_hdg) * (M_PI / 180)));
	}

	
	
	
	if (( next_waypoint != 0 && scalar_proj < 0) ||
		(angle_to_be_turned_to_waypoint <= -90 || angle_to_be_turned_to_waypoint >= 90 || angle_to_be_turned <= -60 || angle_to_be_turned >= 60 || speed < 0))
	{
		target.x = route_coordinates[0][next_waypoint];
		target.y = route_coordinates[1][next_waypoint];
		//XPLMDebugString("SGS[DEBUG] -- Target set to next wpt because scalar projection is below 0\n");

	}
	

	///update temporary cones
	debug_x = target.x;
	debug_z = target.y;

	debug_x2 = prediction_global.x;
	debug_z2 = prediction_global.y;



	/*

	double path_radius = 1; // how accurately to follow.
	// on last few points accuracy is very important, other than that it's good if it roughly stays on the road

	//if (is_connect == 1 && next_waypoint == route_coordinates[0].size() - 1) // max accuracy for last wpt when connect
	if (next_waypoint == route_coordinates[0].size() - 1) // max accuracy for last wpt 
	{
		path_radius = 0.5;
	}

	*/




	// target steering angle is the 'ideal' steering angle in current situation; steering angle is the current, actual
// steering angle. It takes some time to turn the wheel to reach the needed angle. 



	///// testailua. Ei jaksa tehd� kunnon yht�l��, joten selvitet��n kokeilemalla optimi kulmanopeus.
	double target_steering_angle = 0;
	double shortest_distance = 99999;



	// if closer than the path radius, go straight.



	
	//if (offset > path_radius)// && (angle_to_be_turned_to_waypoint > - 45 || angle_to_be_turned_to_waypoint < 45))
	//{
	for (double i = -max_steering_angle; i <= max_steering_angle; i += 0.1)
	{

			



		double temp_turn_radius = wheel_base / (sin(i * (M_PI / 180)));
		double temp_angular_velocity = ((speed) / temp_turn_radius) * (180 / M_PI);
		
		double temp_x = front_axle_x + ((speed * 2) * sin((draw_info.heading + ((temp_angular_velocity))) * (M_PI / 180)));
		double temp_y = front_axle_z - ((speed * 2) * cos((draw_info.heading + ((temp_angular_velocity))) * (M_PI / 180)));
			// n�in lasketaan paikka halutun et�isyyden p��h�n haluttuun suuntaan. 


			/*
			// temporary steering and velocity vectors. Prediction is the position with given steering angle.
			Vec steer, velocity, prediction;
			// nyk positiosta steering angle verran matka speed.
			steer = create_vector(draw_info.x, draw_info.z, temp_x, temp_y);
			velocity.set_data(velocity_x, velocity_z);

			prediction = steer + velocity;

			prediction.set_vector_length(speed / 2);

			*/



		double temp_dist = calculate_distance(temp_x, temp_y, target.x, target.y);
			// muuten ehk� mahdollista ett�



		if (temp_dist < shortest_distance)
		{
			shortest_distance = temp_dist;
			target_steering_angle = i;


			// prediction_global.x = temp_x;
			// prediction_global.y = temp_y;
		}
			/*
			XPLMDebugString("SGS[DEBUG] -- steering angle // distance to target from prediction ");
			print_number_to_log(i);
			XPLMDebugString("\t");
			print_number_to_log(temp_dist);
			XPLMDebugString("\n");
			*/
			
	}
	/*
	
	double offset = calculate_distance(prediction_global.x, prediction_global.y, target.x, target.y);

	
	if (angle_to_be_turned_to_waypoint < -90 || angle_to_be_turned_to_waypoint > 90)
	{
		offset = -offset;
	}

	target_steering_angle = 3 * offset;
	*/

		/*
		if (offset < path_radius)
		{
			target_steering_angle = 0;
		}

		
		XPLMDebugString("SGS[DEBUG] -- optimal steering angle is ");
		print_number_to_log(target_steering_angle);
		XPLMDebugString("\n");
		*/


	


	
	
	if (target_steering_angle < -max_steering_angle)
	{
		target_steering_angle = -max_steering_angle;
	}
	else if (target_steering_angle > max_steering_angle)
	{
		target_steering_angle = max_steering_angle;
	}
	
	// make sure amount of steering is within limits

	if ((speed < 0 && target_speed > 0) || (speed > 0 && target_speed < 0))
	{ 
		target_steering_angle = target_steering_angle * (-1); 
	}

	// when changing from reverse to forward or vice versa, reverse the target steering angle to prepare to correct direction



	if (target_speed == 0)
	{
		target_steering_angle = 0; // straighten the wheels when stopped. //Also, go straight, if close to connecting. 
	}


	double steer_speed = 75;
	
	
	if (target_steering_angle > steering_angle)
	{
		steering_angle += deltaT * steer_speed;
	}
	else if (target_steering_angle < steering_angle)
	{
		steering_angle -= deltaT * steer_speed;
	}
	
	
	


	/*
	if ((target_steering_angle < 0 && steering_angle < target_steering_angle) || (target_steering_angle > 0 && steering_angle > target_steering_angle))
	{
		steering_angle = target_steering_angle;
	}
	// limit so the actual steering angle isn't greater than the ideal in current situation
	*/



	// steering_angle = target_steering_angle;

	

	double turn_radius = wheel_base / (sin(steering_angle * (M_PI / 180)));
	angular_velocity = (speed / turn_radius) * (180 / M_PI);

}

void Vehicle::update_speed(float deltaT)
{

	// cases of changing the target speed.
	/*
	Mietit��n. Miss� tilanteessa millanen nopeus? Alussa luonnollisesti nolla. Kiihdytys 8 m/s kun menu callback. 
	Olkoon 8ms sellane standardi. Se setataan aina jos ei muu ehto t�yty. Sen j�lkeen k�yd��n kaikki ehdot jossa se vois olla jotain muuta.

	-- tilanteet hitaimmasta nopeempaan. Niin periaatteessa hitain katotaan aina eka ja j�� voimaan, vaikka joku nopeempi my�hemmin vois t�ytty�kin.
	- target speed 0: Kun vikan waypointin kohdalla, tai kun route_coordinatesia ei setattu.
	- hyvin hidas, vaikka 0.1: Kun jarrutuset�isyys vikasta waypointista. Menn��n tosi hitaasti viimenen metri, ja pys�htyminen justiin kohalleen.
	- aika hidas, vaikka 2: kun connectataan. 
	- semmone puoli nopee, sanotaan vaikka 4ms: Jos seuraava mutka jyrkk�, ja ollaan jarrutuset�isyydell� siit�.


	
	-- spesiaalikeissit: peruutus. Katotaan vaik my�hemmi.


	
	
	*/

	double dist_dest = calculate_distance_to_destination();
	if (is_drive_off == 1 && next_waypoint <= route_coordinates[0].size() - 3)
	{
		dist_dest = calculate_distance_to_destination(true);
	}
	// distance to the 3rd last waypoint (where to start reversing from) 

	double next_amount_to_turn = 0;
	double dist_to_next_wpt = calculate_distance(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
	if (dist_to_next_wpt > prev_dist && dist_to_next_wpt < (wheel_base / (sin((max_steering_angle * 0.8) * (M_PI / 180)))) && prev_dist >= 0) // min turn radius as a threshold for tests
	{
		dist_to_next_wpt = 0;
	}
	

	if (next_waypoint < route_coordinates[0].size() - 1)
	{

		double corrected_heading = calculate_heading(route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint], route_coordinates[0][next_waypoint + 1], route_coordinates[1][next_waypoint + 1]);
		next_amount_to_turn = fmod((corrected_heading - draw_info.heading + 540), 360) - 180;
	}



	if (!(is_global && vehicle_type == "bus")){ target_speed = 8;}
	
	//else if (is_connect == 1 && (amount_to_turn_to_reach < -60 || amount_to_turn_to_reach > 60))
	

	/*
	else if (is_connect == 1 && next_waypoint == route_coordinates[0].size() - 2 && dist_to_next_wpt < 0.1 && dist_to_next_wpt >= 0)
	{
		target_speed = 0;
		
	}

	*/
	if ((dist_dest < (((pow(speed, 2) / 2)) / decel_rate) + 0.2 && dist_dest >= 0))
	{
		target_speed = 0.5;
	}
	/*
	else if (is_connect == 1 && dist_dest < 15)
	{
		target_speed = 0.5;
	}*/
	else if (is_connect == 1 && vehicle_type != "bus")
	{
		target_speed = 2;
	}
	else if ((next_amount_to_turn < -60 || next_amount_to_turn > 60))// && dist_to_next_wpt < (((pow((speed), 2) / 2)) / decel_rate)) // speed on se m��r� joka hidastetaan. vaihetaan se. En oo varma onko oikei mut temp.
	{
		// XPLMDebugString("SGS[DEBUG] -- Slowing down for a curve\n");
		target_speed = 3;
	}


	if ((route_coordinates.size() != 2)  || ( dist_dest < 0.15 && dist_dest >= 0) || (next_waypoint == route_coordinates[0].size() - 1 && dist_to_next_wpt > prev_dist && prev_dist != -1))
	{
		// XPLMDebugString("\n\n\nSTOPPING\n\n\n");
		target_speed = 0;
		speed = 0;

	}

	/*
	
	XPLMDebugString("SGS[DEBUG] -- Target speed is\n");
	print_number_to_log(target_speed);
	XPLMDebugString("\n");

	XPLMDebugString("SGS[DEBUG] -- Waypoints remaining\n");
	int rem = (route_coordinates[0].size() - 1) - next_waypoint;
	print_number_to_log(rem);
	XPLMDebugString("\n");
	
	*/

	/// reverse cases


	double amount_to_turn_to_reach = 0;
	double corrected_heading_to_reach = calculate_heading(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]); 
	
	amount_to_turn_to_reach = fmod((corrected_heading_to_reach - draw_info.heading + 540), 360) - 180;


	if (is_drive_off == 1 && next_waypoint == 0 && vehicle_type != "bus")
	{
		target_speed = -3;
		//XPLMDebugString("SGS[DEBUG] -- Reversing from the plane when driving off\n");
	}
	
	/*
	else if (is_connect == 1 && (amount_to_turn_to_reach < -60 || amount_to_turn_to_reach > 60))
	{
		target_speed = -1;
		//XPLMDebugString("SGS[DEBUG] -- Reversing to turn to connect to the plane\n");
	}
	*/
	
	else if (is_drive_off == 1 && next_waypoint >= route_coordinates[0].size() - 2 && target_speed != 0 && !(is_global && vehicle_type == "bus")) // might have set to 0 already to stop.
	{
		target_speed = -1;
		speed = -1;
		//XPLMDebugString("SGS[DEBUG] -- Reversing to parking when driving off.\n");

	}

	/*
	XPLMDebugString("SGS[DEBUG] -- Target speed and speed of");
	XPLMDebugString(vehicle_type.c_str());

	XPLMDebugString("\t");

	print_number_to_log(target_speed);
	XPLMDebugString("\t");
	print_number_to_log(speed);
	XPLMDebugString("\n");
	


	

	XPLMDebugString("SGS[DEBUG] -- amonut to turn to reach ");
	print_number_to_log(amount_to_turn_to_reach);
	XPLMDebugString("\n");

	XPLMDebugString("SGS[DEBUG] -- current heading ");
	print_number_to_log(draw_info.heading);
	XPLMDebugString("\n");

	XPLMDebugString("SGS[DEBUG] -- heading to waypoint ");
	print_number_to_log(corrected_heading_to_reach);
	XPLMDebugString("\n");

	*/




	// same acceleration rate and deceleration rate for all vehicles for now
	if (speed < target_speed)
	{
		speed += deltaT * accel_rate;
	}
	else if (speed > target_speed)
	{
		speed -= deltaT * decel_rate;
	}

	// causes inaccuracy: might not be set to exactly correct value. Let's add a condition to fix this. There would prolly be better ways but eh
	if (speed > target_speed - 0.1 && speed < target_speed + 0.1) { speed = target_speed; }





	/*
	XPLMDebugString("SGS -- complete. Speed is\n");
	print_number_to_log(speed);
	XPLMDebugString(" m/s\n");
	*/
}




// this callback updates the object position each frame, so basically draws it and makes it move etc
void Vehicle::set_object_position(float deltaT, int floop_count)
{



	//XPLMDebugString("SGS[DEBUG] -- updating position\n");

	//XPLMDebugString("SGS -- setting position\n");

	if (instance && route_coordinates[0].size() > 0)//(target_speed != 0 || speed != 0))
		// check that the instance is actually there. (Also, no need for the updates if the vehicle isn't moving and not supposed to move (targetspeed 0))
	{
		
	


		double dist = calculate_distance(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
		//double dist = calculate_distance(debug_x2, debug_z2, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);
		// keep track of the distance to the next waypoint
		//XPLMDebugString("SGS -- Distance to next waypoint calculated and saved\n");

		


		// Check if driven past the waypoint. 
		if (dist > prev_dist && dist < ((wheel_base / (sin((max_steering_angle * 0.75) * (M_PI / 180))))) && prev_dist >= 0) // min turn radius as a threshold for tests

		{
			dist = 0;
		}



		//XPLMDebugString("SGS -- Distance saved for the next update\n");



		// when within this distance of the waypoint, will switch to track the next one on the route

		double trigger_distance = 2;


		/*
		//if ((is_drive_off == 0 && is_connect == 0 && next_waypoint >= route_coordinates[0].size() - 2 && speed < 3) ||
			//is_connect == 1 && next_waypoint >= route_coordinates[0].size() - 2) // max accuracy at very last points (around last 10m)
			//is_drive_off == 1 && is_connect == 0 && next_waypoint == route_coordinates[0].size() - 4 ||
		if ((is_connect == 1 && next_waypoint >= route_coordinates[0].size() - 2) ||
			//(is_drive_off == 1 && next_waypoint >= route_coordinates[0].size() - 3) || 
			(is_drive_off == 1 && next_waypoint == 0)
			)
		{
			trigger_distance = 1;

		}

		*/

		// trigger distance dependin on the amount of the next turn
		// testiss� ekaks puolet kaaren pituudesta

		double next_amount_to_turn = 0;
	
		if (next_waypoint < route_coordinates[0].size() - 1)
		{

			double corrected_heading = calculate_heading(route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint], route_coordinates[0][next_waypoint + 1], route_coordinates[1][next_waypoint + 1]);
			next_amount_to_turn = fmod((corrected_heading - draw_info.heading + 540), 360) - 180;

		}




		/*
		// distance mittaus "error moodiin" jos liian sivussa. 
		if (next_waypoint > 0)
		{

			double amount_to_turn_to_reach = 0;
			double corrected_heading_to_reach = calculate_heading(draw_info.x, draw_info.z, route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint]);

			amount_to_turn_to_reach = fmod((corrected_heading_to_reach - draw_info.heading + 540), 360) - 180;
			
			// testiss� vaikka 45 astetta sivussa
			if (amount_to_turn_to_reach > 45 || amount_to_turn_to_reach < -45)
			{
				dist = -1;
			}
		}
		*/   // ^^ toistaseks j�� py�rim��n, mutta ideaa OVERFLY waypointille!! Tollasessa tilanteessa voi sitten l�tk�st� esim ylim waypointin johki kauemmas ett� ehtii k��nty�?

		// puolet kaaren pituudesta. Laske 55% max valuesta. (viive kun renkaat k��ntyy)
		double turn_radius = wheel_base / (sin((max_steering_angle * 0.55) * (M_PI / 180)));

		trigger_distance = (abs((next_amount_to_turn / 360)  *  2 * M_PI * turn_radius)) / 2 ;

		// lis�� aikaistava vakio
		// trigger_distance += 6; // metres
		trigger_distance += 2; // metres

		
		// testis joku random alaraja. Jottei ala se liian tarkka korjaaminen.
		if (trigger_distance < 5)
		{
			trigger_distance = 5;
		}
		


		// temporary:
		// trigger_distance = 2.5;
		

		/*
		// calculate the position if "trigger point" (for debug cone)
		if (next_waypoint > 0)
		{
			double hdg = calculate_heading(route_coordinates[0][next_waypoint], route_coordinates[1][next_waypoint],route_coordinates[0][next_waypoint - 1], route_coordinates[1][next_waypoint - 1]);


			debug_x3 = route_coordinates[0][next_waypoint] + (trigger_distance * sin((hdg) * (M_PI / 180)));
			debug_z3 = route_coordinates[1][next_waypoint] - (trigger_distance * cos((hdg) * (M_PI / 180)));


		}
		*/

		// debug 3 to the next wpt

		debug_x3 = route_coordinates[0][next_waypoint];
		debug_z3 = route_coordinates[1][next_waypoint];

		/*
		XPLMDebugString("SGS[DEBUG] -- next turn amount, trigger distance ");
		print_number_to_log(next_amount_to_turn);
		XPLMDebugString("\t");
		print_number_to_log(trigger_distance);
		XPLMDebugString("\n");
		

		if (is_connect == 1) { trigger_distance = 0; }

		*/

		if (next_waypoint != route_coordinates[0].size() - 1 && dist <= trigger_distance && dist >= 0)
		{
			// when close switch to next to waypoint.
			prev_dist = -1; // -1 meaning error; cant know the last measured distanced as it has never been measured (because the distance just changed because the destination point changed)
			next_waypoint ++;
			
			/* // debug loggins
			XPLMDebugString("SGS[DEBUG] -- Next waypoint is ");
			print_number_to_log(next_waypoint);
			XPLMDebugString("\t");
			print_number_to_log(route_coordinates[0][next_waypoint]);
			XPLMDebugString("\t");
			print_number_to_log(route_coordinates[1][next_waypoint]);
			XPLMDebugString("\n");

			
			XPLMDebugString(vehicle_type.c_str());
			XPLMDebugString(" is: ");
			print_number_to_log(next_waypoint);
			XPLMDebugString("\n");
			*/
		}
		else
		{
			prev_dist = dist;

		}






		//XPLMDebugString("SGS[DEBUG] -- Updating draw info\n");

		update_speed(deltaT);	// accelerate and decelerate as needed
		update_heading(deltaT);	// update angular velocity, for turning.
		 // animations

		// deltaT depending on fps, conversion from deg to rad
		velocity_x =  speed * ((deltaT) * (sin(draw_info.heading * (M_PI / 180))));
		velocity_z = -speed * ((deltaT) * (cos(draw_info.heading * (M_PI / 180))));
		// calculates the change of position in local x and local z


		// assign the changes
		draw_info.x += velocity_x;
		draw_info.z += velocity_z;


		if (!is_global || (is_connect && vehicle_type != "bus")) {draw_info.y = terrain_probe();} // probe to get height. If in global mode, don't unless connecting. (the height gets set == the obj gets visible when hitting prepare.)
		//XPLMDebugString("SGS[DEBUG] -- Terrain probed\n");



		// if using global mode and close to destination when driving off, hide.
		double dist_prep_point;
		if (is_global && is_drive_off)
		{
			dist_prep_point = calculate_distance(drive_up_route[0][0], drive_up_route[1][0], draw_info.x, draw_info.z);
			
			
			

		}

		if (is_global && is_drive_off && dist_prep_point < 6)
		{
			draw_info.y = -1000; // hide underground, reset status just in case.
			is_drive_off = 1;
			is_connect = 0;
		}

		/*
		if (vehicle_type == "belt_loader" && index == 0)
		{
			XPLMDebugString("height: ");
			print_number_to_log(draw_info.y);
			XPLMDebugString("\n");
		}*/






		// heading stuff
		draw_info.heading += angular_velocity * deltaT;

		if (draw_info.heading >= 360)
		{
			draw_info.heading -= 360;
		}

		if (draw_info.heading < 0)
		{
			draw_info.heading += 360;
		}
		// making sure the heading stays between 0...359

		/*
		XPLMDebugString("SGS[DEBUG] -- Current position local x, y, z\n");
		print_number_to_log(draw_info.x);
		XPLMDebugString("\t");

		print_number_to_log(draw_info.y);
		XPLMDebugString("\t");

		print_number_to_log(draw_info.z);
		XPLMDebugString("\n");

		double future_x = draw_info.x, future_z = draw_info.z;

		future_x += velocity_x * 5;
		future_z += velocity_z * 5;
		XPLMDebugString("SGS[DEBUG] -- Position prediction local in 5 frames: x, z\n");
		print_number_to_log(future_x);
		XPLMDebugString("\t");

		//	print_number_to_log(draw_info.y);
		//	XPLMDebugString("\t");

		print_number_to_log(future_z);
		XPLMDebugString("\n");

		*/




		//// temporary, update debug cone positions
		if (is_debug)
		{
			update_debug_cone();
			update_debug2_cone();
			update_debug3_cone();

		}

	}


	// testiss�: Bussin auto connectaus.


	// fix the position for the last wpt
	/*
	if (next_waypoint == route_coordinates[0].size() - 1 && target_speed == 0 && speed == 0 && vehicle_type != "Bus")
	{
		
		draw_info.x = route_coordinates[0][next_waypoint];
		draw_info.z = route_coordinates[1][next_waypoint];

		draw_info.heading = calculate_heading(route_coordinates[0][route_coordinates[0].size() - 2], route_coordinates[1][route_coordinates[0].size() - 2],
			route_coordinates[0][route_coordinates[0].size() - 1], route_coordinates[1][route_coordinates[0].size() - 1]);
		if (is_drive_off == 1) { draw_info.heading += 180; }

		steering_angle = 0;
		// XPLMDebugString("SGS[DEBUG] -- Position corrected\n");

	}
	*/
	XPLMInstanceSetPosition(instance, &draw_info, anim_data);

}

void Child::set_object_position()
{

	if (instance) 
	{ 
		

		double hdg_to_relative_point = calculate_heading(0, 0, x_offset, z_offset);
		double dist_to_relative_point = calculate_distance(0, 0, x_offset, z_offset);

		double relative_x = parent->get_x() - (dist_to_relative_point * sin((parent->get_heading() - hdg_to_relative_point) * (M_PI / 180)));
		double relative_z = parent->get_z() + (dist_to_relative_point * cos((parent->get_heading() - hdg_to_relative_point) * (M_PI / 180)));

		
		draw_info.x = relative_x;
		draw_info.z = relative_z;

		draw_info.y = parent->get_y(); // height


		// calculate the heading in accordance to the turn radius of parent
		

		double turn_radius = parent->get_turn_radius();
		double angle_offset = 0;
		
		if (turn_radius != 0)
		{
			double initial_offset = (fmod((parent -> get_heading() - draw_info.heading + 540), 360) - 180);

			XPLMDebugString("turn radius: ");
			print_number_to_log(turn_radius);
			XPLMDebugString("\n");


			XPLMDebugString("initial_offset * 10: ");
			print_number_to_log(initial_offset * 10);
			XPLMDebugString("\n");

			//TODO: make sure this is correct
			double dist = abs((initial_offset / 360.0) * 2.0 * M_PI * turn_radius);
			//double dist = abs((initial_offset / 36.0) * 2.0 * M_PI * turn_radius);
			XPLMDebugString("dist: ");
			print_number_to_log(dist);
			XPLMDebugString("\n");


			angle_offset = initial_offset * ((z_offset * 1.2) / turn_radius);

			if (angle_offset > 90) { angle_offset = 90; } else if (angle_offset < -90) { angle_offset = -90; }
		}

		
		draw_info.heading = parent->get_heading() + angle_offset;


		float child_anim_data; // TODO: replace with proper stuff, like wheel spin etc.

		XPLMInstanceSetPosition(instance, &draw_info, &child_anim_data);
	}

	
}



// menu items


void Vehicle::menu_connect_callback()
{

	if (speed == 0 && is_drive_off == 0 && is_connect == 0 && (route_coordinates[0].size() > 0 || is_global)) // can only only connect if stopped and waiting in the preparation point (aka next waypoint is last.)
	{
		set_needed_waypoints(true); // set the waypoints again. (plane might have moved since this was done the first time (aka when preparing));
		next_waypoint = 0; // the first waypoint is the correction point 15m from the plane

		// target_speed = 3;
		is_connect = 1;

		// free the reserved waiting pos
		free_waiting_pos(reserved_waiting_pos); 
		//XPLMDebugString("SGS[DEBUG] -- waiting pos ");
		//print_number_to_log(reserved_waiting_pos);
		//XPLMDebugString(" SHOULD be free now\n");

		reserved_waiting_pos = -1;

	
	}
}


void Vehicle::menu_callback()
{

	//XPLMDebugString("Menu callback of ");
	//XPLMDebugString(vehicle_type.c_str());
	//XPLMDebugString("_");
	//print_number_to_log(index);
	//XPLMDebugString("\n");


//if (vehicle_type == "bus") {XPLMDebugString("menu\n");}


	if (vehicle_type == "bus" && is_connect == 0) // bus goes straight to connect phase.
	{
		is_connect = 1;
		//XPLMDebugString("Bus is_connect changed to 1\n");
	}


	if (!is_global)
	{
		change_route_direction();

	} else if (is_global && is_drive_off == 1 && (is_connect == 0 || vehicle_type == "bus")){ // global mode. Do not start driving, only set the position so that the vehicle is visible

		// reset the route first as the plane position might have changed.
		drive_up_route  = global_mode_route(assigned_acf_vehicle_name, vehicle_type);
		drive_off_route = global_mode_route(assigned_acf_vehicle_name, vehicle_type);

		route_coordinates[0].resize(0);
		route_coordinates[1].resize(0);

		draw_info.x = drive_up_route[0][0];
		draw_info.z = drive_up_route[1][0];
		draw_info.y = terrain_probe();
/*
		XPLMDebugString("Setting y of ");
		XPLMDebugString(vehicle_type.c_str());
		XPLMDebugString("_");
		print_number_to_log(index);
		XPLMDebugString("\n");
		print_number_to_log(draw_info.y);
		XPLMDebugString("\n");

		print_number_to_log(get_y());
		XPLMDebugString("\n");
*/


		draw_info.heading = calculate_heading(draw_info.x, draw_info.z, drive_up_route[0][1], drive_up_route[1][1]);


		is_drive_off = 0;		

		//if (vehicle_type == "bus") {XPLMDebugString("so called route set\n");}


	}

	// drive off in global mode
	else if (is_global && is_connect == 1)
	{
		// reset the route so that the only point (or points, as the reversing logic requires few points) is the the beginning point (drive up point start)
		route_coordinates[0].resize(0);
		route_coordinates[1].resize(0);

		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);

		is_drive_off = 1;
		is_connect = 0;


	}

	if (is_connect == 1 && vehicle_type != "bus") // local mode also 
	{
		is_connect = 0;

	}

}



// stairiein oma callbackki jossa destroyataan pax
void Stairtruck_small::menu_callback()
{
	


	if (!is_global)
	{
		change_route_direction();

	} else if (is_global && is_connect == 0) { // global mode. Do not start driving, only set the position so that the vehicle is visible

		drive_up_route =  global_mode_route(assigned_acf_vehicle_name, vehicle_type);
		drive_off_route = global_mode_route(assigned_acf_vehicle_name, vehicle_type);

		route_coordinates[0].resize(0);
		route_coordinates[1].resize(0);

		draw_info.x = drive_up_route[0][0];
		draw_info.z = drive_up_route[1][0];
		draw_info.y = terrain_probe();
		
		draw_info.heading = calculate_heading(draw_info.x, draw_info.z, drive_up_route[0][1], drive_up_route[1][1]);


		is_drive_off = 0;

	}// drive off in global mode
	else if (is_global && is_connect == 1)
	{
		// reset the route so that the only point (or points, as the reversing logic requires few points) is the the beginning point (drive up point start)
		route_coordinates[0].resize(0);
		route_coordinates[1].resize(0);

		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);
		
		route_coordinates[0].push_back(drive_off_route[0][0]);
		route_coordinates[1].push_back(drive_off_route[1][0]);

		is_drive_off = 1;
	}

	if (is_connect == 1)
	{
		for (int i = 0; i < Pax.size(); i++)
		{
			Pax[i].destroy();
		}
		Pax.resize(0);

		is_connect = 0;
	}

}




// getters
double Vehicle::get_x()
{
	
	
	return draw_info.x;
}

double Vehicle::get_y()
{
	/*
	XPLMDebugString("Getting y of ");
	XPLMDebugString(vehicle_type.c_str());
	XPLMDebugString("_");
	print_number_to_log(index);
	XPLMDebugString("\n");
	print_number_to_log(draw_info.y);
	XPLMDebugString("\n");
*/

	return draw_info.y;
}

double Vehicle::get_z()
{
	return draw_info.z;
}

double Vehicle::get_heading()
{
	return draw_info.heading;
}

double Vehicle::get_speed()
{
	return speed;
}

double Vehicle::get_turn_radius()
{
	if (steering_angle == 0)
	{
		return 0;
	}
	return (wheel_base / (sin(steering_angle * (M_PI / 180))));
}


void Vehicle::set_child(bool does_have)
{
	is_child = does_have;
}

bool Vehicle::has_child()
{
	return is_child;
}

std::string Vehicle::get_desired_stand()
{
	return desired_stand;
}



void Vehicle::global_mode_select(bool mode)
{
	is_global = mode;
}




// stand and route assigns
void Vehicle::set_desired_stand(std::string stand)
{
	XPLMDebugString("SGS -- Setting stand ");
	XPLMDebugString(stand.c_str());
	XPLMDebugString(" for vehicle ");
	XPLMDebugString(vehicle_type.c_str());
	XPLMDebugString("_");
	print_number_to_log(index);
	XPLMDebugString("\n");

	desired_stand = stand;
}





std::string Vehicle::get_vehicle_type()
{
	return vehicle_type;
}

int Vehicle::get_index()
{
	return index;
}


void Vehicle::set_assigned_acf_vehicle_name(std::string name)
{
	assigned_acf_vehicle_name = name;
	

	// set the height for the object. For some reason I feel like this is a very weird and inefficent way of doing it but im just being too lazy and stupid to think anything else.

	int profile_index = -1;

	std::vector<Vehicle_for_acf> acf_profile_init;
	read_aircraft_config(acf_profile_init);

	for (int i = 0; i < acf_profile_init.size(); i++)
	{

		
		XPLMDebugString("SGS[DEBUG] -- Comparing ");
		XPLMDebugString(name.c_str());
		XPLMDebugString(" with ");
		XPLMDebugString(acf_profile_init[i].getname().c_str());
		XPLMDebugString("\n");
		


		if (acf_profile_init[i].getname() == name)
		{

			profile_index = i;
		//	XPLMDebugString("match\n");

			break;

		}
	}

	/*
	
	XPLMDebugString("SGS[DEBUG] -- Profile index is ");
	print_number_to_log(profile_index);
	XPLMDebugString("\n");

	*/
	
	if (profile_index != -1)
	{
		profile_height = acf_profile_init[profile_index].get_height();
		
		
		//XPLMDebugString("SGS[DEBUG] -- Profile height ");
		//print_number_to_log(profile_height);
		//XPLMDebugString("\n");
		

	}
	else
	{
		//XPLMDebugString("SGS[DEBUG] -- Oh noes, error getting the object height\n");
	}

	
	if (name != "$unassigned")
	{
		
		XPLMDebugString("SGS[DEBUG] -- Assigned ");
		XPLMDebugString(vehicle_type.c_str());
		XPLMDebugString("_");
		print_number_to_log(index);
		XPLMDebugString(" as ");
		XPLMDebugString(assigned_acf_vehicle_name.c_str());

		XPLMDebugString("\n");
		
	}
	




}



std::string Vehicle::get_assigned_acf_vehicle_name()
{
	return assigned_acf_vehicle_name;
}







///// Vehicle spesific stuff:

// stairs
 
Stairtruck_small::Stairtruck_small()
{
	index = vehicle_count;
	vehicle_count++;

	vehicle_type = "stair_small";
	//virtual_path = "Resources/plugins/Simple Ground Serstair_small.obj"; // virtual path for the object, set in library.txt

	accel_rate = 0.7;
	decel_rate = 0.5;

	wheel_base = 3.5;
	max_steering_angle = 30;


}
int Stairtruck_small::vehicle_count = 0;


void Stairtruck_small::update_animations(float deltaT)
{
	// 0: tire rotation, 1: steering angle, 2: stair height, 3: leg extend;
	anim_data[0] -= 1.25 * speed; // wheels turn // 2.51
	if (anim_data[0] >= 360)
	{
		anim_data[0] = 0;
	}

	if (anim_data[0] < 0)
	{
		anim_data[0] = 359;
	}
	// limit to 1..360

	anim_data[1] = steering_angle;


	// stair height
	if (is_connect == 1 && anim_data[2] < profile_height && calculate_distance_to_destination() < 40) { anim_data[2] += 0.125 * deltaT; }
	else if (anim_data[2] > 2.2) { anim_data[2] -= 0.125 * deltaT; }

	if (anim_data[2] < 2.2) { anim_data[2] = 2.2; }
	else if (anim_data[2] > profile_height) { anim_data[2] = profile_height; }


	// legs
	if (is_connect == 1 && is_drive_off == 0 && speed == 0 && target_speed == 0 && anim_data[3] <= 1) { anim_data[3] += 0.20 * deltaT; }
	else if (target_speed != 0 && anim_data[3] >= 0) { anim_data[3] -= 0.2 * deltaT; }

	if (anim_data[3] > 1) { anim_data[3] = 1; }
	else if (anim_data[3] < 0) { anim_data[3] = 0; }

	// testis t�ll�ne
	//if (anim_data[3] >= 0.1) { speed = 0; }

}



// deleting is vehicle-spesific
void Stairtruck_small::delete_object()
{

	// destroy pax too
	for (int i = 0; i < Pax.size(); i++)
	{
		Pax[i].destroy();
	}
	Pax.resize(0);

	XPLMDestroyInstance(instance);
	XPLMUnloadObject(object);


	//XPLMDebugString("SGS[DEBUG] -- Object unloaded\n");

	vehicle_count = 0;

	route_coordinates[0].resize(0);
	route_coordinates[1].resize(0);

	drive_up_route[0].resize(0);
	drive_up_route[1].resize(0);

	drive_off_route[0].resize(0);
	drive_off_route[1].resize(0);

	// empty all the routes. (clean table when the new one needs to be created)

	is_drive_off = 1;


}



// loaders



Belt_loader::Belt_loader()
{
	// to keep track of the amount of each object type
	index = vehicle_count;
	vehicle_count++;

	vehicle_type = "belt_loader";
	virtual_path = "sgs/belt_loader.obj"; // virtual path for the object, set in library.txt // not used

	accel_rate = 0.6;
	decel_rate = 0.4;

	wheel_base = 4.15;
	max_steering_angle = 35;

	anim_data[2] = 1.2; // initial height



}
int Belt_loader::vehicle_count = 0;


void Belt_loader::update_animations(float deltaT)
{
	///anim_data[2] = profile_height;

	anim_data[0] -= 1.25 * speed; // wheels turn // 2.51
	if (anim_data[0] >= 360)
	{
		anim_data[0] = 0;
	}

	if (anim_data[0] < 0)
	{
		anim_data[0] = 359;
	}
	// limit to 1..360

	anim_data[1] = steering_angle;


	// height
	if (is_connect == 1 && anim_data[2] < profile_height && calculate_distance_to_destination() < 40) { anim_data[2] += 0.125 * deltaT; }
	else if (anim_data[2] > 1.2) { anim_data[2] -= 0.125 * deltaT; }

	if (anim_data[2] < 1.2) { anim_data[2] = 1.2; }
	else if (anim_data[2] > profile_height) { anim_data[2] = profile_height; }
}


void Belt_loader::delete_object()
{

	XPLMDestroyInstance(instance);
	XPLMUnloadObject(object);

	vehicle_count = 0;

	route_coordinates[0].resize(0);
	route_coordinates[1].resize(0);

	drive_up_route[0].resize(0);
	drive_up_route[1].resize(0);

	drive_off_route[0].resize(0);
	drive_off_route[1].resize(0);

	is_drive_off = 1;


}


// Bus
Bus::Bus()
{
	// to keep track of the amount of each object type
	index = vehicle_count;
	vehicle_count++;

	vehicle_type = "bus";
	virtual_path = "sgs/bus_misterx_copy.obj"; // virtual path for the object, set in library.txt Not used atm afaik

	accel_rate = 0.4;
	decel_rate = 0.3;

	wheel_base = 5.4;
	max_steering_angle = 25;



}
int Bus::vehicle_count = 0;


void Bus::update_animations(float deltaT)
{
	anim_data[0] -= 0.705 * speed; // wheels turn
	if (anim_data[0] >= 360)
	{
		anim_data[0] = 0;
	}

	if (anim_data[0] < 0)
	{
		anim_data[0] = 359;
	}
	// limit to 1..360

	anim_data[1] = steering_angle;
}



// bus spesific status getter for people




bool Bus::get_status()
{
	/*
	XPLMDebugString("SGS[DEBUG] -- getting bus status: is_connect, target_speed, speed \n");
	print_number_to_log(is_connect);
	XPLMDebugString("\t");

	print_number_to_log(target_speed);
	XPLMDebugString("\t");

	print_number_to_log(speed);
	XPLMDebugString("\n");
	


	XPLMDebugString("Bus ");
	print_number_to_log(index);
	XPLMDebugString(" status: ");


*/


	if (is_connect == 1 && target_speed == 0 && speed == 0)
	{
		//XPLMDebugString("true\n");
		return 1; // ready
	}

	//XPLMDebugString("false\n");
	return 0; // not ready
}


void Bus::delete_object()
{

	XPLMDestroyInstance(instance);
	XPLMUnloadObject(object);

	vehicle_count = 0;

	route_coordinates[0].resize(0);
	route_coordinates[1].resize(0);

	drive_up_route[0].resize(0);
	drive_up_route[1].resize(0);

	drive_off_route[0].resize(0);
	drive_off_route[1].resize(0);

	is_drive_off = 1;


}





void Vehicle::load_debug_cone() 
{




	//XPLMDebugString("SGS -- Starting the instance loading process for debug cone 1\n");

	



	const char* debug_datarefs[] = {NULL };


	if (!debug_object)
	{
		debug_draw_info.structSize = sizeof(debug_draw_info);

		//XPLMLookupObjects(virtual_path, 0, 0, load_cb, &object);
		debug_object = XPLMLoadObject("sgs_debug/cone.obj");

	}



	if (debug_object)
	{
		if (!debug_instance)
		{
			debug_instance = XPLMCreateInstance(debug_object, debug_datarefs);
			//XPLMDebugString("SGS -- debug Instance created\n");

			float animz_data;
			debug_draw_info.x = 0;
			debug_draw_info.y = 0;
			debug_draw_info.z = 0;

			debug_draw_info.pitch = 0;
			debug_draw_info.heading = 0;
			debug_draw_info.roll = 0;

		

		}

	}

	

}


void Vehicle::update_debug_cone()
{

	float animz_data;
	if (debug_instance)
	{
		debug_draw_info.x = debug_x;
		debug_draw_info.y = draw_info.y;
		debug_draw_info.z = debug_z;
		XPLMInstanceSetPosition(debug_instance, &debug_draw_info, &animz_data);
	}
}




void Vehicle::load_debug2_cone()
{
	//XPLMDebugString("SGS -- Starting the instance loading process for debug cone 2\n");
	const char* debug2_datarefs[] = { NULL };

	if (!debug2_object)
	{
		debug2_draw_info.structSize = sizeof(debug2_draw_info);

		//XPLMLookupObjects(virtual_path, 0, 0, load_cb, &object);
		debug2_object = XPLMLoadObject("sgs_debug/cone_blue.obj");

	}

	if (debug2_object)
	{
		if (!debug2_instance)
		{
			debug2_instance = XPLMCreateInstance(debug2_object, debug2_datarefs);
			//XPLMDebugString("SGS -- debug 2 Instance created\n");

			float animz_data;
			debug2_draw_info.x = 0;
			debug2_draw_info.y = 0;
			debug2_draw_info.z = 0;

			debug2_draw_info.pitch = 0;
			debug2_draw_info.heading = 0;
			debug2_draw_info.roll = 0;


		}
	}
}


void Vehicle::update_debug2_cone()
{

	float animz_data;
	if (debug2_instance)
	{
		debug2_draw_info.x = debug_x2;
		debug2_draw_info.y = draw_info.y;
		debug2_draw_info.z = debug_z2;
		XPLMInstanceSetPosition(debug2_instance, &debug2_draw_info, &animz_data);
	}
}



void Vehicle::load_debug3_cone()
{
	//XPLMDebugString("SGS -- Starting the instance loading process for debug cone 3\n");
	const char* debug3_datarefs[] = { NULL };

	if (!debug3_object)
	{
		debug3_draw_info.structSize = sizeof(debug3_draw_info);

		//XPLMLookupObjects(virtual_path, 0, 0, load_cb, &object);
		debug3_object = XPLMLoadObject("sgs_debug/cone_green.obj");

	}

	if (debug3_object)
	{
		if (!debug3_instance)
		{
			debug3_instance = XPLMCreateInstance(debug3_object, debug3_datarefs);
			//XPLMDebugString("SGS -- debug 3 Instance created\n");


			float animz_data;
			debug3_draw_info.x = 0;
			debug3_draw_info.y = 0;
			debug3_draw_info.z = 0;

			debug3_draw_info.pitch = 0;
			debug3_draw_info.heading = 0;
			debug3_draw_info.roll = 0;


		}
	}
}


void Vehicle::update_debug3_cone()
{

	float animz_data;
	if (debug3_instance)
	{
		debug3_draw_info.x = debug_x3;
		debug3_draw_info.y = draw_info.y;
		debug3_draw_info.z = debug_z3;
		XPLMInstanceSetPosition(debug3_instance, &debug3_draw_info, &animz_data);
	}
}


