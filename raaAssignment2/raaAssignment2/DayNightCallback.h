#pragma once

#include <windows.h>
#include <osg/switch>
#include <osg/Timer>
#include <osg/Light>
#include <osg/LightSource>
#include <algorithm>

class DayNightCallback : public osg::NodeCallback
{
public:
	DayNightCallback(osg::LightSource* lightSource)
		: lightSource(lightSource),
		  startTime(osg::Timer::instance()->tick())
	{
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		// Get the current time
		osg::Timer_t tick = osg::Timer::instance()->tick();
		double timeOfDay = osg::Timer::instance()->delta_s(startTime, tick) + timeOffset;

		// Calculate the sun's position based on the time of day
		float sunAngle = (timeOfDay / 86400.0f) * 2.0f * osg::PI;
		osg::Vec3 sunPosition(cos(-sunAngle), sin(-sunAngle), 0.3f);

		// Update the light source's position
		osg::Light* light = lightSource->getLight();
		light->setPosition(osg::Vec4(sunPosition, 0.0f));

		// Calculate the brightness based on the sun's position
		float brightness = max(0.3f, sunAngle/10) * maxBrightness;

		// Set the brightness of the light
		light->setDiffuse(osg::Vec4(brightness, brightness, brightness, 1.0f));

		// Traverse the scene graph
		traverse(node, nv);
	}

	virtual void updateTimeOffset()
	{
		timeOffset += 3.0 * 3600.0;
		if (timeOffset == 86400) // Avoid overflow by resetting at 24 hour mark
			timeOffset = 0;
	}

private:
	osg::ref_ptr<osg::LightSource> lightSource;
	osg::Timer_t startTime;
	double timeOffset = 0.0;
	float maxBrightness = 1.0f;
};