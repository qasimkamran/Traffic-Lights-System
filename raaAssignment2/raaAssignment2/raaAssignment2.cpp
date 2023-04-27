#include <windows.h>
#include <iostream>
#include <math.h>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osg/AnimationPath>
#include <osg/Matrix>
#include <osg/Material>
#include <osg/NodeVisitor>
#include <osg/ShapeDrawable>
#include "raaInputController.h"
#include "raaAssetLibrary.h"
#include "raaFacarde.h"
#include "raaSwitchActivator.h"
#include "raaRoadTileFacarde.h"
#include "raaAnimationPointFinder.h"
#include "raaAnimatedFacarde.h"
#include "raaCarFacarde.h"
#include "raaFinder.h"
#include "TrafficLightFacarde.h"
#include "TrafficLightControl.h"
#include "RoadNetworkFileParser.h"
#include "raaTrafficSystem.h"

osg::Group* g_pRoot = 0; // root of the sg
float g_fTileSize = 472.441f; // width/depth of the standard road tiles
std::string g_sDataPath = "../../Data/";

enum raaRoadTileType
{
	Normal,
	LitTJunction,
	LitXJunction,
};

raaFacarde* addRoadTile(std::string sAssetName, std::string sPartName, int xUnit, int yUnit, float fRot, osg::Group* pParent)
{
	raaFacarde* pFacarde = new raaRoadTileFacarde(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot);
	pParent->addChild(pFacarde->root());
	return pFacarde;
}

TrafficLightControl* addTrafficLightControlTile(std::string sAssetName, std::string sPartName, int xUnit, int yUnit, float fRot, float fScale, osg::Group* pParent)
{
	TrafficLightControl* pSpecialTile = new TrafficLightControl(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot, fScale);
	pParent->addChild(pSpecialTile->root());
	return pSpecialTile;
}

TrafficLightFacarde* addTrafficLight(std::string sPartName, osg::Vec3 vTrans, float fRot, float fScale, osg::Group* pParent)
{
	TrafficLightFacarde* tlFacarde = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName), vTrans, fRot, fScale);
	pParent->addChild(tlFacarde->root());
	return tlFacarde;
}

osg::Node* buildAnimatedVehicleAsset()
{
	osg::Group* pGroup = new osg::Group();
	
	osg::Geode* pGB = new osg::Geode();
	osg::ShapeDrawable* pGeomB = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, 0.0f), 100.0f, 60.0f, 40.0f));
	osg::Material* pMat = new osg::Material();
	pMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3f, 0.3f, 0.1f, 1.0f));
	pMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.8f, 0.8f, 0.3f, 1.0f));
	pMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 0.6f, 1.0f));

	pGroup->addChild(pGB);
	pGB->addDrawable(pGeomB);

	pGB->getOrCreateStateSet()->setAttribute(pMat, osg::StateAttribute::ON || osg::StateAttribute::OVERRIDE);
	pGB->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE), osg::StateAttribute::ON || osg::StateAttribute::OVERRIDE);

	return pGroup;
}

void addNodeChild(osg::Node* node1, osg::Node* node2, osg::Group* pGroup)
{
	osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(node1);
	if (group)
	{
		g_pRoot->addChild(pGroup);
		group->addChild(node2);
	}
	else
		printf("nullptr recieved");
}

osg::Node* buildSpecialTJunction(std::string sIdentifier, osg::Group* pParent, osg::Group* pChildren)
{
	std::string sTilePartName = sIdentifier + "Tile";

	std::string sLightPartName1 = sIdentifier + "Light1";
	std::string sLightPartName2 = sIdentifier + "Light2";
	std::string sLightPartName3 = sIdentifier + "Light3";

	TrafficLightControl* pSpecialTile = addTrafficLightControlTile("roadTJunction", sTilePartName, -1, 0, -90.0f, 1.0f, pParent);

	TrafficLightFacarde* pTrafficLight1 = addTrafficLight(sLightPartName1, osg::Vec3(-290, +230, 0), 180, 0.08f, pChildren);
	TrafficLightFacarde* pTrafficLight2 = addTrafficLight(sLightPartName2, osg::Vec3(-240, -170, 0), 90, 0.08f, pChildren);
	TrafficLightFacarde* pTrafficLight3 = addTrafficLight(sLightPartName3, osg::Vec3(-710, +180, 0), -90, 0.08f, pChildren);

	pSpecialTile->addTrafficLight(pTrafficLight1);
	pSpecialTile->addTrafficLight(pTrafficLight2);
	pSpecialTile->addTrafficLight(pTrafficLight3);

	return pSpecialTile->root();
}

osg::Node* buildSpecialXJunction(std::string sIdentifier, osg::Group* pParent, osg::Group* pChildren)
{
	std::string sTilePartName = sIdentifier + "Tile";

	std::string sLightPartName1 = sIdentifier + "Light1";
	std::string sLightPartName2 = sIdentifier + "Light2";
	std::string sLightPartName3 = sIdentifier + "Light3";
	std::string sLightPartName4 = sIdentifier + "Light4";


	TrafficLightControl* pSpecialTile = addTrafficLightControlTile("roadXJunction", sTilePartName, 1, 2, 180.0f, 1.0f, pParent);

	TrafficLightFacarde* pTrafficLight1 = addTrafficLight(sLightPartName1, osg::Vec3(240, +1130, 0), -90, 0.08, pChildren);
	TrafficLightFacarde* pTrafficLight2 = addTrafficLight(sLightPartName2, osg::Vec3(650, +1190, 0), 180, 0.08, pChildren);
	TrafficLightFacarde* pTrafficLight3 = addTrafficLight(sLightPartName3, osg::Vec3(710, +760, 0), 90, 0.08, pChildren);
	TrafficLightFacarde* pTrafficLight4 = addTrafficLight(sLightPartName4, osg::Vec3(290, +710, 0), 0, 0.08, pChildren);

	pSpecialTile->addTrafficLight(pTrafficLight1);
	pSpecialTile->addTrafficLight(pTrafficLight2);
	pSpecialTile->addTrafficLight(pTrafficLight3);
	pSpecialTile->addTrafficLight(pTrafficLight4);

	return pSpecialTile->root();
}

void buildRoad(osg::Group* pRoadGroup, osg::Group* pTrafficLightsGroup)
{
	addRoadTile("roadStraight", "a", 0, 0, 0.0f, pRoadGroup);

	addRoadTile("roadCurve", "b", 1, 0, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "c", 1, 1, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "e", 0, 2, 0.0f, pRoadGroup);

	addRoadTile("roadXJunction", "f", -1, 2, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "g", -1, 1, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "i", -1, 3, -90.0f, pRoadGroup);

	addRoadTile("roadStraight", "j", -2, 0, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "k", 2, 2, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "l", 1, 3, -90.0f, pRoadGroup);

	addRoadTile("roadCurve", "m", 3, 2, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "n", 3, 3, 90.0f, pRoadGroup);

	addRoadTile("roadCurve", "o", 3, 4, 180.0f, pRoadGroup);

	addRoadTile("roadTJunction", "p", 1, 4, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "q", 0, 4, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "r", 2, 4, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "s", -2, 2, 0.0f, pRoadGroup);

	addRoadTile("roadCurve", "t", -3, 2, -90.0f, pRoadGroup);

	addRoadTile("roadStraight", "u", -3, 1, 90.0f, pRoadGroup);

	addRoadTile("roadCurve", "v", -3, 0, 0.0f, pRoadGroup);

	addRoadTile("roadCurve", "w", -1, 4, -90.0f, pRoadGroup);

	buildSpecialXJunction("xNetwork", pRoadGroup, pTrafficLightsGroup);

	buildSpecialTJunction("tNetwork", pRoadGroup, pTrafficLightsGroup);
}

raaAnimationPointFinders getFromPathFinders(osg::Group* pRoadGroup, int routeIndex)
{
	raaAnimationPointFinders apfs;

	switch (routeIndex)
	{
		case 1:
		{
			apfs.push_back(raaAnimationPointFinder("a", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("a", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("b", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("b", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("b", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("c", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("c", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 4, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("l", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("l", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 11, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 7, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 2, pRoadGroup));
		}
		break;
		case 2:
		{
			apfs.push_back(raaAnimationPointFinder("b", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("b", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("b", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("c", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("c", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 4, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("l", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("l", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 11, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 7, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("a", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("a", 0, pRoadGroup));
		}
		break;
		case 3:
		{
			apfs.push_back(raaAnimationPointFinder("r", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("r", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("o", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("o", 4, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("o", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("n", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("n", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("m", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("m", 4, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("m", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("k", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("k", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("xNetworkTile", 10, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("e", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("e", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 8, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("s", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("s", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("t", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("t", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("t", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("u", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("u", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("v", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("v", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("v", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("j", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("j", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 8, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 9, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("tNetworkTile", 7, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("g", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("f", 10, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 1, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("i", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 3, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 4, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("w", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 2, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("q", 0, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 5, pRoadGroup));
			apfs.push_back(raaAnimationPointFinder("p", 4, pRoadGroup));
		}
		break;
	}

	return apfs;
}

void buildCars(osg::Group* pRoadGroup)
{
	raaAnimationPointFinders apfs;
	raaCarFacarde* pCar;
	osg::AnimationPath* ap = new osg::AnimationPath();
	RoadNetworkFileParser fileParser = RoadNetworkFileParser("../../roadRelations.txt", "../../networkRelations.txt");

	apfs = getFromPathFinders(pRoadGroup, 1);
	ap = createAnimationPath(apfs, pRoadGroup);
	pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car1"), ap, 40.0);

	pRoadGroup->addChild(pCar->root());

	apfs = getFromPathFinders(pRoadGroup, 2);
	ap = createAnimationPath(apfs, pRoadGroup);
	pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car2"), ap, 70.0);

	pRoadGroup->addChild(pCar->root());
	
	apfs = getFromPathFinders(pRoadGroup, 3);
	ap = createAnimationPath(apfs, pRoadGroup);
	pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car3"), ap, 80.0);

	pRoadGroup->addChild(pCar->root());
}

int main(int argc, char** argv)
{
	raaAssetLibrary::start();
	raaTrafficSystem::start();

	osgViewer::Viewer viewer;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "-d") g_sDataPath = argv[++i];
	}

	// the root of the scene - use for rendering
	g_pRoot = new osg::Group();
	g_pRoot->ref();

	// build asset library - instances or clones of parts can be created from this
	raaAssetLibrary::loadAsset("roadStraight", g_sDataPath + "roadStraight.osgb");
	raaAssetLibrary::loadAsset("roadCurve", g_sDataPath + "roadCurve.osgb");
	raaAssetLibrary::loadAsset("roadTJunction", g_sDataPath + "roadTJunction.osgb");
	raaAssetLibrary::loadAsset("roadXJunction", g_sDataPath + "roadXJunction.osgb");
	raaAssetLibrary::loadAsset("trafficLight", g_sDataPath + "raaTrafficLight.osgb");
	raaAssetLibrary::loadAsset("vehicle", g_sDataPath + "car-veyron.osgb");

	// add a group node to the scene to hold the road sub-tree
	osg::Group* pRoadGroup = new osg::Group();
	osg::Group* pTrafficLightsGroup = new osg::Group();
	g_pRoot->addChild(pRoadGroup);
	g_pRoot->addChild(pTrafficLightsGroup);

	// Create road
	buildRoad(pRoadGroup, pTrafficLightsGroup);
	
	// Create cars
	buildCars(pRoadGroup);

	// osg setup stuff
	osg::GraphicsContext::Traits* pTraits = new osg::GraphicsContext::Traits();
	pTraits->x = 20;
	pTraits->y = 20;
	pTraits->width = 600;
	pTraits->height = 480;
	pTraits->windowDecoration = true;
	pTraits->doubleBuffer = true;
	pTraits->sharedContext = 0;

	osg::GraphicsContext* pGC = osg::GraphicsContext::createGraphicsContext(pTraits);
	osgGA::KeySwitchMatrixManipulator* pKeyswitchManipulator = new osgGA::KeySwitchMatrixManipulator();
	pKeyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	pKeyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	pKeyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	viewer.setCameraManipulator(pKeyswitchManipulator);
	osg::Camera* pCamera = viewer.getCamera();
	pCamera->setGraphicsContext(pGC);
	pCamera->setViewport(new osg::Viewport(0, 0, pTraits->width, pTraits->height));

	// add own event handler - this currently switches on an off the animation points
	viewer.addEventHandler(new raaInputController(g_pRoot));

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the thread model handler
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the LOD Scale handler
	viewer.addEventHandler(new osgViewer::LODScaleHandler);

	// add the screen capture handler
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	// set the scene to render
	viewer.setSceneData(g_pRoot);

	viewer.realize();

	return viewer.run();
}


