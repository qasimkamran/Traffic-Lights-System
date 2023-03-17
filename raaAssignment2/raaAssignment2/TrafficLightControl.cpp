#include "TrafficLightControl.h"

TrafficLightControl::TrafficLightControl(osg::Node* pPart, osg::Vec3 vTrans, float fRot, float fScale) : raaNodeCallbackFacarde(pPart, vTrans, fRot, fScale)
{
	timeCount = 0;
	activeTrafficLight = 0;
}

TrafficLightControl::~TrafficLightControl()
{

}

void TrafficLightControl::updateActiveTrafficLight()
{
	activeTrafficLight++;
	if (activeTrafficLight > m_lTrafficLights.size()-1)
		activeTrafficLight = 0;
	printf("%d", activeTrafficLight);
}

void TrafficLightControl::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
	if (timeCount == 200)
	{
		auto it = std::next(m_lTrafficLights.begin(), activeTrafficLight);

		changeTrafficLight(*it);
		(*it)->m_iUpdateCounter--;
		
		if ((*it)->m_iUpdateCounter == 0)
		{
			updateActiveTrafficLight();
			(*it)->resetUpdateCounter();
		}
		timeCount = 0;
	}
	timeCount++;
}

void TrafficLightControl::changeTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	pTrafficLight->m_iTrafficLightStatus++;
	if (pTrafficLight->m_iTrafficLightStatus > 3)
	{
		pTrafficLight->m_iTrafficLightStatus = 1;
	}
	if (pTrafficLight->m_iTrafficLightStatus == 1)
	{
		pTrafficLight->setRedTrafficLight();
	}
	if (pTrafficLight->m_iTrafficLightStatus == 2)
	{
		pTrafficLight->setAmberTrafficLight();
		timeCount += 50;
	}
	if (pTrafficLight->m_iTrafficLightStatus == 3)
	{
		pTrafficLight->setGreenTrafficLight();
	}
}

void TrafficLightControl::addTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	m_lTrafficLights.push_back(pTrafficLight);
}

