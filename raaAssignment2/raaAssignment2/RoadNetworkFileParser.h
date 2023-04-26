#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <variant>
#include <unordered_map>

using namespace std;

typedef enum {
	LEFT,
	RIGHT,
	STRAIGHT,
} Lane;

typedef enum {
	X,
	T
} Junction;

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
	string name;
	Path left;
	Path right;
	Path straight;
} NetworkChild;

typedef struct {
	string name;
	Junction type;
	list<NetworkChild> children;
} NetworkTile;

Lane intToLane(int integer);

class RoadNetworkFileParser
{
public:
	RoadNetworkFileParser(string roadTileFilename, string NetworkTileFilename);
	virtual ~RoadNetworkFileParser();
	list<RoadTile> parsedRoadTiles;
	list<NetworkTile> parsedNetworkTiles;
	void parseRoadTiles(string filename);
	void parseNetworkTiles(string filename);
	void printRoadTiles();
	void printNetworkTiles();
};