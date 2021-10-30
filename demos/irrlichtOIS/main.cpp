//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS 1

#define _IRR_LINUX_PLATFORM_
#define _IRR_USE_LINUX_DEVICE_

#include "irrlicht.h"
#include "COIS.h"

using namespace irr;
using namespace irr::io;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::gui;
using namespace video;

typedef rect<f32> rectf;
typedef rect<s32> rectd;
typedef std::ostringstream StrStream;

#define WINDOW_SIZE_X       800
#define WINDOW_SIZE_Y       600
#define DEVICE_BPP          24

#define ID_DBGCONSOLE       101
#define ID_ALPHA            102
#define ID_MAGNITUDE        103

static COIS*                m_ois;
static IrrlichtDevice*      m_device;
static IVideoDriver*        m_videoDriver;
static ISceneManager*       m_sceneManager;
static IFileSystem*         m_fileSystem;
static IGUIEnvironment*     m_gui;
static bool                 m_running=true;
static int                  m_capNumber=1;
static IGUIListBox*         m_listbox = 0;
static IGUIListBox*         m_mlistbox = 0;
static IGUIListBox*         m_jlistbox = 0;
static IGUICheckBox*        m_cb1 = 0;
static IGUIStaticText*      m_magText;


static E_DRIVER_TYPE        m_driverType=EDT_OPENGL;
//static E_DRIVER_TYPE        m_driverType=EDT_DIRECT3D9;

//-----------------------------------------------------------------------------
//                                 M y O I S
//-----------------------------------------------------------------------------
class MyOIS: public COIS, public IEventReceiver
{
protected:
    OIS::Effect*        m_effect;
    OIS::ForceFeedback* m_ff;

public:
    MyOIS(IrrlichtDevice* idevice, bool showCursor=true, bool buffered=true,
        bool enableDebug=true) : COIS(idevice, showCursor, buffered, enableDebug)
    {
        m_effect = new OIS::Effect(OIS::Effect::ConstantForce, OIS::Effect::Constant);
        //        m_effect = new OIS::Effect(OIS::Effect::PeriodicForce, OIS::Effect::Triangle);


       // m_effect->direction		  = OIS::Effect::North;
        // Set the duration of the effect in micro seconds.  The default (-1) will
        // play forever or until we "remove" it.
     //   m_effect->replay_length = OIS::Effect::OIS_INFINITE; //1500 * 10000; // 1.5 seconds
        m_effect->replay_length = 1500 * 10000; // 1.5 seconds


        //
        // you can use this to modify the envelope parameters or magnitude
        //
        OIS::ConstantEffect* ce = (OIS::ConstantEffect*)m_effect->getForceEffect();
        ce->level = 5000;  // -10k -> 10k

        m_ois = this;
    };

    int initialize()
    {
        int result = COIS::initialize();
        if(result)
            return result;

        m_ff = getFF(0);
        return 0;
    }

    //
    // plays a "constant" force feedback effect on device 0
    //
    void playEffect()
    {
        if(m_ff)
            m_ff->upload(m_effect);
    }

    // override default handlers
    bool keyPressed( const OIS::KeyEvent& arg );
    bool keyReleased( const OIS::KeyEvent& arg );
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool buttonPressed( const OIS::JoyStickEvent &arg, int button );
    bool buttonReleased( const OIS::JoyStickEvent &arg, int button );
    bool axisMoved( const OIS::JoyStickEvent &arg, int axis );
    bool povMoved( const OIS::JoyStickEvent &arg, int pov );
	virtual bool OnEvent(const SEvent& event);
};

//-----------------------------------------------------------------------------
//                             k e y P r e s s e d
//-----------------------------------------------------------------------------
bool MyOIS::keyPressed(const OIS::KeyEvent& arg )
{
    // add debug info to listbox
    stringw temp = "key.down.";
    temp += getKeyString(arg.key).c_str();
    m_listbox->setSelected(m_listbox->addItem(temp.c_str()));

    //
    // for default gui handling
    //
playEffect();
    if(COIS::keyPressed(arg))
        return true;

    switch(arg.key)
    {
    case OIS::KC_ESCAPE:
        m_running = false;
        return true;

    case OIS::KC_F:
        playEffect();
        return true;

    case OIS::KC_SYSRQ: /* print screen */
        {
            IImage* image = m_videoDriver->createScreenShot();
            char buf[32];

            sprintf(buf,"cap%.2d.png",m_capNumber++);

            m_videoDriver->writeImageToFile(image,buf);

            image->drop();
        }
        break;
    default:
        break;
    }
    return false;
}

//-----------------------------------------------------------------------------
//                            k e y R e l e a s e d
//-----------------------------------------------------------------------------
bool MyOIS::keyReleased(const OIS::KeyEvent& arg )
{
    // add debug info to listbox
    stringw temp = "key.up.";
    temp += getKeyString(arg.key).c_str();
    m_listbox->setSelected(m_listbox->addItem(temp.c_str()));

    //
    // for default gui handling
    //
    if(COIS::keyReleased(arg))
        return true;

    return false;
}

//-----------------------------------------------------------------------------
//                            m o u s e M o v e d
//-----------------------------------------------------------------------------
bool MyOIS::mouseMoved( const OIS::MouseEvent &arg )
{
    stringw temp = "mouse.move (";
    temp += arg.state.X.abs;
    temp += ",";
    temp += arg.state.Y.abs;
    temp += ",";
    temp += arg.state.Z.abs;
    temp += ")";
    m_mlistbox->setSelected(m_mlistbox->addItem(temp.c_str()));
    if(m_mlistbox->getItemCount() > 200)
    {
        for(int i=0; i<50; i++)
        {
            m_mlistbox->removeItem(0);
        }
    }

    return COIS::mouseMoved(arg);
}
//-----------------------------------------------------------------------------
//                            m o u s e P r e s s e d
//-----------------------------------------------------------------------------
bool MyOIS::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    stringw temp = "mouse.pressed (";
    temp += id;
    temp += ") (";
    temp += arg.state.X.abs;
    temp += ",";
    temp += arg.state.Y.abs;
    temp += ")";
    m_mlistbox->setSelected(m_mlistbox->addItem(temp.c_str()));
    return COIS::mousePressed(arg, id);
}

//-----------------------------------------------------------------------------
//                          m o u s e R e l e a s e d
//-----------------------------------------------------------------------------
bool MyOIS::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    stringw temp = "mouse.released (";
    temp += id;
    temp += ") (";
    temp += arg.state.X.abs;
    temp += ",";
    temp += arg.state.Y.abs;
    temp += ")";
    m_mlistbox->setSelected(m_mlistbox->addItem(temp.c_str()));
    return COIS::mouseReleased(arg, id);
}

//-----------------------------------------------------------------------------
//                          b u t t o n P r e s s e d
//-----------------------------------------------------------------------------
bool MyOIS::buttonPressed( const OIS::JoyStickEvent &arg, int button )
{
    if(button == 7)
        playEffect();

    stringw temp = "button.pressed (";
    temp += button;
    temp += ")";
    m_jlistbox->setSelected(m_jlistbox->addItem(temp.c_str()));

    return COIS::buttonPressed(arg, button);
}

//-----------------------------------------------------------------------------
//                          b u t t o n R e l e a s e d
//-----------------------------------------------------------------------------
bool MyOIS::buttonReleased( const OIS::JoyStickEvent &arg, int button )
{
    stringw temp = "button.released (";
    temp += button;
    temp += ")";
    m_jlistbox->setSelected(m_jlistbox->addItem(temp.c_str()));

    return COIS::buttonReleased(arg, button);
}

//-----------------------------------------------------------------------------
//                             a x i s M o v e d
//-----------------------------------------------------------------------------
bool MyOIS::axisMoved( const OIS::JoyStickEvent &arg, int axis )
{
    stringw temp = "axisMoved (";
    temp += axis;
    temp += ") (";
    temp += arg.state.mAxes[axis].abs;
    temp += ")";
    m_jlistbox->setSelected(m_jlistbox->addItem(temp.c_str()));

    return COIS::axisMoved(arg, axis);
}

//-----------------------------------------------------------------------------
//                              p o v M o v e d
//-----------------------------------------------------------------------------
bool MyOIS::povMoved( const OIS::JoyStickEvent &arg, int pov )
{
    stringw temp = "povMoved (";
    temp += pov;
    temp += ") (";
    temp += arg.state.mPOV[pov].direction;
    temp += ")";
    m_jlistbox->setSelected(m_jlistbox->addItem(temp.c_str()));
    return COIS::povMoved(arg, pov);
}
//-----------------------------------------------------------------------------
//                              O n E v e n t
//-----------------------------------------------------------------------------
bool MyOIS::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch(event.GUIEvent.EventType)
        {
        case EGET_CHECKBOX_CHANGED:
            setDebugEnabled(m_cb1->isChecked());
            break;
        case EGET_SCROLL_BAR_CHANGED:
            if (id == ID_ALPHA)
            {
                s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();

                for (u32 i=0; i<EGDC_COUNT ; ++i)
                {
                    SColor col = m_gui->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
                    col.setAlpha(pos);
                    m_gui->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
                }

            }
            else if(id == ID_MAGNITUDE)
            {
                s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                stringw temp;
                temp = "FF Magnitude (";
                temp += pos;
                temp += ")";
                m_magText->setText(temp.c_str());

                //
                // takes affect the next time the effect is played.
                //
                OIS::ConstantEffect* ce = (OIS::ConstantEffect*)m_effect->getForceEffect();
                ce->level = pos;  // -10k -> 10k


            }
            break;
        default:
            break;
        };
    }

    return false;
}


//-----------------------------------------------------------------------------
//                            _ c r e a t e G U I
//-----------------------------------------------------------------------------
static void _createGUI()
{
    stringw temp;

	m_gui->addStaticText(L"Transparent Control:", rect<s32>(50,20,250,40), true);
	IGUIScrollBar* scrollbar = m_gui->addScrollBar(true, rect<s32>(50, 45, 250, 60), 0, ID_ALPHA);
	scrollbar->setMax(255);
	scrollbar->setPos(255);

    m_cb1 = m_gui->addCheckBox(true, rect<s32>(300, 20, 500, 40), 0, ID_DBGCONSOLE, L"Print Debug Info To Console");

	m_gui->addEditBox(L"Editable Text", rect<s32>(50, 80, 250, 100));

	m_gui->addStaticText(L"Key Logging ListBox:", rect<s32>(50,110,250,130), true);
	m_listbox = m_gui->addListBox(rect<s32>(50, 130, 250, 500), 0, -1, true);
    m_listbox->setAutoScrollEnabled(true);

	m_gui->addStaticText(L"Mouse Logging ListBox:", rect<s32>(300,110,500,130), true);
	m_mlistbox = m_gui->addListBox(rect<s32>(300, 130, 500, 500), 0, -1, true);
    m_mlistbox->setAutoScrollEnabled(true);


    // count the number of joysticks with FF
    u32 numFF = 0;
    for(u32 i=0; i<m_ois->getNumSticks(); i++)
    {
        if(m_ois->hasForceFeedback(i))
            ++numFF;
    }

    temp = "Keyboards: ";
    temp += m_ois->getNumKeyboards();
    temp += ", Mice: ";
    temp += m_ois->getNumMice();
    temp += ", JoySticks: ";
    temp += m_ois->getNumSticks();
    temp += ", FF: ";
    temp += numFF;
    m_gui->addStaticText(temp.c_str(), rect<s32>(550,20,750,40), true)->setDrawBorder(false);

	m_gui->addStaticText(L"JoyStick Logging ListBox:", rect<s32>(550,110,750,130), true);
	m_jlistbox = m_gui->addListBox(rect<s32>(550, 130, 750, 500), 0, -1, true);
    m_jlistbox->setAutoScrollEnabled(true);

    if(numFF)
    {
        m_gui->addStaticText(L"Play Force Feedback: Key - \"F\", or Joy Button - 7",
            rect<s32>(550,40,750,60), true)->setDrawBorder(false);
        // FF controls
        m_magText = m_gui->addStaticText(L"FF Magnitude (5000)", rect<s32>(550,508,750,520), true);
        m_magText->setDrawBorder(false);

	    IGUIScrollBar* scrollbar = m_gui->addScrollBar(true, rect<s32>(550, 520, 750, 535), 0, ID_MAGNITUDE);
	    scrollbar->setMax(10000);
	    scrollbar->setPos(5000);

    }

    // turn up the alpha for skin colors
    for (u32 i=0; i<EGDC_COUNT ; ++i)
    {
        SColor col = m_gui->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
        col.setAlpha(255);
        m_gui->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
    }
}

//-----------------------------------------------------------------------------
//                           _ c r e a t e D e v i c e
//-----------------------------------------------------------------------------
static IrrlichtDevice* _createDevice()
{
    SIrrlichtCreationParameters cp;
    cp.DriverType = m_driverType;
    cp.WindowSize = dimension2du(WINDOW_SIZE_X,WINDOW_SIZE_Y);
    cp.Bits = DEVICE_BPP;
    cp.Fullscreen = false;
    cp.Vsync = false;
    cp.Stencilbuffer = false;
    cp.AntiAlias = false;
    cp.EventReceiver = 0;
    cp.WindowId = 0;

#if defined(_IRR_LINUX_PLATFORM_) && defined(_IRR_USE_LINUX_DEVICE_)
    cp.IgnoreInput = true;
#endif

    return createDeviceEx(cp);
}

//-----------------------------------------------------------------------------
//                                 m a i n
//-----------------------------------------------------------------------------
#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:console /ENTRY:mainCRTStartup")
#endif
int main(int argc, char* argv[])
{
    MyOIS* m_ois;

    m_device = _createDevice();
    if(!m_device)
        return -1;

    m_fileSystem = m_device->getFileSystem();
    m_videoDriver = m_device->getVideoDriver();
    m_sceneManager = m_device->getSceneManager();
    m_gui = m_device->getGUIEnvironment();

    //
    // init ois (device, show cursor, buffered, debug enabled)
    //
    m_ois = new MyOIS(m_device, true, true, true);
    if(m_ois->initialize())
        return 1;
    m_ois->setGUIEnabled(true);
    //m_ois->setGUIExclusive(true);

    m_device->setWindowCaption(L"OIS Example");

    _createGUI();

    m_device->setEventReceiver(m_ois);

    m_sceneManager->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

    while(m_device->run() && m_running)
    {
        // capture input
        m_ois->capture();

        m_videoDriver->beginScene(true, true, SColor(0,200,200,200));

        m_sceneManager->drawAll();
        m_gui->drawAll();

        m_videoDriver->endScene();
    }

    delete m_ois;
    m_device->drop();

    return 0;
}
