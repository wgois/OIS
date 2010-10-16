/*
The zlib/libpng License

Copyright (c) 2010 Martin Preisler

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that 
		you wrote the original software. If you use this software in a product, 
		an acknowledgment in the product documentation would be appreciated but is 
		not required.

    2. Altered source versions must be plainly marked as such, and must not be 
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "Application.h"

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreTimer.h>
#include <OgreWindowEventUtilities.h>
#include <OgreCamera.h>
#include <OgreEntity.h>

#include <OISInputManager.h>

#include <OISBSystem.h>
#include <OISBAction.h>
#include <OISBActionSchema.h>
#include <OISBAnalogAxisAction.h>
#include <OISBSequenceAction.h>
#include <OISBBinding.h>

#include <RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <CEGUISystem.h>
#include <elements/CEGUIGUISheet.h>
#include <elements/CEGUIProgressBar.h>
#include <CEGUIWindowManager.h>
#include <CEGUISchemeManager.h>

Application::Application():
    mRunning(false),
    
    mOgreRoot(0),
    mOgreWindow(0),

    mVelocity(0, 0, 0),

    mUIRoot(0)
{}

Application::~Application()
{}

void Application::run()
{
    initialize();
    setupScene();

    Ogre::Timer timer;
    timer.reset();

    mRunning = true;
    while (mRunning)
    {
        const Ogre::Real delta=
            timer.getMicroseconds() * static_cast<Ogre::Real>(0.000001f);
        timer.reset();

        runOneFrame(delta);
    }

    finalize();
}

void Application::initializeOgre()
{
    mOgreRoot = new Ogre::Root("OgrePlugins.cfg", "OgreConfig.cfg", "Ogre.log");
    mOgreRoot->addResourceLocation("../../../Data/OISBDemo", "FileSystem");
    //mOgreRoot->addResourceLocation("Data", "FileSystem");
    mOgreRoot->showConfigDialog();
    mOgreWindow = mOgreRoot->initialise(true, "OISB demo");

    mOgreRoot->getRenderSystem()->_initRenderTargets();

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    // Clear event times
    mOgreRoot->clearEventTimes();
}

void Application::defineActions()
{
    mDefaultSchema = OISB::System::getSingleton().getDefaultActionSchemaAutoCreate();
    mExitAction = mDefaultSchema->createAction(OISB::AT_TRIGGER, "Exit");

    mExitAction->bind("Keyboard/Esc");
    mExitAction->bind("Keyboard/Ctrl", "Keyboard/Q");

    mChopterSchema = OISB::System::getSingleton().createActionSchema("Chopter", false);

    mThrottle = mChopterSchema->createAction<OISB::AnalogAxisAction>("Throttle");
    mThrottle->setProperty("MinimumValue", 0.0f);
    mThrottle->setProperty("EmulationReturnValue", 9.0f);
    mThrottle->setProperty("MaximumValue", 15.0f);
    mThrottle->setProperty("EmulationSpeed", 10.0f);
    mThrottle->setProperty("EmulationReturnSpeed", 10.0);
    mThrottle->bind("Keyboard/S", "Keyboard/W");
    //mThrottle->bind("Mouse/Y Axis");

    mSteering = mChopterSchema->createAction<OISB::AnalogAxisAction>("Steering");
    mSteering->setProperty("MinimumValue", -50.0f);
    mSteering->setProperty("MaximumValue", 50.0f);
    mSteering->setProperty("EmulationSpeed", 50.0f);
    mSteering->setProperty("EmulationReturnSpeed", 50.0f);
    mSteering->bind("Keyboard/A", "Keyboard/D");
    //mSteering->bind("Mouse/X Axis");

    mTilting = mChopterSchema->createAction<OISB::AnalogAxisAction>("Tilting");
    mTilting->setProperty("MinimumValue", -50.0f);
    mTilting->setProperty("MaximumValue", 50.0f);
    mTilting->setProperty("EmulationSpeed", 50.0f);
    mTilting->setProperty("EmulationReturnSpeed", 50.0f);
    mTilting->bind("Keyboard/Up", "Keyboard/Down");
    //mTilting->bind("Mouse/Y Axis");

    mLeaning = mChopterSchema->createAction<OISB::AnalogAxisAction>("Leaning");
    mLeaning->setProperty("MinimumValue", -50.0f);
    mLeaning->setProperty("MaximumValue", 50.0f);
    mLeaning->setProperty("EmulationSpeed", 50.0f);
    mLeaning->setProperty("EmulationReturnSpeed", 50.0f);
    mLeaning->bind("Keyboard/Left", "Keyboard/Right");

    mBoostCheat = mChopterSchema->createAction<OISB::SequenceAction>("Boost cheat!");
    OISB::Binding* b = mBoostCheat->createBinding();
    b->bind("Keyboard/I");
    b->bind("Keyboard/D");
    b->bind("Keyboard/K");
    b->bind("Keyboard/F");
    b->bind("Keyboard/A");
}

void Application::initializeOISB()
{
    // initialize OIS and OISB

    // Setup basic variables
    OIS::ParamList paramList;    
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    // Get window handle
    mOgreWindow->getCustomAttribute("WINDOW", &windowHnd);
 
    // Fill parameter list
    windowHndStr << (unsigned int) windowHnd;
    paramList.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
    // Create inputsystem
    new OISB::System();
    OISB::System::getSingleton().initialize(
        OIS::InputManager::createInputSystem(paramList));

    defineActions();
}

void Application::initializeCEGUI()
{
    Ogre::Timer timer;
    timer.reset();

    CEGUI::OgreRenderer::bootstrapSystem(*mOgreWindow);

    mUIRoot = static_cast<CEGUI::GUISheet*>(CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Root"));
    mUIRoot->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0)));
    mUIRoot->setSize(CEGUI::UVector2(CEGUI::UDim(1, 0), CEGUI::UDim(1, 0)));
    mUIRoot->setVisible(true);

    CEGUI::System::getSingleton().setGUISheet(mUIRoot);

    CEGUI::SchemeManager::getSingleton().create("OISBDemo.scheme");
	CEGUI::System::getSingleton().setDefaultMouseCursor("OgreTrayImages", "MouseArrow");
    CEGUI::MouseCursor::getSingleton().hide();

    //mUIRoot->addChildWindow(CEGUI::WindowManager::getSingleton().loadWindowLayout("Introduction.layout"));
    mUIRoot->addChildWindow(CEGUI::WindowManager::getSingleton().loadWindowLayout("HUD.layout"));
    mUIHUDThrottle = static_cast<CEGUI::ProgressBar*>(CEGUI::WindowManager::getSingleton().getWindow("HUD/Throttle"));
    mUIHUDVelocity = CEGUI::WindowManager::getSingleton().getWindow("HUD/Velocity");
    mUIHUDAltitude = CEGUI::WindowManager::getSingleton().getWindow("HUD/Altitude");

    std::stringstream sstr;
    sstr << timer.getMicroseconds();

    CEGUI::Logger::getSingleton().logEvent("Time to start CEGUI: " + sstr.str());
}

void Application::initialize()
{
    initializeOgre();
    initializeOISB();
    initializeCEGUI();
}

void Application::finalizeOgre()
{
    mOgreRoot->detachRenderTarget(mOgreWindow);
    mOgreWindow = 0;

    delete mOgreRoot;
    mOgreRoot = 0;
}

void Application::finalizeOISB()
{
    delete OISB::System::getSingletonPtr();
}

void Application::finalizeCEGUI()
{
    CEGUI::OgreRenderer::destroySystem();
	mUIRoot = 0;
}

void Application::finalize()
{
    finalizeCEGUI();
    finalizeOISB();
    finalizeOgre();
}

void Application::setupScene()
{
    mOgreScene = Ogre::Root::getSingleton().createSceneManager("OctreeSceneManager");
    mOgreScene->setAmbientLight(Ogre::ColourValue::Black);

    mOgreCamera = mOgreScene->createCamera("Camera");
	mOgreCamera->setNearClipDistance(0.1f);
	mOgreCamera->setFarClipDistance(1000.0f);

	Ogre::Viewport* viewport = mOgreWindow->addViewport(mOgreCamera);
    mOgreCamera->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));
    mOgreCamera->setPosition(0, 0, 0);

    mOgreFloor = mOgreScene->createEntity("Floor", "plocha.mesh");
    mOgreScene->getRootSceneNode()->attachObject(mOgreFloor);

    Ogre::Light* light = mOgreScene->createLight("Light");
    light->setType(Ogre::Light::LT_POINT);
    
    mOgreChopterNode = mOgreScene->getRootSceneNode()->createChildSceneNode("ChopterNode");
    mOgreChopterNode->setPosition(0, 20, 0);
    mOgreChopterNode->attachObject(mOgreCamera);
    mOgreChopterNode->attachObject(light);
    mOgreChopterNode->setFixedYawAxis(true);
}

void Application::checkDefaultActions()
{
    if (mExitAction->isActive())
    {
        mRunning = false;
    }
}

void Application::checkChopterActions(Ogre::Real delta)
{
    const Ogre::Vector3 acceleration = (mOgreChopterNode->getOrientation() * Ogre::Vector3(0.0f, 1.0f, -0.3f)) *
        mThrottle->getAbsoluteValue();

    mUIHUDThrottle->setProgress((mThrottle->getAbsoluteValue() - mThrottle->getProperty<float>("EmulationReturnValue") + 1.0f) /
        (mThrottle->getMaximumValue() - mThrottle->getProperty<float>("EmulationReturnValue") + 1.0f));

    mVelocity += acceleration * delta;
    // newtonian gravity
    mVelocity += Ogre::Vector3(0, -9.81f, 0) * delta;
    // naive damping (this should somewhat resemble air drag and what not)
    mVelocity *= 1.0f - (0.3f * delta);

    std::stringstream velocityStr;
    velocityStr << (mVelocity.length() * 3.6f) << " km/h";

    mUIHUDVelocity->setText(velocityStr.str().c_str());

    std::stringstream altitudeStr;
    Ogre::Real altitude = mOgreChopterNode->getPosition().y - 8.0f;
    // dirty dirty fix for the near 0.0 flicker
    if (fabs(altitude) < 0.0001)
    {
        altitude = 0.0f;
    }
    altitudeStr << altitude << " m";
    mUIHUDAltitude->setText(altitudeStr.str().c_str());

    if (mOgreChopterNode->getPosition().y <= 8.0f && mVelocity.y < 0.0f)
    {
        // a bit of overall damping
        mVelocity.x *= 0.3f;
        mVelocity.z *= 0.3f;
        // 30% bounce? not realistic but playable
        mVelocity.y = mVelocity.y * -0.3f;
    }

    mOgreChopterNode->translate(mVelocity * delta, Ogre::Node::TS_WORLD);

    mOgreChopterNode->rotate(Ogre::Vector3(0, -1, 0), Ogre::Degree(mSteering->getAbsoluteValue() * delta), Ogre::Node::TS_WORLD);
    mOgreChopterNode->rotate(Ogre::Vector3(1, 0, 0), Ogre::Degree(mTilting->getAbsoluteValue() * delta), Ogre::Node::TS_LOCAL);
    mOgreChopterNode->rotate(Ogre::Vector3(0, 0, -1), Ogre::Degree(mLeaning->getAbsoluteValue() * delta), Ogre::Node::TS_LOCAL);

    // secret cheat, don't read!!!
    if (mBoostCheat->isActive())
    {
        // pimp my heli!
        mThrottle->setMaximumValue(50.0f);
        // crazy throttle action as a bonus for keyboarders!
        //mThrottle->setEmulationSpeed(200.0f);
        //mThrottle->setEmulationReturnSpeed(1000.0f);
    }
}

void Application::runOneFrame(Ogre::Real delta)
{
    OISB::System::getSingleton().process(delta);
    CEGUI::System::getSingleton().injectTimePulse(delta);

    // Pump messages in all registered RenderWindow windows
    Ogre::WindowEventUtilities::messagePump();

    if (!mOgreRoot->renderOneFrame())
    {
	    //mRunning = false;
    }

    checkDefaultActions();
    checkChopterActions(delta);
}
