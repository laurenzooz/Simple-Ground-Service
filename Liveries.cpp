
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPLMInstance.h"
#include "XPLMScenery.h"



#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <math.h>


#include "Liveries.h"
#include "Vehicle.h"
#include "Profile_reader.h"



std::vector<std::string> get_global_liveries(std::string vehicle_type)
{
	std::vector<std::string> liveries;

	namespace fs = std::filesystem;

	std::string path = "Resources/plugins/Simple Ground Service/liveries/";
	path += vehicle_type;
	auto ret = fs::create_directories(path);


	//XPLMDebugString("SGS[DEBUG] -- Scanning liveries\n");
	for (const auto& entry : fs::directory_iterator(path))
	{
		//std::cout << entry.path().string() << std::endl;

		std::string livery = entry.path().string();
		// remove the extension
		size_t extension_start_pos = livery.find(".png");


		// if .png not found, it's something else than a png and therefore not a livery (so it can be just ignored)
		if (extension_start_pos != std::string::npos)
		{
			livery.erase(extension_start_pos); // deletes the extension. disabled for now.
			size_t name_start_pos = livery.rfind('\\'); // delete the path and leave only the file name.
			liveries.push_back(livery.substr(name_start_pos + 1));
		}

	}
	//XPLMDebugString("SGS[DEBUG] -- Scanning liveries completed\n");


	/*
	// debug purposes
	XPLMDebugString("SGS[DEBUG] -- List of found liveries: \n");
	for (int i = 0; i < liveries.size(); i++)
	{
		XPLMDebugString(liveries[i].c_str());
		XPLMDebugString("\n");
	}
	*/





	return liveries;
}




std::vector<std::string> get_airport_spesific_liveries(std::string vehicle_type)
{
	std::vector<std::string> liveries;

	namespace fs = std::filesystem;


	std::string path = get_current_scenery_livery_dir(vehicle_type);
	auto ret = fs::create_directories(path);


	//XPLMDebugString("SGS[DEBUG] -- Scanning airport spesific liveries\n");
	for (const auto& entry : fs::directory_iterator(path))
	{
		//std::cout << entry.path().string() << std::endl;

		std::string livery = entry.path().string();
		// remove the extension
		size_t extension_start_pos = livery.find(".png");


		// if .png not found, it's something else than a png and therefore not a livery (so it can be just ignored)
		if (extension_start_pos != std::string::npos)
		{
			livery.erase(extension_start_pos); // deletes the extension. disabled for now.
			size_t name_start_pos = livery.rfind('/'); // delete the path and leave only the file name.
			liveries.push_back(livery.substr(name_start_pos + 1));
		}

	}
	//XPLMDebugString("SGS[DEBUG] -- Scanning liveries completed\n");

	/*

	// debug purposes
	XPLMDebugString("SGS[DEBUG] -- List of found liveries: \n");
	for (int i = 0; i < liveries.size(); i++)
	{
		XPLMDebugString(liveries[i].c_str());
		XPLMDebugString("\n");
	}

	*/




	return liveries;
}




void set_objects_for_each_livery(std::string vehicle_type, bool is_airport_spesific)
{
	//XPLMDebugString("SGS[DEBUG] -- Beginning to set objects for liveries\n");
	namespace fs = std::filesystem;


	std::string custom_obj_dir;

	if (is_airport_spesific == 0)
	{
		custom_obj_dir = "Resources/plugins/Simple Ground Service/objects/";
		custom_obj_dir += vehicle_type;
		custom_obj_dir += "/custom/";
	}
	else
	{
		custom_obj_dir = get_current_scenery_obj_dir(vehicle_type);
	}

	auto ret = fs::create_directories(custom_obj_dir);






	//XPLMDebugString("SGS[DEBUG] -- Making sure the directory is empty: \n");
	//XPLMDebugString(custom_obj_dir.c_str());
	//XPLMDebugString("\n");

	for (const auto& entry : std::filesystem::directory_iterator(custom_obj_dir))
	{
		std::filesystem::remove_all(entry.path());
	}

	// first, empty the custom object folder. But only if loading global stuff, not when dealing with airport spesific.


	//XPLMDebugString("SGS[DEBUG] -- Reading the default obj\n");
	std::string default_obj_dir = "Resources/plugins/Simple Ground Service/objects/";
	default_obj_dir += vehicle_type;
	default_obj_dir += "/default.obj";

	std::string default_obj_content = read_file_to_string(default_obj_dir);

	std::vector<std::string> liveries;
	if (is_airport_spesific == 0)
	{
		liveries = get_global_liveries(vehicle_type);
	}
	else
	{
		liveries = get_airport_spesific_liveries(vehicle_type);
	}

	for (int i = 0; i < liveries.size(); i++)
	{
		//XPLMDebugString("SGS[DEBUG] -- Setting the object\n");
		std::string new_obj_dir = custom_obj_dir + liveries[i];
		new_obj_dir += ".obj";

		std::string new_obj_content = default_obj_content;

		size_t texture_start_pos = new_obj_content.find("TEXTURE");
		size_t texture_end_pos = new_obj_content.find(".png");
		//size_t texture_end_pos = texture_start_pos + 10;


		std::string new_texture_file_name;

		// global liveries
		if (is_airport_spesific == 0)
		{
			new_texture_file_name = "TEXTURE    ../../../liveries/";
			new_texture_file_name += vehicle_type;
			new_texture_file_name += "/";
			new_texture_file_name += liveries[i];
		}
		else
		{
			new_texture_file_name = "TEXTURE    ../../liveries/";
			new_texture_file_name += vehicle_type;
			new_texture_file_name += "/";
			new_texture_file_name += liveries[i];

		}






		//XPLMDebugString("SGS[DEBUG] -- Replacing the texture name\n");

		new_obj_content.replace(texture_start_pos, texture_end_pos - texture_start_pos, new_texture_file_name);


		// find where the livery name starts and replace it

		//XPLMDebugString("SGS[DEBUG] -- Writing a new file in\n");
		//XPLMDebugString(new_obj_dir.c_str());
		//XPLMDebugString("\n");

		std::ofstream write_custom_obj_file(new_obj_dir);
		write_custom_obj_file << new_obj_content;
	}

}