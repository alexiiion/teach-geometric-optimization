#ifndef model_path_h_included
#define model_path_h_included

#ifndef MODEL_PATH
#define MODEL_PATH "../models/"
#endif
#endif

class PathHelper
{
public:
	static std::string get_folder_path(std::string file)
	{
		auto index = file.find_last_of("/\\");
		auto path = file.substr(0, index + 1);
		return path;
	}

	static std::string get_this_folder_path()
	{
		return get_folder_path(__FILE__);
	}
};