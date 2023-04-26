
#include <windows.h>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Material>
#include <osg/Switch>

#include "raaTrafficSystem.h"
#include "raaCarFacarde.h"
#include "raaFinder.h"
#include "TrafficLightControl.h"
#include <iostream>
#include <cmath>

raaCarFacarde::raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed): raaAnimatedFacarde(pPart, ap, dSpeed)
{
	this->m_dDefaultSpeed = this->m_dSpeed;
	raaTrafficSystem::addTarget(this); // adds the car to the traffic system (static class) which holds a record of all the dynamic parts in the system
}

raaCarFacarde::~raaCarFacarde()
{
	raaTrafficSystem::removeTarget(this); // removes the car from the traffic system (static class) which holds a reord of all the dynamic parts in the system
}

void raaCarFacarde::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	auto thisPos = this->getWorldDetectionPoint();

	double thisX = thisPos[0];
	double thisY = thisPos[1];

	double finalSpeed = 0;

	// Collision control between cars
	for (auto it = raaTrafficSystem::colliders().begin(); it != raaTrafficSystem::colliders().end(); it++)
	{
		auto collider = *it;

		if (collider == this) // cannot collide with itself
		{
			continue;
		}

		auto colliderPos = collider->getWorldDetectionPoint();

		double colliderX = colliderPos[0];
		double colliderY = colliderPos[1];

		double distance = hypot(colliderX - thisX, colliderY - thisY);

		raaCarFacarde* colliderCar = static_cast<raaCarFacarde*>(collider);

		double colliderSpeed = colliderCar->m_dSpeed;
		double thisSpeed = this->m_dSpeed;

		bool sameDirection = (thisSpeed >= 0 && colliderSpeed >= 0) || (thisSpeed < 0 && colliderSpeed < 0);

		if (distance < 160 && sameDirection)
			finalSpeed = colliderSpeed;
		else
			finalSpeed = this->m_dDefaultSpeed;
	}
	
	// Stopping car at traffic light
	for (auto it = raaTrafficSystem::controllers().begin(); it != raaTrafficSystem::controllers().end(); it++)
	{
		TrafficLightControl* trafficLightControl = *it;

		for (auto it = trafficLightControl->m_lTrafficLights.begin(); it != trafficLightControl->m_lTrafficLights.end(); it++)
		{
			TrafficLightFacarde* trafficLight = *it;

			osg::Vec3f trafficLightLocation = trafficLight->getWorldDetectionPoint();

			double lightX = trafficLightLocation[0];
			double lightY = trafficLightLocation[1];

			double distance = hypot(lightX - thisX, lightY - thisY);

			if (distance < 150 && trafficLight->m_iTrafficLightStatus == 1 && this->trafficLightPass == false)
				finalSpeed = 0;
			else if (distance < 150 && trafficLight->m_iTrafficLightStatus == 1 && this->trafficLightPass == true)
				this->trafficLightPass = false;

			if (distance < 200 && trafficLight->m_iTrafficLightStatus == 3)
			{
				this->trafficLightPass = true;
				finalSpeed = this->m_dDefaultSpeed;
			}
		}
	}
	
	this->m_dSpeed = finalSpeed;

	raaAnimationPathCallback::operator()(node, nv);
}

osg::Vec3f raaCarFacarde::getWorldDetectionPoint()
{
	return this->m_pRoot->getBound().center(); // should return the world position of the detection point for this subtree
}

osg::Vec3f raaCarFacarde::getWorldCollisionPoint()
{
	return osg::Vec3(); // should return the world position of the collision point for this subtree
}

osg::Matrix raaCarFacarde::getWorldRotationPoint()
{
	osg::MatrixTransform* matrixTransform = dynamic_cast<osg::MatrixTransform*>(this->rotation());
	if (matrixTransform)
		return matrixTransform->getMatrix();
	else
		return osg::Matrix::identity();
}

void raaCarFacarde::setTileLists(std::list<RoadTile>* parsedRoadTiles, std::list<NetworkTile>* parsedNetworkTiles)
{
	this->parsedRoadTiles = parsedRoadTiles;
	this->parsedNetworkTiles = parsedNetworkTiles;
}

osg::AnimationPath * createAnimationPath(raaAnimationPointFinders apfs, osg::Group * pRoadGroup)
{
	float fAnimTime = 0.0f;
	osg::AnimationPath* ap = new osg::AnimationPath();

	for (int i = 0; i < apfs.size(); i++)
	{
		float fDistance = 0.0f;
		osg::Vec3 vs;
		osg::Vec3 ve;

		vs.set(apfs[i].translation().x(), apfs[i].translation().y(), apfs[i].translation().z());

		if (i == apfs.size() - 1)
			ve.set(apfs[0].translation().x(), apfs[0].translation().y(), apfs[0].translation().z());
		else
			ve.set(apfs[i + 1].translation().x(), apfs[i + 1].translation().y(), apfs[i + 1].translation().z());

		float fXSqr = pow((ve.x() - vs.x()), 2);
		float fYSqr = pow((ve.y() - vs.y()), 2);
		float fZSqr = pow((ve.z() - vs.z()), 2);

		fDistance = sqrt(fXSqr + fYSqr);
		ap->insert(fAnimTime, osg::AnimationPath::ControlPoint(apfs[i].translation(), apfs[i].rotation()));
		fAnimTime += (fDistance / 10.0f);
	}

	return ap;
}
