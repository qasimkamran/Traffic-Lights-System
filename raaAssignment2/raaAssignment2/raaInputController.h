#pragma once

#include <windows.h>
#include <osgGA/GUIEventHandler>
#include "DayNightCallback.h"

// the basic input controller. You should extend and enhance this for your user input

class raaInputController: public osgGA::GUIEventHandler
{
public:
	virtual bool handle(const osgGA::GUIEventAdapter&, osgGA::GUIActionAdapter&);
	raaInputController(osg::Node* pWorldRoot, DayNightCallback* pDayNightCallback);
	virtual ~raaInputController();
protected:
	bool m_bShowAnimationPoints;
	bool m_bShowAnimationNames;
	bool m_bShowCollisionObjects;
	bool m_bShowAssetName;
	DayNightCallback* pDayNightCallback;
	osg::Node* m_pWorldRoot;
};

