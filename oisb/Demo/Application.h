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

#include <OgrePrerequisites.h>
#include <CEGUIForwardRefs.h>
#include <OISBGlobal.h>

#include <OgreVector3.h>

/**
 * @brief central demo class
 */
class Application
{
    public:
        Application();

        ~Application();

        void run();

    private:
        void initializeOgre();
        
        void defineActions();

        void initializeOISB();
        void initializeCEGUI();
        void initialize();

        void finalizeOgre();
        void finalizeOISB();
        void finalizeCEGUI();
        void finalize();

        void setupScene();

        void checkDefaultActions();
        void checkChopterActions(Ogre::Real delta);

        void runOneFrame(Ogre::Real delta);

        bool mRunning;

        Ogre::Root* mOgreRoot;
        Ogre::RenderWindow* mOgreWindow;
        Ogre::SceneManager* mOgreScene;
        Ogre::Camera* mOgreCamera;
        Ogre::Entity* mOgreFloor;
        Ogre::SceneNode* mOgreChopterNode;

        OISB::ActionSchema* mDefaultSchema;
        OISB::Action* mExitAction;

        OISB::ActionSchema* mChopterSchema;
        OISB::AnalogAxisAction* mThrottle;
        OISB::AnalogAxisAction* mSteering;
        OISB::AnalogAxisAction* mTilting;
        OISB::AnalogAxisAction* mLeaning;

        // secret cheat!
        OISB::SequenceAction* mBoostCheat;

        Ogre::Vector3 mVelocity;

        //OISB::ActionSchema* mUISchema;

        CEGUI::GUISheet* mUIRoot;
        CEGUI::ProgressBar* mUIHUDThrottle;
        CEGUI::Window* mUIHUDVelocity;
        CEGUI::Window* mUIHUDAltitude;
};
