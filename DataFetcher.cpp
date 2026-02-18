#include "httplib.h"

#include <print>

std::string FetchData()
{

	std::string query = R"(
		[out:xml];
		(
		  way["highway"~"^(motorway|trunk|primary|secondary|tertiary|residential|primary_link|secondary_link|tertiary_link|service|living_street)$"](55.6400,12.4774,55.6938,12.5986);
		);
		out body;
		>;
		out skel qt;
	)";


	httplib::Client cli("http://overpass-api.de");
	auto res = cli.Post("/api/interpreter", "data="+ query, "application/x-www-form-urlencoded");

	if (res && res->status == 200) 
	{
		std::println("Data fetched successfully:");
		//std::println("{}", res->body);
		return res->body;
	} 
	else 
	{
		std::println("Failed to fetch data");
		return "";
	}
}