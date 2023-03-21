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

void initialisePathGenerics(NetworkChild* child, string contents, Lane lane)
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
		child ->left.connection = connection;
		child->left.route = intConversionRoute;
	}
	else if (lane == RIGHT)
	{
		child->right.connection = connection;
		child ->right.route = intConversionRoute;
	}
	else
	{
		child->straight.connection = connection;
		child->straight.route = intConversionRoute;
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

bool networkChildIsAcceptable(NetworkChild* networkChild, Junction parentType)
{
	if (networkChild->name.empty())
		return false;

	int passes = 0;

	if ((!networkChild->left.connection.empty()) &&
	    (networkChild->left.lane == LEFT) &&
	    (!networkChild->left.route.empty()))
		passes++;

	if ((!networkChild->right.connection.empty()) &&
		(networkChild->right.lane == RIGHT) &&
		(!networkChild->right.route.empty()))
		passes++;

	if ((!networkChild->straight.connection.empty()) &&
		(networkChild->straight.lane == STRAIGHT) &&
		(!networkChild->straight.route.empty()))
		passes++;

	if (parentType == T)
	{
		if (passes < 2)
			return false;
		if (passes > 2)
		{
			printf("\nSomething went terribly wrong! - networkChildIsAcceptable(..., T)");
			return false;
		}
	}
	else
	{
		if (passes < 3)
			return false;
		if (passes > 3)
		{
			printf("\nSomething went terribly wrong! - networkChildIsAcceptable(..., X)");
			return false;
		}
	}

	return true;
}

bool networkTileIsAcceptable(NetworkTile* networkTile)
{
	if (networkTile->name.empty())
		return false;

	int count = networkTile->children.size();

	if (networkTile->type == T)
	{
		if (count < 3)
			return false;
		if (count > 3)
		{
			printf("\nCount: %d", count);
			printf("\nSomething went terribly wrong! - networkTileIsAcceptable(..., T)");
			return false;
		}
	}
	else
	{
		if (count < 4)
			return false;
		if (count > 4)
		{
			printf("\nCount: %d", count);
			printf("\nSomething went terribly wrong! - networkTileIsAcceptable(..., X)");
			return false;
		}
	}

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
		{
			parsedRoadTiles.push_back(currentTile);
			currentTile = {};
		}
	}
}

void RoadNetworkFileParser::parseNetworkTiles(std::string filename)
{
	ifstream inputFile(filename);
	string line;

	NetworkTile currentTile;
	NetworkChild currentChild;

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
		else if (!heading.compare("type"))
		{
			if (!contents.compare("T"))
				currentTile.type = T;
			else
				currentTile.type = X;
		}
		else if (!heading.compare("child"))
		{
			currentChild.name = contents;
		}
		else if (!heading.compare("left"))
		{
			currentChild.left.lane = LEFT;
			initialisePathGenerics(&currentChild, contents, currentChild.left.lane);
		}
		else if (!heading.compare("right"))
		{
			currentChild.right.lane = RIGHT;
			initialisePathGenerics(&currentChild, contents, currentChild.right.lane);
		}
		else if (!heading.compare("straight"))
		{
			currentChild.straight.lane = STRAIGHT;
			initialisePathGenerics(&currentChild, contents, currentChild.straight.lane);
		}

		if (networkChildIsAcceptable(&currentChild, currentTile.type))
		{
			currentTile.children.push_back(currentChild);
			currentChild = {};
		}

		if (networkTileIsAcceptable(&currentTile))
		{
			parsedNetworkTiles.push_back(currentTile);
			currentTile = {};
		}
	}
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

	for (auto it_parent = parsedNetworkTiles.begin(); it_parent != parsedNetworkTiles.end(); ++it_parent)
	{
		printf("\nTile: %s", (*it_parent).name.c_str());
		for (auto it_child = (*it_parent).children.begin(); it_child != (*it_parent).children.end(); ++it_child)
			printf("\nTile: %s \nLeft Connection: %s \nRight Connection: %s \nStraight Connection: %s",
				  (*it_child).name.c_str(), (*it_child).left.connection.c_str(), (*it_child).right.connection.c_str(), (*it_child).straight.connection.c_str());
	}
}
