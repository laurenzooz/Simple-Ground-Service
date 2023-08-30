#define _USE_MATH_DEFINES// to use pi, rad deg conversions


#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"
#include "XPLMPlugin.h"
#include "XPLMMenus.h"


#include <string>
#include <vector>
#include <memory>
#include <math.h>


#include "temp_message.h"
#include "Custom_stairs.h"
#include "Path.h"
#include "Profile_reader.h"



// initialization
void Custom_stairs::initialize(std::vector<double> custom_stair_data)
{
	XPLMDebugString("SGS -- Initializing custom stairs \n");

	// [0] = x, [1] = z, [2] = hdg, [3] = step length, [4] = step height, [5] = step amount

	double ac_x = custom_stair_data[0], ac_z = custom_stair_data[1], ac_hdg = custom_stair_data[2];

	step_length = custom_stair_data[3];
	step_height = custom_stair_data[4];
	step_amount = custom_stair_data[5];

	double hdg_to_relative_point = calculate_heading(0, 0, ac_x, ac_z);
	double dist_to_relative_point = calculate_distance(0, 0, ac_x, ac_z);



	XPLMDataRef plane_x_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_x");
	XPLMDataRef plane_z_pos_ref = XPLMFindDataRef("sim/flightmodel/position/local_z");
	XPLMDataRef heading_ref = XPLMFindDataRef("sim/flightmodel/position/psi");

	float plane_x = XPLMGetDataf(plane_x_pos_ref);
	float plane_z = XPLMGetDataf(plane_z_pos_ref);
	float plane_hdg = XPLMGetDataf(heading_ref);

	pos_x = plane_x - (dist_to_relative_point * sin((plane_hdg - hdg_to_relative_point) * (M_PI / 180)));
	pos_z = plane_z + (dist_to_relative_point * cos((plane_hdg - hdg_to_relative_point) * (M_PI / 180)));
	hdg = plane_hdg + ac_hdg;

	// get the plane position, then add the profile offsets to the pos values of these custom stairs.




	XPLMDebugString("SGS -- Custom stairs initialized\n");

/*
	XPLMDebugString("SGS[DEBUG] --custom stair x, z, hdg, step length, step height, step amount\n");
	XPLMDebugString((std::to_string(ac_x)).c_str());
	XPLMDebugString("\t");
	XPLMDebugString((std::to_string(ac_z)).c_str());
	XPLMDebugString("\t");
	XPLMDebugString((std::to_string(ac_hdg)).c_str());
	XPLMDebugString("\t");
	XPLMDebugString((std::to_string(step_length)).c_str());
	XPLMDebugString("\t");
	XPLMDebugString((std::to_string(step_height)).c_str());
	XPLMDebugString("\t");
	XPLMDebugString((std::to_string(step_amount)).c_str());
	XPLMDebugString("\n");
	*/

}




void Custom_stairs::menu_callback()
{


	//XPLMDebugString("SGS -- Custom stairs menu callback\n");

	if (status == 0)
	{
		status = 1;
		//XPLMDebugString("SGS -- Custom stairs status 1\n");

	}
	else // disconnect, destroy Pax
	{
		status = 0;

		for (int i = 0; i < Pax.size(); i++)
		{
			Pax[i].destroy();
		}


		Pax.resize(0);

	}
}

void Custom_stairs::unload()
{
	status = 0;

	for (int i = 0; i < Pax.size(); i++)
	{
		Pax[i].destroy();
	}


	Pax.resize(0);
}


std::string Custom_stairs::get_desired_stand()
{
	return desired_stand;
}

bool Custom_stairs::get_status()
{
	return status;
}

void Custom_stairs::global_mode_select(bool mode)
{
	is_global = mode;

}





// stand and route assigns
void Custom_stairs::set_desired_stand(std::string stand)
{
	XPLMDebugString("SGS -- Setting stand for custom stairs");
	desired_stand = stand;
}

bool is_custom_stair()
{
	std::string whole_file = read_file_to_string(get_acf_profile_dir());

	if (whole_file.find("custom_stair") != std::string::npos)
	{
		XPLMDebugString("Custom stairs found\n");
		return 1;
	}
	XPLMDebugString("Custom stairs NOT found\n");
	return 0;


}

// if found, this reads the data and sets it to the provided arguments
std::vector<double> get_custom_stair_data(std::string& custom_stair_name)
{

	//XPLMDebugString("SGS[DEBUG] -- Getting custom stair data\n");

	std::string whole_file = read_file_to_string(get_acf_profile_dir());
	// find where the custom stair section starts

	size_t start_pos = whole_file.find("custom_stair");

	// actual row the custom stair data is on
	std::string row = "";
	for (size_t i = start_pos; i < whole_file.size(); i++)
	{
		if (whole_file[i] == '\n') { break; }
		row += whole_file[i];

	}


	// now the actual parsing. More or less copypaste from normal profile parses

	std::string temp_name;
	double temp_x = 0, temp_z = 0, temp_hdg = 0, temp_step_length = 0, temp_step_height = 0, temp_step_amount = 0;
	

	std::vector<double> custom_stair_data;
	
	/*
	[0] = x, [1] = z, [2] = hdg, [3] = step length, [4] = step height, [5] = step amount

	*/

	//XPLMDebugString("SGS[DEBUG] -- starting actual parsing of row\n");
	//XPLMDebugString(row.c_str());
	//XPLMDebugString("\n");


	std::string bfr_str = "";
	int amount_of_separators = 0;

	for (int i = 0; i < row.size(); i++)
	{

		if (row[i] == '|')
		{
			amount_of_separators++;

			//XPLMDebugString("SGS[DEBUG] -- separator found\n");


			// each case individually
			
			
			// type is already known so only need to empty the buffer with the first one
			if (amount_of_separators == 1)
			{
				bfr_str = "";
				continue;
			}

			if (amount_of_separators == 2) // name
			{
				temp_name = bfr_str;
				bfr_str = "";
		

				continue;
			}

			else if (amount_of_separators == 3) // x
			{
				temp_x = stod(bfr_str);
				custom_stair_data.push_back(temp_x);

		

				bfr_str = "";
				continue;
			}

			else if (amount_of_separators == 4) // z
			{
				temp_z = stod(bfr_str);
				custom_stair_data.push_back(temp_z);

		

				bfr_str = "";
				continue;

			}

			else if (amount_of_separators == 5) // hdg
			{
				temp_hdg = stod(bfr_str);
				custom_stair_data.push_back(temp_hdg);

			

				bfr_str = "";
				continue;

			}

			else if (amount_of_separators == 6) // step length
			{
				temp_step_length = stod(bfr_str);
				custom_stair_data.push_back(temp_step_length);


				bfr_str = "";
				continue;

			}

			else if (amount_of_separators == 7) // step height
			{
				temp_step_height = stod(bfr_str);
				custom_stair_data.push_back(temp_step_height);


				bfr_str = "";
				continue;

			}

		}

		bfr_str += row[i];
		
	}

	//XPLMDebugString("SGS[DEBUG] -- Completed\n");
	temp_step_amount = stod(bfr_str);



	custom_stair_data.push_back(temp_step_amount);

	bfr_str = "";




	//custom_stair_data.push_back(temp_x, temp_z, temp_hdg, temp_step_length, temp_step_height, temp_step_amount);
	
	
	custom_stair_name = temp_name;


	



	// then at the end set this stuff
	return custom_stair_data;


}