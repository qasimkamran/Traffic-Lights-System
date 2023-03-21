#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <unordered_map>

using namespace std;

typedef enum {
	LEFT,
	RIGHT
} Lane;

typedef struct {
	Lane lane;
	list<int> route;
	string connection;
} Path;

typedef struct {
	string name;
	string trafficLightString;
	Path left;
	Path right;
} RoadTile;

typedef struct {
	string startTileName;
	Path internalPath;
} NetworkPath;

typedef struct {
	string name;
	list<string> connections;
	list<NetworkPath> paths;
} NetworkTile;

class RoadNetworkFileParser
{
public:
	RoadNetworkFileParser();
	virtual ~RoadNetworkFileParser();
	list<RoadTile> parsedRoadTiles;
	list<NetworkTile> parsedNetworkTiles;
	void parseRoadTiles(string filename);
	void parseNetworkTiles(string filename);
	void printRoadTiles();
	void printNetworkTiles();
};