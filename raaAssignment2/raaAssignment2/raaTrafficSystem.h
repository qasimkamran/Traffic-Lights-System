#pragma once

#include <list>

typedef std::list<class raaCollisionTarget*> raaCollisionTargets;
typedef std::list<class TrafficLightControl*> trafficLightControllerList;

class raaTrafficSystem
{
public:
	static void start();
	static void end();
	static void addTarget(raaCollisionTarget* pTarget);
	static void removeTarget(raaCollisionTarget* pTarget);
	static const raaCollisionTargets& colliders();
	static void addTargetController(TrafficLightControl* pTarget);
	static void removeTargetController(TrafficLightControl* pTarget);
	static const trafficLightControllerList& controllers();
protected:
	static raaCollisionTargets sm_lColliders;
	static trafficLightControllerList sm_lTrafficLightControllerList;
};

