#pragma once

#include <windows.h>
#include <osg/switch>
#include <osg/AnimationPath>
#include <list>

#include "raaAnimatedFacarde.h"
#include "raaCollisionTarget.h"
#include "RoadNetworkFileParser.h"
#include "raaAnimationPointFinder.h"

// a facarde for the cars in the scene - note this also inherets from collision target to provide support for collision management

typedef std::vector<raaAnimationPointFinder>raaAnimationPointFinders;

osg::AnimationPath* createAnimationPath(raaAnimationPointFinders apfs, osg::Group* pRoadGroup);

class raaCarFacarde: public raaAnimatedFacarde, public raaCollisionTarget
{
public:
	raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed);
	virtual ~raaCarFacarde();
	void operator()(osg::Node* node, osg::NodeVisitor* nv) override;

	virtual osg::Vec3f getWorldDetectionPoint(); // from raaCollisionTarget
	virtual osg::Vec3f getWorldCollisionPoint(); // from raaCollisionTarget
	osg::MatrixTransform* getWorldRotationPoint();
	osg::MatrixTransform* getWorldScalePoint();
	osg::MatrixTransform* getWorldTranslationPoint();
	void setTileLists(std::list<RoadTile>* parsedRoadTiles, std::list<NetworkTile>* parsedNetworkTiles);
	bool trafficLightPass = 0;

protected:
	std::list<RoadTile>* parsedRoadTiles;
	std::list<NetworkTile>* parsedNetworkTiles;
	Lane currentLane;
	std::string currentTile;
	std::string nextTile;
	std::string destinationTile;
};

