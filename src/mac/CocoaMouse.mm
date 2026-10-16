/*
The zlib/libpng License

Copyright (c) 2018 Arthur Brainville
Copyright (c) 2015 Andrew Fenn
Copyright (c) 2005-2010 Phillip Castaneda (pjcast -- www.wreckedgames.com)

This software is provided 'as-is', without any express or implied warranty. In no
event will the authors be held liable for any damages arising from the use of this
software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to the
following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that
        you wrote the original software. If you use this software in a product,
        an acknowledgment in the product documentation would be appreciated
        but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution. 
 */
// Altered source: focus resets, fractional motion and application canvas coordinates.
#include "mac/CocoaMouse.h"
#include "mac/CocoaInputManager.h"
#include "mac/CocoaHelpers.h"
#include "OISException.h"
#include "OISEvents.h"

using namespace OIS;

//-------------------------------------------------------------------//
CocoaMouse::CocoaMouse(InputManager* creator, bool buffered) :
 Mouse(creator->inputSystemName(), buffered, 0, creator)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	CocoaInputManager* man = static_cast<CocoaInputManager*>(mCreator);
	mResponder			   = [[CocoaMouseView alloc] initWithFrame:[[man->_getWindow() contentView] bounds]];
	if(!mResponder)
		OIS_EXCEPT(E_General, "CocoaMouseView::CocoaMouseView >> Error creating event responder");

	[mResponder setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[[man->_getWindow() contentView] addSubview:mResponder];
	[man->_getWindow() setAcceptsMouseMovedEvents:YES];
	[mResponder setOISMouseObj:this];
	[mResponder observeWindow:man->_getWindow()];

	static_cast<CocoaInputManager*>(mCreator)->_setMouseUsed(true);

	[pool drain];
}

CocoaMouse::~CocoaMouse()
{
	// Restore only a disassociation successfully requested by this responder.
	[mResponder restoreAssociation];
	CGDisplayShowCursor(kCGDirectMainDisplay);

	if(mResponder)
	{
		[mResponder detach];
		[mResponder removeFromSuperview];
		[mResponder release];
		mResponder = nil;
	}

	static_cast<CocoaInputManager*>(mCreator)->_setMouseUsed(false);
}

void CocoaMouse::_initialize()
{
	mState.clear();
	[mResponder beginCapture];
}

void CocoaMouse::setBuffered(bool buffered)
{
	mBuffered = buffered;
}

void CocoaMouse::capture()
{
	[mResponder capture];
}

@implementation CocoaMouseView

- (id)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if(self) {
		mTempState.clear();
		mDeltaX = mDeltaY = 0.0;
		mNeedsToRegainFocus = false;

		CGDisplayHideCursor(kCGDirectMainDisplay);

		// Use NSTrackingArea to track mouse move events
		NSTrackingAreaOptions trackingOptions = NSTrackingMouseMoved | NSTrackingEnabledDuringMouseDrag | NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp | NSTrackingInVisibleRect;

		NSDictionary* trackerData	= [NSDictionary dictionaryWithObjectsAndKeys:
														 [NSNumber numberWithInt:0], @"OISMouseTrackingKey", nil];
		NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
			initWithRect:NSZeroRect // NSTrackingInVisibleRect follows bounds and resizing
				 options:trackingOptions
				   owner:self
				userInfo:trackerData];
		[self addTrackingArea:trackingArea];
		[trackingArea release];
	}
	return self;
}

- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent
{
	return YES;
}

- (void)setOISMouseObj:(CocoaMouse*)obj
{
	oisMouseObj = obj;
}

- (void)observeWindow:(NSWindow*)window
{
	observedWindow = [window retain];
	NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
	[center addObserver:self selector:@selector(focusLost:) name:NSApplicationDidResignActiveNotification object:nil];
	if(window)
		[center addObserver:self selector:@selector(focusLost:) name:NSWindowDidResignKeyNotification object:window];
}

- (void)detach
{
	[self restoreAssociation];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	oisMouseObj = 0;
	pendingButtons.clear();
	[observedWindow release];
	observedWindow = nil;
}

- (void)dealloc
{
	[self detach];
	[super dealloc];
}

- (void)focusLost:(NSNotification*)notification
{
	if(!oisMouseObj)
		return;
	if(focusListener == oisMouseObj->getEventCallback())
		releaseButtons |= deliveredButtons;
	deliveredButtons = 0;
	pendingButtons.clear();
	deferButtons = true;
	mTempState.clear();
	mDeltaX = mDeltaY = 0.0;
	positionChanged = false;
	MouseState* state = oisMouseObj->getMouseStatePtr();
	state->buttons = 0;
	state->X.rel = state->Y.rel = state->Z.rel = 0;
	++focusGeneration;
}

- (void)deliverButton:(MouseButtonID)button pressed:(bool)pressed
{
	[self refreshCoordinates];
	MouseState snapshot = *oisMouseObj->getMouseStatePtr();
	snapshot.X.rel = snapshot.Y.rel = snapshot.Z.rel = 0;
	MouseListener* listener = oisMouseObj->getEventCallback();
	if(focusListener != listener || !oisMouseObj->buffered() || !listener)
	{
		deliveredButtons = releaseButtons = 0;
		pendingButtons.clear();
		focusListener = listener;
	}
	if(!oisMouseObj->buffered() || !listener)
		return;
	if(deferButtons)
	{
		// Overflow discards undelivered input and resets polling state again.
		if(pendingButtons.size() == 32)
			[self focusLost:nil];
		else
			pendingButtons.push_back(std::make_pair(button, snapshot));
		return;
	}
	const int bit = 1 << button;
	if(pressed)
		deliveredButtons |= bit;
	else
		deliveredButtons &= ~bit;
	if(pressed)
		listener->mousePressed(MouseEvent(oisMouseObj, snapshot), button);
	else
		listener->mouseReleased(MouseEvent(oisMouseObj, snapshot), button);
}

- (void)capture
{
	if(!oisMouseObj || capturing)
		return;
	[self retain];
	capturing = true;
	[self refreshCoordinates];
	MouseListener* listener = oisMouseObj->getEventCallback();
	if(focusListener != listener || !oisMouseObj->buffered() || !listener)
	{
		deliveredButtons = releaseButtons = 0;
		pendingButtons.clear();
		focusListener = listener;
	}
	const unsigned int generation = focusGeneration;
	for(unsigned int button = 0; listener && button != 8; ++button)
	{
		if(releaseButtons & (1 << button))
		{
			releaseButtons &= ~(1 << button);
			MouseState released = *oisMouseObj->getMouseStatePtr();
			released.buttons = 0;
			released.X.rel = released.Y.rel = released.Z.rel = 0;
			listener->mouseReleased(MouseEvent(oisMouseObj, released), (MouseButtonID)button);
			if(!oisMouseObj || focusGeneration != generation || !oisMouseObj->buffered() || oisMouseObj->getEventCallback() != listener)
				break;
		}
	}
	size_t remaining = pendingButtons.size();
	while(remaining-- && oisMouseObj && focusGeneration == generation && oisMouseObj->buffered() &&
		oisMouseObj->getEventCallback() == listener && !pendingButtons.empty())
	{
		const std::pair<MouseButtonID, MouseState> event = pendingButtons.front();
		pendingButtons.pop_front();
		const int bit = 1 << event.first;
		if(event.second.buttonDown(event.first))
		{
			deliveredButtons |= bit;
			listener->mousePressed(MouseEvent(oisMouseObj, event.second), event.first);
		}
		else
		{
			deliveredButtons &= ~bit;
			listener->mouseReleased(MouseEvent(oisMouseObj, event.second), event.first);
		}
	}
	if(!oisMouseObj || focusGeneration != generation || oisMouseObj->getEventCallback() != listener)
	{
		capturing = false;
		[self release];
		return;
	}
	deferButtons = !pendingButtons.empty();
	[self refreshCoordinates];
	const bool moved = positionChanged;
	positionChanged = false;
	// Publish whole counts once per capture, retaining signed fractional motion.
	const int deltaX = static_cast<int>(mDeltaX);
	const int deltaY = static_cast<int>(mDeltaY);
	mTempState.X.rel += deltaX;
	mTempState.Y.rel += deltaY;
	mDeltaX -= deltaX;
	mDeltaY -= deltaY;

	const MouseState captured = mTempState;
	mTempState.clear();
	MouseState* state = oisMouseObj->getMouseStatePtr();
	state->X.rel	  = 0;
	state->Y.rel	  = 0;
	state->Z.rel	  = 0;

	if(moved || captured.X.rel || captured.Y.rel || captured.Z.rel)
	{
		//     NSLog(@"%i %i %i", captured.X.rel, captured.Y.rel, captured.Z.rel);

		// Set new relative motion values
		state->X.rel = captured.X.rel;
		state->Y.rel = captured.Y.rel;
		state->Z.rel = captured.Z.rel;

		state->Z.abs += captured.Z.rel;

		//Fire off event
		if(oisMouseObj->buffered() && oisMouseObj->getEventCallback())
			oisMouseObj->getEventCallback()->mouseMoved(MouseEvent(oisMouseObj, *state));
	}

	capturing = false;
	[self release];
}

#pragma mark Left Mouse Event overrides
- (void)mouseDown:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Left;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

	if(mNeedsToRegainFocus)
		return;

    if((type == NSEventTypeLeftMouseDown) && ([theEvent modifierFlags] & NSEventModifierFlagOption))
	{
		mouseButton = MB_Middle;
	}
    else if((type == NSEventTypeLeftMouseDown) && ([theEvent modifierFlags] & NSEventModifierFlagControl))
	{
		mouseButton = MB_Right;
	}
    else if(type == NSEventTypeLeftMouseDown)
	{
		mouseButton = MB_Left;
	}
	state->buttons |= 1 << mouseButton;
	[self deliverButton:(MouseButtonID)mouseButton pressed:true];
}

- (void)mouseUp:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Left;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

    if((type == NSEventTypeLeftMouseUp) && ([theEvent modifierFlags] & NSEventModifierFlagOption))
	{
		mouseButton = MB_Middle;
	}
    else if((type == NSEventTypeLeftMouseUp) && ([theEvent modifierFlags] & NSEventModifierFlagControl))
	{
		mouseButton = MB_Right;
	}
    else if(type == NSEventTypeLeftMouseUp)
	{
		mouseButton = MB_Left;
	}
	if(!state->buttonDown((MouseButtonID)mouseButton))
		return;
	state->buttons &= ~(1 << mouseButton);

	[self deliverButton:(MouseButtonID)mouseButton pressed:false];
}

- (void)mouseDragged:(NSEvent*)theEvent
{
	[self updateMotion:theEvent];
}

#pragma mark Right Mouse Event overrides
- (void)rightMouseDown:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Right;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

	if(mNeedsToRegainFocus)
		return;

    if(type == NSEventTypeRightMouseDown)
	{
		state->buttons |= 1 << mouseButton;
	}

	[self deliverButton:(MouseButtonID)mouseButton pressed:true];
}

- (void)rightMouseUp:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Right;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

    if(type == NSEventTypeRightMouseUp)
	{
		if(!state->buttonDown((MouseButtonID)mouseButton))
			return;
		state->buttons &= ~(1 << mouseButton);
	}

	[self deliverButton:(MouseButtonID)mouseButton pressed:false];
}

- (void)rightMouseDragged:(NSEvent*)theEvent
{
	[self updateMotion:theEvent];
}

#pragma mark Other Mouse Event overrides
- (void)otherMouseDown:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Middle;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

	if(mNeedsToRegainFocus)
		return;

    if(type == NSEventTypeOtherMouseDown)
	{
		state->buttons |= 1 << mouseButton;
	}

	[self deliverButton:(MouseButtonID)mouseButton pressed:true];
}

- (void)otherMouseUp:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	int mouseButton   = MB_Middle;
	NSEventType type  = [theEvent type];
	MouseState* state = oisMouseObj->getMouseStatePtr();

    if(type == NSEventTypeOtherMouseUp)
	{
		if(!state->buttonDown((MouseButtonID)mouseButton))
			return;
		state->buttons &= ~(1 << mouseButton);
	}

	[self deliverButton:(MouseButtonID)mouseButton pressed:false];
}

- (void)otherMouseDragged:(NSEvent*)theEvent
{
	[self updateMotion:theEvent];
}

- (void)scrollWheel:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	[self refreshCoordinates];
	if([theEvent deltaY] != 0.0)
		mTempState.Z.rel += ([theEvent deltaY] * 60);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
	[self updateMotion:theEvent];
}

- (void)mouseEntered:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	CGDisplayHideCursor(kCGDirectMainDisplay);
	[self beginCapture];
}

- (void)mouseExited:(NSEvent*)theEvent
{
	if(!oisMouseObj)
		return;
	CGDisplayShowCursor(kCGDirectMainDisplay);
	[self restoreAssociation];
}

- (void)refreshCoordinates
{
	if(!oisMouseObj)
		return;
	MouseState* state = oisMouseObj->getMouseStatePtr();
	NSRect bounds = [self bounds];
	coordinates.geometry(bounds.size.width, bounds.size.height, state->width, state->height);
	positionChanged |= coordinates.publish(state->X.abs, state->Y.abs);
}

- (void)seedCoordinates:(NSPoint)point
{
	[self refreshCoordinates];
	NSRect bounds = [self bounds];
	coordinates.seed(point.x, point.y, bounds.origin.x, bounds.origin.y, [self isFlipped]);
	MouseState* state = oisMouseObj->getMouseStatePtr();
	positionChanged |= coordinates.publish(state->X.abs, state->Y.abs);
}

- (void)beginCapture
{
	if(!oisMouseObj || ownsDisassociation)
		return;
	[self refreshCoordinates];
	// Only associated positions can seed a captured cursor. Convert window space
	// through the actual input view, including its bounds origin and flippedness.
	if([self window])
		[self seedCoordinates:[self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil]];
	// Without a valid associated point the helper retains its defined center.
	mTempState.clear();
	mDeltaX = mDeltaY = 0.0;
	MouseState* state = oisMouseObj->getMouseStatePtr();
	state->X.rel = state->Y.rel = state->Z.rel = 0;
	ownsDisassociation = CGAssociateMouseAndMouseCursorPosition(false) == kCGErrorSuccess;
}

- (void)restoreAssociation
{
	if(ownsDisassociation && CGAssociateMouseAndMouseCursorPosition(true) == kCGErrorSuccess)
		ownsDisassociation = false;
}

- (void)updateMotion:(NSEvent*)theEvent
{
	if(!oisMouseObj || mNeedsToRegainFocus)
		return;
	const double dx = [theEvent deltaX], dy = [theEvent deltaY];
	if(!std::isfinite(dx) || !std::isfinite(dy))
		return;
	[self refreshCoordinates];
	if(ownsDisassociation)
	{
		// Disassociated NSEvent absolute locations stay constant (CGRemoteOperation.h).
		coordinates.move(dx, dy);
		MouseState* state = oisMouseObj->getMouseStatePtr();
		positionChanged |= coordinates.publish(state->X.abs, state->Y.abs);
	}
	else
		[self seedCoordinates:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
	mDeltaX += dx;
	mDeltaY += dy;
}

@end
