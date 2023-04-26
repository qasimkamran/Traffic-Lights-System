
#include "raaCollisionTarget.h"
#include "TrafficLightControl.h"
#include "raaTrafficSystem.h"

raaCollisionTargets raaTrafficSystem::sm_lColliders;
trafficLightControllerList raaTrafficSystem::sm_lTrafficLightControllerList;

void raaTrafficSystem::start()
{
	sm_lColliders.clear();
}

void raaTrafficSystem::end()
{
	sm_lColliders.clear();
}

void raaTrafficSystem::addTarget(raaCollisionTarget* pTarget)
{
	if (pTarget && std::find(sm_lColliders.begin(), sm_lColliders.end(), pTarget) == sm_lColliders.end()) sm_lColliders.push_back(pTarget);
}

void raaTrafficSystem::removeTarget(raaCollisionTarget* pTarget)
{
	if (pTarget && std::find(sm_lColliders.begin(), sm_lColliders.end(), pTarget) != sm_lColliders.end()) sm_lColliders.remove(pTarget);
}

const raaCollisionTargets& raaTrafficSystem::colliders()
{
	return sm_lColliders;
}

void raaTrafficSystem::addTargetController(TrafficLightControl* pTarget)
{
	if (pTarget && std::find(sm_lTrafficLightControllerList.begin(), sm_lTrafficLightControllerList.end(), pTarget) == sm_lTrafficLightControllerList.end()) sm_lTrafficLightControllerList.push_back(pTarget);
}

void raaTrafficSystem::removeTargetController(TrafficLightControl * pTarget)
{
	if (pTarget && std::find(sm_lTrafficLightControllerList.begin(), sm_lTrafficLightControllerList.end(), pTarget) != sm_lTrafficLightControllerList.end()) sm_lTrafficLightControllerList.remove(pTarget);
}

const trafficLightControllerList & raaTrafficSystem::controllers()
{
	return sm_lTrafficLightControllerList;
}
