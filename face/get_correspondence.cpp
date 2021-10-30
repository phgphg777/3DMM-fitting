#include <vector>
#include "toml11\toml.hpp"


static const char* filename = "data/sfm/ibug_to_sfm_contour.toml";


std::pair< std::vector<int>, std::vector<int> > 
get_corresponding_pair(std::vector<int>& interested_landmark_id)
{
	auto root_table = toml::parse(filename);
	auto corr_table = toml::get<toml::Table>(root_table.at("landmark_mappings") );

	std::vector<int> landmark_id;
	std::vector<int> vertex_id;
	for (int id : interested_landmark_id)
	{
		auto it = corr_table.find(std::to_string(id));
		if( it != corr_table.end() ) {
			landmark_id.push_back(id);
			vertex_id.push_back(toml::get<int>(it->second));
		}
		else {
			printf("There is no vertex on the sfm-model corresponding to %d'th ibug landmark\n", id);
		}
	}

	return std::make_pair(landmark_id, vertex_id);
}
