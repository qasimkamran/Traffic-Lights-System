#include "RoadNetworkFileParser.h"

void initialisePathGenerics(RoadTile* tile, string contents, Lane lane)
{
	string connection = contents.substr(contents.find_last_of(",") + 1, contents.length() - contents.find_last_of(","));

	string route = contents.substr(0, contents.find_last_of(","));
	stringstream stream(route);
	std::list<int> intConversionRoute;
	while (stream.good())
	{
		string substr;
		getline(stream, substr, ',');
		intConversionRoute.push_back(stoi(substr));
	}

	if (lane == LEFT)
	{
		tile->left.connection = connection;
		tile->left.route = intConversionRoute;
	}
	else
	{
;		tile->right.connection = connection;
		tile->right.route = intConversionRoute;
	}
}

bool roadTileIsAcceptable(RoadTile* roadTile)
{
	if (roadTile->name.empty())
		return false;

	if (roadTile->left.connection.empty())
		return false;
	if (roadTile->left.lane != LEFT)
		return false;
	if (roadTile->left.route.empty())
		return false;
	
	if (roadTile->right.connection.empty())
		return false;
	if (roadTile->right.lane != RIGHT)
		return false;
	if (roadTile->right.route.empty())
		return false;

	return true;
}

RoadNetworkFileParser::RoadNetworkFileParser()
{
}

RoadNetworkFileParser::~RoadNetworkFileParser()
{
}

void RoadNetworkFileParser::parseRoadTiles(string filename)
{
	ifstream inputFile(filename);
	string line;

	RoadTile currentTile;

	while (getline(inputFile, line))
	{
		line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

		if (line.empty())
		{
			printf("\nFailed to parse road tiles file - No line should start with white spaces or be empty");
			return;
		}

		string heading = line.substr(0, line.find_first_of(":"));
		string contents = line.substr(line.find_first_of(":") + 1, line.length() - line.find_first_of(":"));

		if (heading.empty() || contents.empty())
		{
			printf("\nEmpty heading or contents - please follow acceptable format guidelines");
			return;
		}

		if (!heading.compare("tile"))
		{
			currentTile.name = contents;
		}
		else if (!heading.compare("trafficLight"))
		{
			currentTile.trafficLightString = contents;
		}
		else if (!heading.compare("left"))
		{
			currentTile.left.lane = LEFT;
			initialisePathGenerics(&currentTile, contents, currentTile.left.lane);
		}
		else if (!heading.compare("right"))
		{
			currentTile.right.lane = RIGHT;
			initialisePathGenerics(&currentTile, contents, currentTile.right.lane);
		}

		if (roadTileIsAcceptable(&currentTile))
			parsedRoadTiles.push_back(currentTile);
	}
}

void RoadNetworkFileParser::parseNetworkTiles(std::string filename)
{
	return;
}

void RoadNetworkFileParser::printRoadTiles()
{
	if (parsedRoadTiles.empty())
	{
		printf("\nRoad Tiles uninitialized");
		return;
	}

	for (auto it = parsedRoadTiles.begin(); it != parsedRoadTiles.end(); ++it)
		printf("\nTile: %s \nLeft Connection: %s \nRight Connection: %s",
			(*it).name.c_str(), (*it).left.connection.c_str(), (*it).right.connection.c_str());
}

void RoadNetworkFileParser::printNetworkTiles()
{
	if (parsedNetworkTiles.empty())
	{
		printf("\nRoad Tiles uninitialized");
		return;
	}
}
