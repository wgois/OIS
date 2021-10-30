//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#ifndef _COIS_H_
#define _COIS_H_
#include <OIS.h>
#ifdef _IRR_WINDOWS_
#include "windows.h"
#endif

#define MAX_JOYS    4

using namespace irr;
using namespace irr::gui;
//! OIS/Irrlicht Wrapper Class.
/**
*/
class COIS : public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
protected:
    IrrlichtDevice*         m_device;
    IGUIEnvironment*        m_gui;
    OIS::InputManager*      m_inputManager;
    void*                   m_windowHandle;
    u32                     m_display;
    bool                    m_showCursor;
    bool                    m_buffered;
    bool                    m_debugEnabled;
    bool                    m_GUIEnabled;
    bool                    m_GUIExclusive;
    OIS::Keyboard*          m_keyboard;
    OIS::Mouse*             m_mouse;
    OIS::JoyStick*          m_joys[MAX_JOYS];
    OIS::ForceFeedback*     m_ff[MAX_JOYS];
    u32                     m_numSticks;
    u32                     m_numKeyboards;
    u32                     m_numMice;

#ifdef _IRR_WINDOWS_
    LONG_PTR                m_oldWndProc;
    static LRESULT _wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

public:
    //! Constructor
    /**
    \param idevice Irrlicht Device.
    \param showCursor Whether or not the mouse cursor is visible.
    \param buffered Whether or not to use buffered input.
    \param debugEnabled Whether or not to display event info on stdout.*/
    COIS(IrrlichtDevice* idevice, bool showCursor=true, bool buffered=true, bool debugEnabled=false);

    //! destructor
    virtual ~COIS();

    //! COIS initialization
    /** This method must be called to initialize OIS before input handling can occur.
    */
    virtual int initialize();

    //! Allows OIS to gather and fire input events.
    void capture();

    //! Turn debugging on/off on stdout.
    void setDebugEnabled(bool value) {m_debugEnabled = value;}

    //! Turn Irrlicht GUI processing on/off.
    /** If GUI processing is enabled, COIS translates OIS keyboard/mouse events
    into the appropriate Irrlicht event and then posts the translated event
    into Irrlicht (postEventFromUser).
    */
    void setGUIEnabled(bool value) {m_GUIEnabled = value;}

    //! Returns whether or not events are being feed into Irrlicht.
    bool getGUIEnabled() {return m_GUIEnabled;}

    void setGUIExclusive(bool value) {m_GUIExclusive = value;}
    bool getGUIExclusive() {return m_GUIExclusive;}

    //! Translates an OIS key code to an equivalent Irrlicht key code
    static EKEY_CODE getIrrKeyCode(OIS::KeyCode key);

    //! Translates an OIS mouse event to an equivalent Irrlicht mouse event
    static EMOUSE_INPUT_EVENT getIrrMouseEvent(OIS::MouseButtonID id, bool pressed);

    //! Checks if a specific OIS key is down.
    bool isKeyDown(OIS::KeyCode key);

    //! Sets the display size to be used by OIS.
    void setDisplaySize(int width, int height);

    //! Returns the number of Keyboards available.
    u32  getNumKeyboards() {return m_numKeyboards;}

    //! Returns the number of Mice available.
    u32  getNumMice() {return m_numMice;}

    //! Returns the number of Joysticks available.
    u32  getNumSticks() {return m_numSticks;}

    //! Keyboard keycode -> string representation
    irr::core::stringc getKeyString(const OIS::KeyCode key);

    //! Checks if a particular Joystick contains force feed back functionality.
    bool hasForceFeedback(u32 stickNumber);

    //! Gets the force feedback interface for a specified Joystick
    OIS::ForceFeedback* getFF(u32 stickNumber) {return m_ff[stickNumber];}

    //! Invoked when a key is pressed.
    virtual bool keyPressed( const OIS::KeyEvent& arg );

    //! Invoked when a key is released.
    virtual bool keyReleased( const OIS::KeyEvent& arg );

    //! Invoked when the mouse is moved.
    virtual bool mouseMoved( const OIS::MouseEvent &arg );

    //! Invoked when a mouse button is pressed.
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    //! Invoked when a mouse button is released.
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    //! Invoked when a joystick button is pressed.
    virtual bool buttonPressed( const OIS::JoyStickEvent &arg, int button );

    //! Invoked when a joystick button is released.
    virtual bool buttonReleased( const OIS::JoyStickEvent &arg, int button );

    //! Invoked when a joystick axis is moved.
    virtual bool axisMoved( const OIS::JoyStickEvent &arg, int axis );

    //! Invoked when a joystick pov is moved.
    virtual bool povMoved( const OIS::JoyStickEvent &arg, int pov );

};

#ifdef _IRR_WINDOWS_
#ifdef _DEBUG
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "ois_static_d.lib")
#else
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "ois_static.lib")
#endif
#endif

#endif
