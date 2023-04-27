
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
	this->m_dDefaultSpeed = this->m_dSpeed; // Original speed to revert to after alterations
	
	// Setup car transformations
	osg::MatrixTransform* rotationMatrixTransform = this->getWorldRotationPoint();
	rotationMatrixTransform->preMult(osg::Matrix::rotate(osg::PI_2, osg::Vec3(0.0f, 0.0f, 1.0f)));
	osg::MatrixTransform* scaleMatrixTransform = this->getWorldScalePoint();
	scaleMatrixTransform->preMult(osg::Matrix::scale(10.0f, 10.0f, 10.0f));
	osg::MatrixTransform* translationMatrixTransform = this->getWorldTranslationPoint();
	translationMatrixTransform->postMult(osg::Matrix::translate(osg::Vec3(0.0f, 0.0f, 30.0f)));

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
	double collisionDetectionSpeed = 0;

	// Collision control between cars
	for (auto it = raaTrafficSystem::colliders().begin(); it != raaTrafficSystem::colliders().end(); it++)
	{
		auto collider = *it;

		if (collider == this) // Cannot collide with itself
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

		// Only adjust speed if the colliders are moving in the same direction through this variable
		bool sameDirection = (-10 < (thisX - colliderX) < 10);

		if (distance < 200 && sameDirection)
		{
			collisionDetectionSpeed = colliderSpeed;
			break;
		}
		else
			collisionDetectionSpeed = this->m_dDefaultSpeed;
	}
	
	finalSpeed = collisionDetectionSpeed;

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

			/*
			Traffic Light: have you already passed a traffic light?
			if condition:
				Car: No.
				Traffic Light: Then you wait.
			else if condition:
				Car: Yes.
				Traffic Light: Reset your pass state for the next time you find a signal.
			*/
			if (distance < 150 && trafficLight->m_iTrafficLightStatus != 3 && this->trafficLightPass == false)
			{
				finalSpeed = 0;
				break; // Optimisation for loop
			}
			else if (distance < 150 && trafficLight->m_iTrafficLightStatus != 3 && this->trafficLightPass == true)
				this->trafficLightPass = false;

			// Traffic light pass indicates that it has passed a traffic light
			if (distance < 150 && trafficLight->m_iTrafficLightStatus == 3)
			{
				this->trafficLightPass = true;
				finalSpeed = collisionDetectionSpeed;
				if (collisionDetectionSpeed == 0)
					finalSpeed = this->m_dDefaultSpeed;
				break; // Optimisation for loop
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

osg::MatrixTransform* raaCarFacarde::getWorldRotationPoint()
{
	return this->rotation();
}

osg::MatrixTransform* raaCarFacarde::getWorldScalePoint()
{
	return this->scale();
}

osg::MatrixTransform* raaCarFacarde::getWorldTranslationPoint()
{
	return this->translation();
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
