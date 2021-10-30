//-----------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
// For the full text of the Unlicense, see the file "docs/unlicense.html".
// Additional Unlicense information may be found at http://unlicense.org.
//-----------------------------------------------------------------------------
#include "irrlicht.h"
#include "COIS.h"
#ifdef _IRR_WINDOWS_API_
#else
#ifdef _IRR_COMPILE_WITH_X11_
#include <X11/Xlib.h>
#endif
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

using namespace irr::core;
using namespace OIS;
const char* wmDeleteWindow = "WM_DELETE_WINDOW";
const char *g_DeviceType[6] = {"OISUnknown", "OISKeyboard", "OISMouse", "OISJoyStick",
"OISTablet", "OISOther"};

// FF force type
static stringc SForce[] =
{
    "UnknownForce",
    "ConstantForce",
    "RampForce",
    "PeriodicForce",
    "ConditionalForce",
    "CustomForce"
};

// Type of FF effect
static stringc SType[] =
{
    "Unknown",      //UnknownForce
    "Constant",     //ConstantForce
    "Ramp",         //RampForce
    "Square",       //PeriodicForce
    "Triangle",     //PeriodicForce
    "Sine",         //PeriodicForce
    "SawToothUp",   //PeriodicForce
    "SawToothDown", //PeriodicForce
    "Friction",     //ConditionalForce
    "Damper",       //ConditionalForce
    "Inertia",      //ConditionalForce
    "Spring",       //ConditionalForce
    "Custom"        //CustomForce
};




// used for debugging key codes
static stringc scancodes[]=
{
    "0x00","esc","1","2","3","4","5","6","7","8","9","0",
    "minus","equal","back","tab",
    "q","w","e","r","t","y","u","i","o","p",
    "lbracket","rbracket","return","lcontrol",
    "a","s","d","f","g","h","j","k","l",
    "semicolon","apostrophe","grave","lshift","backslash",
    "z","x","c","v","b","n","m",
    "comma","period","slash","rshift","multiply","lmenu","space","capital",
    "f1","f2","f3","f4","f5","f6","f7","f8","f9","f10",
    "numlock","scroll","numpad7","numpad8","numpad9","subtract",
    "numpad4","numpad5","numpad6","add","numpad1","numpad2","numpad3",
    "numpad0","decimal","0x54","0x55","oem_102","f11","f12","0x59",
    "0x5a","0x5b","0x5c","0x5d","0x5e","0x5f","0x60","0x61","0x62",
    "0x63","f13","f14","f15","0x67","0x68","0x69","0x6a","0x6b",
    "0x6c","0x6d","0x6e","0x6f","kana","0x71","0x72","abnt_c1",
    "0x74","0x75","0x76","0x77","0x78","convert", "0x7a","noconvert",
    "0x7c","yen","abnt_c2","0x7f","0x80","0x81","0x82","0x83","0x84",
    "0x85","0x86","0x87","0x88","0x89","0x8a","0x8b","0x8c",
    "numpadequals","0x8e","0x8f","prevtrack","at","colon","underline",
    "kanji","stop","ax","unlabeled", "0x98","nexttrack", "0x9a",
    "0x9b","numpadenter","rcontrol", "0x9e", "0x9f","mute","calculater",
    "playpause", "0xa3","mediastop","0xa5","0xa6","0xa7","0xa8","0xa9",
    "0xaa","0xab","0xac","0xad","volumedown", "0xaf","volumeup", "0xb1",
    "webhome","numpadcomma", "0xb4","divide", "0xb6","prtscr","rmenu",
    "0xb9","0xba","0xbb","0xbc","0xbd","0xbe", "0xbf","0xc0","0xc1",
    "0xc2","0xc3","0xc4","pause","0xc6","home","up","pgup","0xca",
    "left", "0xcc","right", "0xce","end","down","pgdown","insert",
    "delete","0xd4","0xd5","0xd6","0xd7","0xd8","0xd9","0xda","lwin",
    "rwin","apps","power","sleep","0xe0","0xe1","0xe2","wake", "0xe4",
    "websearch","webfavorites","webstop","webforward","webback",
    "mycomputer","mail","mediaselect"
};

// OIS to Irrlicht key map. Table position is the OIS keycode. KEY_PA1
// is used when Irrlicht doesn't contain a corresponding OIS key code.
static EKEY_CODE oiKeyMap[238] =
{
//  Irrlicht Key       OIS Key as Index
    KEY_PA1,        // KC_UNASSIGNED  = 0x00,
    KEY_ESCAPE,     // KC_ESCAPE      = 0x01,
    KEY_KEY_1,      // KC_1           = 0x02,
    KEY_KEY_2,      // KC_2           = 0x03,
    KEY_KEY_3,      // KC_3           = 0x04,
    KEY_KEY_4,      // KC_4           = 0x05,
    KEY_KEY_5,      // KC_5           = 0x06,
    KEY_KEY_6,      // KC_6           = 0x07,
    KEY_KEY_7,      // KC_7           = 0x08,
    KEY_KEY_8,      // KC_8           = 0x09,
    KEY_KEY_9,      // KC_9           = 0x0A,
    KEY_KEY_0,      // KC_0           = 0x0B,
    KEY_MINUS,      // KC_MINUS       = 0x0C,    // - on main keyboard
    KEY_PLUS,       // KC_EQUALS      = 0x0D,
    KEY_BACK,       // KC_BACK        = 0x0E,    // backspace
    KEY_TAB,        // KC_TAB         = 0x0F,
    KEY_KEY_Q,      // KC_Q           = 0x10,
    KEY_KEY_W,      // KC_W           = 0x11,
    KEY_KEY_E,      // KC_E           = 0x12,
    KEY_KEY_R,      // KC_R           = 0x13,
    KEY_KEY_T,      // KC_T           = 0x14,
    KEY_KEY_Y,      // KC_Y           = 0x15,
    KEY_KEY_U,      // KC_U           = 0x16,
    KEY_KEY_I,      // KC_I           = 0x17,
    KEY_KEY_O,      // KC_O           = 0x18,
    KEY_KEY_P,      // KC_P           = 0x19,
    KEY_PA1,        // KC_LBRACKET    = 0x1A,
    KEY_PA1,        // KC_RBRACKET    = 0x1B,
    KEY_RETURN,     // KC_RETURN      = 0x1C,    // Enter on main keyboard
    KEY_LCONTROL,   // KC_LCONTROL    = 0x1D,
    KEY_KEY_A,      // KC_A           = 0x1E,
    KEY_KEY_S,      // KC_S           = 0x1F,
    KEY_KEY_D,      // KC_D           = 0x20,
    KEY_KEY_F,      // KC_F           = 0x21,
    KEY_KEY_G,      // KC_G           = 0x22,
    KEY_KEY_H,      // KC_H           = 0x23,
    KEY_KEY_J,      // KC_J           = 0x24,
    KEY_KEY_K,      // KC_K           = 0x25,
    KEY_KEY_L,      // KC_L           = 0x26,
    KEY_PA1,        // KC_SEMICOLON   = 0x27,
    KEY_PA1,        // KC_APOSTROPHE  = 0x28,
    KEY_PA1,        // KC_GRAVE       = 0x29,    // accent
    KEY_LSHIFT,     // KC_LSHIFT      = 0x2A,
    KEY_PA1,        // KC_BACKSLASH   = 0x2B,
    KEY_KEY_Z,      // KC_Z           = 0x2C,
    KEY_KEY_X,      // KC_X           = 0x2D,
    KEY_KEY_C,      // KC_C           = 0x2E,
    KEY_KEY_V,      // KC_V           = 0x2F,
    KEY_KEY_B,      // KC_B           = 0x30,
    KEY_KEY_N,      // KC_N           = 0x31,
    KEY_KEY_M,      // KC_M           = 0x32,
    KEY_COMMA,      // KC_COMMA       = 0x33,
    KEY_PERIOD,     // KC_PERIOD      = 0x34,    // . on main keyboard
    KEY_PA1,        // KC_SLASH       = 0x35,    // / on main keyboard
    KEY_RSHIFT,     // KC_RSHIFT      = 0x36,
    KEY_MULTIPLY,   // KC_MULTIPLY    = 0x37,    // * on numeric keypad
    KEY_LMENU,      // KC_LMENU       = 0x38,    // left Alt
    KEY_SPACE,      // KC_SPACE       = 0x39,
    KEY_CAPITAL,    // KC_CAPITAL     = 0x3A,
    KEY_F1,         // KC_F1          = 0x3B,
    KEY_F2,         // KC_F2          = 0x3C,
    KEY_F3,         // KC_F3          = 0x3D,
    KEY_F4,         // KC_F4          = 0x3E,
    KEY_F5,         // KC_F5          = 0x3F,
    KEY_F6,         // KC_F6          = 0x40,
    KEY_F7,         // KC_F7          = 0x41,
    KEY_F8,         // KC_F8          = 0x42,
    KEY_F9,         // KC_F9          = 0x43,
    KEY_F10,        // KC_F10         = 0x44,
    KEY_NUMLOCK,    // KC_NUMLOCK     = 0x45,
    KEY_SCROLL,     // KC_SCROLL      = 0x46,    // Scroll Lock
    KEY_NUMPAD7,    // KC_NUMPAD7     = 0x47,
    KEY_NUMPAD8,    // KC_NUMPAD8     = 0x48,
    KEY_NUMPAD9,    // KC_NUMPAD9     = 0x49,
    KEY_SUBTRACT,   // KC_SUBTRACT    = 0x4A,    // - on numeric keypad
    KEY_NUMPAD4,    // KC_NUMPAD4     = 0x4B,
    KEY_NUMPAD5,    // KC_NUMPAD5     = 0x4C,
    KEY_NUMPAD6,    // KC_NUMPAD6     = 0x4D,
    KEY_ADD,        // KC_ADD         = 0x4E,    // + on numeric keypad
    KEY_NUMPAD1,    // KC_NUMPAD1     = 0x4F,
    KEY_NUMPAD2,    // KC_NUMPAD2     = 0x50,
    KEY_NUMPAD3,    // KC_NUMPAD3     = 0x51,
    KEY_NUMPAD0,    // KC_NUMPAD0     = 0x52,
    KEY_DECIMAL,    // KC_DECIMAL     = 0x53,    // . on numeric keypad
    KEY_PA1, KEY_PA1, // 0x54, 0x55,
    KEY_PA1,        // KC_OEM_102     = 0x56,    // < > | on UK/Germany keyboards
    KEY_F11,        // KC_F11         = 0x57,
    KEY_F12,        // KC_F12         = 0x58,
    // 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_F13,        // KC_F13         = 0x64,    //                     (NEC PC98)
    KEY_F14,        // KC_F14         = 0x65,    //                     (NEC PC98)
    KEY_F15,        // KC_F15         = 0x66,    //                     (NEC PC98)
    // 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_KANA,       // KC_KANA        = 0x70,    // (Japanese keyboard)
    // 0x71, 0x72,
    KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_ABNT_C1     = 0x73,    // / ? on Portugese (Brazilian) keyboards
    // 0x74, 0x75, 0x76, 0x77, 0x78,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_CONVERT,    // KC_CONVERT     = 0x79,    // (Japanese keyboard)
    KEY_PA1,        // 0x7A,
    KEY_NONCONVERT, // KC_NOCONVERT   = 0x7B,    // (Japanese keyboard)
    KEY_PA1,        // 0x7C,
    KEY_PA1,        // KC_YEN         = 0x7D,    // (Japanese keyboard)
    KEY_PA1,        // KC_ABNT_C2     = 0x7E,    // Numpad . on Portugese (Brazilian) keyboards
    //0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_NUMPADEQUALS= 0x8D,    // = on numeric keypad (NEC PC98)
    // 0x8E, 0x8F,
    KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_PREVTRACK   = 0x90,    // Previous Track (KC_CIRCUMFLEX on Japanese keyboard)
    KEY_PA1,        // KC_AT          = 0x91,    //                     (NEC PC98)
    KEY_PA1,        // KC_COLON       = 0x92,    //                     (NEC PC98)
    KEY_PA1,        // KC_UNDERLINE   = 0x93,    //                     (NEC PC98)
    KEY_KANJI,      // KC_KANJI       = 0x94,    // (Japanese keyboard)
    KEY_PA1,        // KC_STOP        = 0x95,    //                     (NEC PC98)
    KEY_PA1,        // KC_AX          = 0x96,    //                     (Japan AX)
    KEY_PA1,        // KC_UNLABELED   = 0x97,    //                        (J3100)
    KEY_PA1,        // 0x98,
    KEY_PA1,        // KC_NEXTTRACK   = 0x99,    // Next Track
    // 0x9A, 0x9B,
    KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_NUMPADENTER = 0x9C,    // Enter on numeric keypad
    KEY_RCONTROL,   // KC_RCONTROL    = 0x9D,
    //0x9E, 0x9F,
    KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_MUTE        = 0xA0,    // Mute
    KEY_PA1,        // KC_CALCULATOR  = 0xA1,    // Calculator
    KEY_PLAY,       // KC_PLAYPAUSE   = 0xA2,    // Play / Pause
    KEY_PA1,        // 0xA3,
    KEY_PA1,        // KC_MEDIASTOP   = 0xA4,    // Media Stop
    //0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_VOLUMEDOWN  = 0xAE,    // Volume -
    KEY_PA1,        // 0xAF,
    KEY_PA1,        // KC_VOLUMEUP    = 0xB0,    // Volume +
    KEY_PA1,        // 0xB1,
    KEY_PA1,        // KC_WEBHOME     = 0xB2,    // Web home
    KEY_SEPARATOR,  // KC_NUMPADCOMMA = 0xB3,    // , on numeric keypad (NEC PC98)
    KEY_PA1,        // 0xB4,
    KEY_DIVIDE,     // KC_DIVIDE      = 0xB5,    // / on numeric keypad
    KEY_PA1,        // 0xB6,
    KEY_SNAPSHOT,   // KC_SYSRQ       = 0xB7,
    KEY_RMENU,      // KC_RMENU       = 0xB8,    // right Alt
    // 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    // 0xC2, 0xC3, 0xC4,
    KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_PAUSE,      // KC_PAUSE       = 0xC5,    // Pause
    KEY_PA1,        // 0xC6,
    KEY_HOME,       // KC_HOME        = 0xC7,    // Home on arrow keypad
    KEY_UP,         // KC_UP          = 0xC8,    // UpArrow on arrow keypad
    KEY_PRIOR,      // KC_PGUP        = 0xC9,    // PgUp on arrow keypad
    KEY_PA1,        // 0xCA,
    KEY_LEFT,       // KC_LEFT        = 0xCB,    // LeftArrow on arrow keypad
    KEY_PA1,        // 0xCC,
    KEY_RIGHT,      // KC_RIGHT       = 0xCD,    // RightArrow on arrow keypad
    KEY_PA1,        // 0xCE,
    KEY_END,        // KC_END         = 0xCF,    // End on arrow keypad
    KEY_DOWN,       // KC_DOWN        = 0xD0,    // DownArrow on arrow keypad
    KEY_NEXT,       // KC_PGDOWN      = 0xD1,    // PgDn on arrow keypad
    KEY_INSERT,     // KC_INSERT      = 0xD2,    // Insert on arrow keypad
    KEY_DELETE,     // KC_DELETE      = 0xD3,    // Delete on arrow keypad
    // 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
    KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_LWIN,       // KC_LWIN        = 0xDB,    // Left Windows key
    KEY_RWIN,       // KC_RWIN        = 0xDC,    // Right Windows key
    KEY_APPS,       // KC_APPS        = 0xDD,    // AppMenu key
    KEY_PA1,        // KC_POWER       = 0xDE,    // System Power
    KEY_SLEEP,      // KC_SLEEP       = 0xDF,    // System Sleep
    // 0xE0, 0xE1, 0xE2,
    KEY_PA1, KEY_PA1, KEY_PA1,
    KEY_PA1,        // KC_WAKE        = 0xE3,    // System Wake
    KEY_PA1,        // 0xE4,
    KEY_PA1,        // KC_WEBSEARCH   = 0xE5,    // Web Search
    KEY_PA1,        // KC_WEBFAVORITES= 0xE6,    // Web Favorites
    KEY_PA1,        // KC_WEBREFRESH  = 0xE7,    // Web Refresh
    KEY_PA1,        // KC_WEBSTOP     = 0xE8,    // Web Stop
    KEY_PA1,        // KC_WEBFORWARD  = 0xE9,    // Web Forward
    KEY_PA1,        // KC_WEBBACK     = 0xEA,    // Web Back
    KEY_PA1,        // KC_MYCOMPUTER  = 0xEB,    // My Computer
    KEY_PA1,        // KC_MAIL        = 0xEC,    // Mail
    KEY_PA1         // KC_MEDIASELECT = 0xED     // Media Select
};

//
// OIS to Irrlicht mouse event map
//
static EMOUSE_INPUT_EVENT oiMouseMap[2][8]=
{
{EMIE_LMOUSE_LEFT_UP,EMIE_RMOUSE_LEFT_UP,
EMIE_MMOUSE_LEFT_UP, EMIE_LMOUSE_LEFT_UP,
EMIE_LMOUSE_LEFT_UP, EMIE_LMOUSE_LEFT_UP,
EMIE_LMOUSE_LEFT_UP, EMIE_LMOUSE_LEFT_UP},

{EMIE_LMOUSE_PRESSED_DOWN,EMIE_RMOUSE_PRESSED_DOWN,
EMIE_MMOUSE_PRESSED_DOWN, EMIE_LMOUSE_PRESSED_DOWN,
EMIE_LMOUSE_PRESSED_DOWN, EMIE_LMOUSE_PRESSED_DOWN,
EMIE_LMOUSE_PRESSED_DOWN, EMIE_LMOUSE_PRESSED_DOWN}
};

static COIS*    m_cois;
//-----------------------------------------------------------------------
//                              C O I S
//-----------------------------------------------------------------------
COIS::COIS(IrrlichtDevice* idevice, bool showCursor, bool buffered, bool debugEnabled) : m_device(0),
m_gui(0),
m_inputManager(0),
m_windowHandle(0),
m_display(0),
m_showCursor(showCursor),
m_buffered(buffered),
m_debugEnabled(debugEnabled),
m_GUIEnabled(false),
m_GUIExclusive(false),
m_keyboard(0),
m_mouse(0),
m_numSticks(0)
#ifdef _IRR_WINDOWS_
,m_oldWndProc(0)
#endif
{
    m_cois = this;
    m_device = idevice;
    for(u32 i=0;i<MAX_JOYS;i++)
    {
        m_joys[i] = 0;
        m_ff[i] = 0;
    }
}

//-----------------------------------------------------------------------
//                             ~ C O I S
//-----------------------------------------------------------------------
COIS::~COIS()
{

    if( m_inputManager )
    {
        if(m_keyboard)
            m_inputManager->destroyInputObject( m_keyboard );
        if(m_mouse)
            m_inputManager->destroyInputObject( m_mouse );
        InputManager::destroyInputSystem(m_inputManager);
    }

#ifdef _IRR_WINDOWS_
    // restore wndproc
    SetWindowLongPtr((HWND)m_windowHandle, GWLP_WNDPROC, (LONG)m_oldWndProc);
#endif

}

//-----------------------------------------------------------------------
//                            _ w n d P r o c
//-----------------------------------------------------------------------
#ifdef _IRR_WINDOWS_
LRESULT COIS::_wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        {
            // resize
            m_cois->setDisplaySize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
#endif

//-----------------------------------------------------------------------
//                          i n i t i a l i z e
//-----------------------------------------------------------------------
int COIS::initialize()
{
    int result=0;
    unsigned int* p=(unsigned int*)&m_windowHandle;
    char buf[32];
    ParamList pl;

    if(!m_device)
        return 1;

    m_gui = m_device->getGUIEnvironment();

#ifdef _IRR_WINDOWS_
    //
    // replace irrlicht wndproc with ours to make ois work properly.
    //
    HWND hwnd = (HWND)(m_device->getVideoDriver()->getExposedVideoData().OpenGLWin32.HWnd);
    m_oldWndProc = GetWindowLongPtr(hwnd, GWL_WNDPROC);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)((LONG_PTR)_wndProc));
    m_windowHandle = (void *)hwnd;
    //Default mode is foreground exclusive..but, we want to show mouse - so nonexclusive
    pl.insert(std::make_pair("w32_mouse", "DISCL_FOREGROUND"));

    if(m_showCursor)
        pl.insert(std::make_pair("w32_mouse", "DISCL_NONEXCLUSIVE"));
    else pl.insert(std::make_pair("w32_mouse", "DISCL_EXCLUSIVE"));
#else

    m_windowHandle = (void *)m_device->getVideoDriver()->getExposedVideoData().OpenGLLinux.X11Window;
    m_display = (int)m_device->getVideoDriver()->getExposedVideoData().OpenGLLinux.X11Display;
    pl.insert(std::make_pair("x11_keyboard_grab","false"));
    if(m_showCursor)
    {
        pl.insert(std::make_pair("x11_mouse_grab","false"));
        pl.insert(std::make_pair("x11_mouse_hide","false"));
    }
    else
    {
        pl.insert(std::make_pair("x11_mouse_grab","true"));
        pl.insert(std::make_pair("x11_mouse_hide","true"));
    }
#endif


    sprintf(buf,"%u",*p);
    pl.insert(std::make_pair( "WINDOW", buf ));


    //This never returns null.. it will raise an exception on errors

    m_inputManager = InputManager::createInputSystem(pl);

    unsigned int v = m_inputManager->getVersionNumber();

    if(m_debugEnabled)
    {
        printf("OIS Version: %d.%d.%d\n", (v>>16 ),((v>>8) & 0x000000FF),(v & 0x000000FF));
        printf("Release Name: %s\n", m_inputManager->getVersionName().c_str());
        printf("Manager: %s\n", m_inputManager->inputSystemName().c_str());
        printf("Total Keyboards: %d\n", m_inputManager->getNumberOfDevices(OISKeyboard));
        printf("Total Mice: %d\n", m_inputManager->getNumberOfDevices(OISMouse));
        printf("Total JoySticks: %d\n", m_inputManager->getNumberOfDevices(OISJoyStick));
    }

    //List all devices
    OIS::DeviceList list = m_inputManager->listFreeDevices();
    for( DeviceList::iterator i = list.begin(); i != list.end(); ++i )
    {
        if(m_debugEnabled)
            printf("   Device: %s, Vendor %s\n",g_DeviceType[i->first],i->second.c_str());
    }

    //Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
    m_numKeyboards = m_inputManager->getNumberOfDevices(OIS::OISKeyboard);
    m_keyboard = static_cast<Keyboard*>(m_inputManager->createInputObject( OISKeyboard, m_buffered ));

    m_numMice = m_inputManager->getNumberOfDevices(OIS::OISMouse);
    m_mouse = static_cast<Mouse*>(m_inputManager->createInputObject( OISMouse, m_buffered ));

    m_keyboard->setEventCallback( this );
    m_mouse->setEventCallback( this );

    try
    {
        //This demo uses at most MAX_JOYS joysticks
        m_numSticks = m_inputManager->getNumberOfDevices(OISJoyStick);
        if(m_numSticks > MAX_JOYS)
            m_numSticks = MAX_JOYS;
        for( u32 i = 0; i < m_numSticks; ++i )
        {
            m_joys[i] = (JoyStick*)m_inputManager->createInputObject( OISJoyStick, m_buffered );
            m_joys[i]->setEventCallback( this );

            if(m_debugEnabled)
                printf("\nJoystick %d - ", i+1);
            //Check for FF, if so, dump info
            m_ff[i] = (ForceFeedback*)m_joys[i]->queryInterface( Interface::ForceFeedback );
            if( m_ff[i] )
            {
                if(m_debugEnabled)
                    printf(" Has FF Support!, Effects:\n");
                const ForceFeedback::SupportedEffectList &list = m_ff[i]->getSupportedEffects();
                ForceFeedback::SupportedEffectList::const_iterator i = list.begin(),
                    e = list.end();
                if(m_debugEnabled)
                {
                    for( ; i != e; ++i)
                    {
                        printf("   Force=%s, Type=%s\n",SForce[i->first].c_str(),SType[i->second].c_str());
                    }
                }
            }
            else if(m_debugEnabled)

                printf("No FF Support\n");
        }
    }
    catch(OIS::Exception &ex)
    {
        if(m_debugEnabled)
            printf("Exception raised on joystick creation: %s\n", ex.eText);
    }

    //
    // set OIS display size
    //
    dimension2du dims = m_device->getVideoDriver()->getScreenSize();
    setDisplaySize(dims.Width,dims.Height);

    return result;
}

//-----------------------------------------------------------------------
//                     g e t I r r K e y C o d e
//-----------------------------------------------------------------------
EKEY_CODE COIS::getIrrKeyCode(OIS::KeyCode key)
{
    return oiKeyMap[key];
}

//-----------------------------------------------------------------------
//                       g e t K e y S t r i n g
//-----------------------------------------------------------------------
irr::core::stringc COIS::getKeyString(const OIS::KeyCode key)
{
    return scancodes[key];
}

//-----------------------------------------------------------------------
//                     g e t I r r M o u s e E v e n t
//-----------------------------------------------------------------------
EMOUSE_INPUT_EVENT COIS::getIrrMouseEvent(OIS::MouseButtonID id, bool pressed)
{
    return oiMouseMap[pressed][id];
}

//-----------------------------------------------------------------------
//                     h a s F o r c e F e e d b a c k
//-----------------------------------------------------------------------
bool COIS::hasForceFeedback(u32 stickNumber)
{
    if( stickNumber > MAX_JOYS)
        return false;

    return (m_ff[stickNumber] != 0);
}

//-----------------------------------------------------------------------
//                        s e t S c r e e n S i z e
//-----------------------------------------------------------------------
void COIS::setDisplaySize(int width, int height)
{
    m_mouse->getMouseState().width = width;
    m_mouse->getMouseState().height = height;
}

//-----------------------------------------------------------------------
//                              c a p t u r e
//-----------------------------------------------------------------------
void COIS::capture()
{
    if(m_keyboard)
        m_keyboard->capture();
    if(m_mouse)
        m_mouse->capture();
    for(u32 i=0; i<m_numSticks; i++)
    {
        m_joys[i]->capture();
    }
}

//-----------------------------------------------------------------------
//                         i s K e y D o w n
//-----------------------------------------------------------------------
bool COIS::isKeyDown(OIS::KeyCode key)
{
    return m_keyboard->isKeyDown(key);
}

//-----------------------------------------------------------------------
//                           k e y P r e s s e d
//-----------------------------------------------------------------------
bool COIS::keyPressed( const OIS::KeyEvent& arg )
{

    if(m_debugEnabled)
    {
        stringc info = "key.down.";
        info += scancodes[arg.key];
        printf("OIS: %s\n", info.c_str());
    }

    if(m_GUIEnabled)
    {
        irr::SEvent event;
        event.EventType = irr::EET_KEY_INPUT_EVENT;
        event.KeyInput.Key = oiKeyMap[arg.key];
        event.KeyInput.Char = arg.text;
        event.KeyInput.PressedDown = true;
        event.KeyInput.Shift = m_keyboard->isModifierDown(OIS::Keyboard::Shift);
        event.KeyInput.Control = m_keyboard->isModifierDown(OIS::Keyboard::Ctrl);

        if(m_gui && (m_gui->postEventFromUser(event) || m_GUIExclusive))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------
//                        k e y R e l e a s e d
//-----------------------------------------------------------------------
bool COIS::keyReleased( const OIS::KeyEvent& arg )
{
    if(m_debugEnabled)
    {
        stringc info = "key.up.";
        info += scancodes[arg.key];
        printf("OIS: %s\n", info.c_str());
    }

    if(m_GUIEnabled)
    {
        irr::SEvent event;
        event.EventType = irr::EET_KEY_INPUT_EVENT;
        event.KeyInput.Key = oiKeyMap[arg.key];
        event.KeyInput.Char = arg.text;
        event.KeyInput.PressedDown = false;
        event.KeyInput.Shift = m_keyboard->isModifierDown(OIS::Keyboard::Shift);
        event.KeyInput.Control = m_keyboard->isModifierDown(OIS::Keyboard::Ctrl);

        if(m_gui && (m_gui->postEventFromUser(event) || m_GUIExclusive))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------
//                        m o u s e M o v e d
//-----------------------------------------------------------------------
bool COIS::mouseMoved( const OIS::MouseEvent &arg )
{
    static int lastX=0, lastY=0, lastZ=0;
    bool result=false;

    if(m_debugEnabled)
    {
        printf("OIS: mouse.move (%d,%d,%d)\n", arg.state.X.abs, arg.state.Y.abs,
            arg.state.Z.abs);
    }

    if(m_GUIEnabled)
    {
        irr::SEvent event;
        event.EventType = irr::EET_MOUSE_INPUT_EVENT;

        if (m_gui)
        {
            // post moved event ?
            if( (lastX != arg.state.X.abs) ||
                (lastY != arg.state.Y.abs) )
            {
                event.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
                event.MouseInput.X = arg.state.X.abs;
                event.MouseInput.Y = arg.state.Y.abs;
                result = m_gui->postEventFromUser(event);
                lastX = event.MouseInput.X;
                lastY = event.MouseInput.Y;
            }

            //post wheel event ?
            if( lastZ != arg.state.Z.abs )
            {
                event.EventType = irr::EET_MOUSE_INPUT_EVENT;
                event.MouseInput.Event = EMIE_MOUSE_WHEEL;
                event.MouseInput.Wheel = ((irr::f32)((lastZ - arg.state.Z.abs))) / (irr::f32)WHEEL_DELTA;
                result |= m_gui->postEventFromUser(event);
                lastZ = arg.state.Z.abs;
            }
        }

        if(result || m_GUIExclusive)
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------
//                        m o u s e P r e s s e d
//-----------------------------------------------------------------------
bool COIS::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if(m_debugEnabled)
    {
        printf("OIS: mouse.pressed(%d) (%d,%d)\n", id, arg.state.X.abs, arg.state.Y.abs);
    }

    if(m_GUIEnabled)
    {
        irr::SEvent event;
        event.EventType = irr::EET_MOUSE_INPUT_EVENT;
        event.MouseInput.Event = oiMouseMap[1][id];
        event.MouseInput.X = arg.state.X.abs;
        event.MouseInput.Y = arg.state.Y.abs;

        if(m_gui && (m_gui->postEventFromUser(event) || m_GUIExclusive))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------
//                      m o u s e R e l e a s e d
//-----------------------------------------------------------------------
bool COIS::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if(m_debugEnabled)
    {
        printf("OIS: mouse.released(%d) (%d,%d)\n", id, arg.state.X.abs, arg.state.Y.abs);
    }

    if(m_GUIEnabled)
    {
        irr::SEvent event;
        event.EventType = irr::EET_MOUSE_INPUT_EVENT;
        event.MouseInput.Event = oiMouseMap[0][id];
        event.MouseInput.X = arg.state.X.abs;
        event.MouseInput.Y = arg.state.Y.abs;

        if(m_gui && (m_gui->postEventFromUser(event) || m_GUIExclusive))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------
//                      b u t t o n P r e s s e d
//-----------------------------------------------------------------------
bool COIS::buttonPressed( const OIS::JoyStickEvent &arg, int button )
{
    if(m_debugEnabled)
    {
        printf("OIS: joy.buttonPressed(%d)\n", button);
    }
    return false;
}

//-----------------------------------------------------------------------
//                      b u t t o n R e l e a s e d
//-----------------------------------------------------------------------
bool COIS::buttonReleased( const OIS::JoyStickEvent &arg, int button )
{
    if(m_debugEnabled)
    {
        printf("OIS: joy.buttonReleased(%d)\n", button);
    }
    return false;
}

//-----------------------------------------------------------------------
//                          a x i s M o v e d
//-----------------------------------------------------------------------
bool COIS::axisMoved( const OIS::JoyStickEvent &arg, int axis )
{
    if(m_debugEnabled)
    {
        printf("OIS: joy.axisMoved(%d) (%d)\n", axis,
            arg.state.mAxes[axis].abs);
    }
    return false;
}

//-----------------------------------------------------------------------
//                           p o v M o v e d
//-----------------------------------------------------------------------
bool COIS::povMoved( const OIS::JoyStickEvent &arg, int pov )
{
    if(m_debugEnabled)
    {
        printf("OIS: joy.povMoved(%d), direction=%d\n", pov, arg.state.mPOV[pov].direction);
    }
    return false;
}
