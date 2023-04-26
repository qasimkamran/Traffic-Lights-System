#pragma once
#include "raaNodeCallBackFacarde.h"
#include "raaCollisionTarget.h"
#include "TrafficLightFacarde.h"
#include "raaTrafficSystem.h"
#include <list>

typedef std::list<TrafficLightFacarde*> trafficLightList;

class TrafficLightControl : public raaNodeCallbackFacarde, public raaTrafficSystem
{
public:
	TrafficLightControl(osg::Node* pPart, osg::Vec3 vTrans, float fRot, float fScale);
	virtual ~TrafficLightControl();
	void updateActiveTrafficLight();
	void operator() (osg::Node* node, osg::NodeVisitor* nv) override;
	void addTrafficLight(TrafficLightFacarde* pTrafficLight);
	void changeTrafficLight(TrafficLightFacarde* pTrafficLight);
	trafficLightList m_lTrafficLights;

protected:
	int timeCount;
	int activeTrafficLight;
};

