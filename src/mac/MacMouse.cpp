#include "mac/MacMouse.h"
#include "mac/MacInputManager.h"
#include "mac/MacHelpers.h"
#include "OISException.h"
#include "OISEvents.h"

#include <Carbon/Carbon.h>

#include <list>

#include <iostream>

using namespace OIS;

//Events we subscribe to and remove from queue
const EventTypeSpec mouseEvents[] = {
	{ kEventClassMouse, kEventMouseDown },
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassMouse, kEventMouseDragged },
	{ kEventClassMouse, kEventMouseWheelMoved }
};

const EventTypeSpec WinFocusAcquired [] = {{kEventClassApplication, kEventAppDeactivated}};

//-------------------------------------------------------------------//
MacMouse::MacMouse( InputManager* creator, bool buffered, bool nonExclusive )
	: Mouse(creator->inputSystemName(), buffered, 0, creator), mNeedsToRegainFocus( false )
{
    mouseEventRef = NULL;
	mWindowFocusHandler = NULL;

    // Get a "Univeral procedure pointer" for our callback
    mouseUPP = NewEventHandlerUPP(MouseWrapper);
	mWindowFocusListener = NewEventHandlerUPP(WindowFocusChanged);

	mMouseNonExclusive = nonExclusive;

	static_cast<MacInputManager*>(mCreator)->_setMouseUsed(true);
}

MacMouse::~MacMouse()
{
    if(mouseEventRef != NULL)
		RemoveEventHandler(mouseEventRef);

	if(mWindowFocusHandler != NULL)
		RemoveEventHandler(mWindowFocusHandler);

	DisposeEventHandlerUPP(mouseUPP);
	DisposeEventHandlerUPP(mWindowFocusListener);

	// Restore Mouse
	CGAssociateMouseAndMouseCursorPosition(TRUE);
	CGDisplayShowCursor(kCGDirectMainDisplay);

	static_cast<MacInputManager*>(mCreator)->_setMouseUsed(false);
}

void MacMouse::_initialize()
{
	mState.clear();
	mTempState.clear();
	mMouseWarped = false;

	// Hide OS Mouse
	if(!mMouseNonExclusive)
    {
        CGDisplayHideCursor(kCGDirectMainDisplay);
    }

	MacInputManager* im = static_cast<MacInputManager*>(mCreator);
	WindowRef win = im->_getWindow();

	if(win)
	{
		Rect clipRect = {0.0f, 0.0f, 0.0f, 0.0f};
		GetWindowBounds(win, kWindowContentRgn, &clipRect);

        if(!mMouseNonExclusive)
        {
            CGPoint warpPoint;
            warpPoint.x = ((clipRect.right - clipRect.left) / 2) + clipRect.left;
            warpPoint.y = ((clipRect.bottom - clipRect.top) / 2) + clipRect.top;
            CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, warpPoint); //Place at display origin*
        }

		mMouseWarped = true;
	}

	//Now that mouse is warped, start listening for events
	EventTargetRef event = ((MacInputManager*)mCreator)->_getEventTarget();

	if(mouseEventRef != NULL)
		RemoveEventHandler(mouseEventRef);

	if(mWindowFocusHandler != NULL)
		RemoveEventHandler(mWindowFocusHandler);

	mouseEventRef = mWindowFocusHandler = NULL;

	if(InstallEventHandler(event, mouseUPP, GetEventTypeCount(mouseEvents), mouseEvents, this, &mouseEventRef) != noErr)
		OIS_EXCEPT( E_General, "MacMouse::_initialize >> Error loading Mouse event handler" );

	if(InstallEventHandler(event, mWindowFocusListener, GetEventTypeCount(WinFocusAcquired), WinFocusAcquired, this, &mWindowFocusHandler) != noErr)
		OIS_EXCEPT( E_General, "MacMouse::_initialize >> Error loading Mouse event handler" );

	//Lock OS Mouse movement
	mNeedsToRegainFocus = false;
	if(!mMouseNonExclusive)
    {
        CGAssociateMouseAndMouseCursorPosition(FALSE);
    }
}

OSStatus MacMouse::WindowFocusChanged(EventHandlerCallRef nextHandler, EventRef event, void* macMouse)
{
	//std::cout << "Window Focus Changed\n";

	MacMouse* _this = static_cast<MacMouse*>(macMouse);
    if (_this)
	{
		_this->mNeedsToRegainFocus = true;
		CGAssociateMouseAndMouseCursorPosition(TRUE);

        // propagate the event down the chain
        return CallNextEventHandler(nextHandler, event);
    }
    else
        OIS_EXCEPT(E_General, "MouseWrapper >> Being called by something other than our event handler!");
}

void MacMouse::setBuffered( bool buffered )
{
	mBuffered = buffered;
}

void MacMouse::capture()
{
	mState.X.rel = 0;
	mState.Y.rel = 0;
	mState.Z.rel = 0;

	if(mTempState.X.rel || mTempState.Y.rel || mTempState.Z.rel)
	{

        if(!mMouseNonExclusive)
        {
            //Update absolute position when using non exclusives
            mState.X.abs += mTempState.X.rel;
            mState.Y.abs += mTempState.Y.rel;

            //Make sure crap don't go off the screen.
            if(mState.X.abs > mState.width)
                mState.X.abs = mState.width;
            else if(mState.X.abs < 0)
                mState.X.abs = 0;

            if(mState.Y.abs > mState.height)
                mState.Y.abs = mState.height;
            else if(mState.Y.abs < 0)
                mState.Y.abs = 0;
        }


        //update mState with temps generated in _mouseCallback....
        mState.X.rel = mTempState.X.rel;
        mState.Y.rel = mTempState.Y.rel;
        mState.Z.rel = mTempState.Z.rel;

		mState.Z.abs += mTempState.Z.rel;

		//Fire off event
		if(mListener && mBuffered)
			mListener->mouseMoved(MouseEvent(this, mState));
	}





	mTempState.clear();
}

void MacMouse::_mouseCallback( EventRef theEvent )
{
	OSStatus result = eventNotHandledErr;
    UInt32 kind = GetEventKind (theEvent);

	switch(kind)
	{
		case kEventMouseDragged:
		case kEventMouseMoved:
		{


		    if(mNeedsToRegainFocus)
			{
			    break;
			}

			if(!mMouseWarped)
            {
                if(mMouseNonExclusive)//Non-exclusive mouse mode.
                {
                    HIPoint location = {0.0f, 0.0f};
                    GetEventParameter(theEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);

                    //START OPTIMIZE (>',')> We may not need to really call this all the time. How often does window manager decoration (v','v)
                    //    (>',')> change after the program has launched in OSX? making clipRect a member would speed up the lib...<(','<)
                    MacInputManager* im = static_cast<MacInputManager*>(mCreator);
                    WindowRef win = im->_getWindow();
                    if(win)
                    {
                        HIRect clipRect = {0.0f, 0.0f, 0.0f, 0.0f}; // <(','<) This what i'm talking about.... (^','^)

                        HIWindowGetBounds(win, kWindowContentRgn, kHICoordSpaceWindow, &clipRect);
                        //END OPTIMIZE (^','^) End of Optimization Recommendation (^','^)


                        mState.X.abs = location.x;
                        mState.Y.abs = location.y - clipRect.origin.y; //Window border offsets mouse pointer.

                    }


                }

                //OPTIMIZE (>',')> We could eliminate delta and relitive generation in non-exclusive mode IF we generate (v','v)
                //    (>',')> relitive values from the non-exclusive stuff... <(','<)
                HIPoint delta = {0.0f, 0.0f};

                GetEventParameter(theEvent, kEventParamMouseDelta, typeHIPoint, NULL, sizeof(HIPoint), NULL, &delta);

                mTempState.X.rel = delta.x;
                mTempState.Y.rel = delta.y;

            }


                mMouseWarped = false;

			break;
		}
		case kEventMouseDown:
		{
			EventMouseButton button = 0;
			int mouseButton = 3;
			UInt32 modifiers = 0;

			if(mNeedsToRegainFocus)
				break;

			// Capture parameters
			GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
			GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);

			if((button == kEventMouseButtonTertiary) || ((button == kEventMouseButtonPrimary) && (modifiers & optionKey)))
			{
				mouseButton = 2;
				mState.buttons |= 1 << mouseButton;
			}
            else if((button == kEventMouseButtonSecondary) || ((button == kEventMouseButtonPrimary) && (modifiers & controlKey)))
            {
                mouseButton = 1;
                mState.buttons |= 1 << mouseButton;
            }
            else if(button == kEventMouseButtonPrimary)
            {
                mouseButton = 0;
                mState.buttons |= 1 << mouseButton;
            }

            if( mListener && mBuffered )
                mListener->mousePressed( MouseEvent( this, mState ), (MouseButtonID)mouseButton );

            break;
		}
		case kEventMouseUp:
		{
			EventMouseButton button = 0;
			int mouseButton = 3;
			UInt32 modifiers = 0;

			if(mNeedsToRegainFocus)
			{
				mNeedsToRegainFocus = false;
				CGAssociateMouseAndMouseCursorPosition(false);

				MacInputManager* im = static_cast<MacInputManager*>(mCreator);
				WindowRef win = im->_getWindow();

				if(win)
				{
					Rect clipRect = {0.0f, 0.0f, 0.0f, 0.0f};
					GetWindowBounds(win, kWindowContentRgn, &clipRect);

                    if(!mMouseNonExclusive)
                    {
                        CGPoint warpPoint;
                        warpPoint.x = ((clipRect.right - clipRect.left) / 2) + clipRect.left;
                        warpPoint.y = ((clipRect.bottom - clipRect.top) / 2) + clipRect.top;
                        CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, warpPoint); //Place at display origin*/

                        CGDisplayHideCursor(kCGDirectMainDisplay);
                    }

					mMouseWarped = true;
				}

				//Once we regain focus, we do not really know what state all the buttons are in - for now, set to not pressed. todo, check current status
				//compare against old status, and send off any needed events
				mState.buttons = 0;

				break;
			}

			// Capture parameters
			GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
			GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);

			if ((button == kEventMouseButtonTertiary) || ((button == kEventMouseButtonPrimary) && (modifiers & optionKey)))
			{
				mouseButton = 2;
				mState.buttons &= ~(1 << mouseButton);
			}
            else if ((button == kEventMouseButtonSecondary) || ((button == kEventMouseButtonPrimary) && (modifiers & controlKey)))
            {
                mouseButton = 1;
                mState.buttons &= ~(1 << mouseButton);
            }
            else if (button == kEventMouseButtonPrimary)
            {
                mouseButton = 0;
                mState.buttons &= ~(1 << mouseButton);
            }

            if( mListener && mBuffered )
                mListener->mouseReleased( MouseEvent( this, mState ), (MouseButtonID)mouseButton );

            break;
		}
		case kEventMouseWheelMoved:
		{
			SInt32 wheelDelta = 0;
			EventMouseWheelAxis	wheelAxis = 0;

			// Capture parameters
			GetEventParameter(theEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(EventMouseWheelAxis), NULL, &wheelAxis);
			GetEventParameter(theEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof(SInt32), NULL, &wheelDelta);

			// If the Y axis of the wheel changed, then update the Z
			// Does OIS care about the X wheel axis?
			if(wheelAxis == kEventMouseWheelAxisY)
				mTempState.Z.rel += (wheelDelta * 60);

            break;
		}
		default:
			break;
	}
}
