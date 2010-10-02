/*
The zlib/libpng License

Copyright (c) 2009-2010 Martin Preisler

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

#include "OISBBindable.h"
#include "OISException.h"

#include <algorithm>
#include <iostream>

namespace OISB
{
    BindableListener::~BindableListener()
    {}

    void BindableListener::bindableActivated(Bindable* /*bindable*/)
    {
        // default behaviour is to do nothing
    }

    void BindableListener::bindableDeactivated(Bindable* /*bindable*/)
    {
        // default behaviour is to do nothing
    }

    void BindableListener::bindableProcessed(Bindable* /*bindable*/)
    {
        // default behaviour is to do nothing
    }

    DebugBindableListener::~DebugBindableListener()
    {}

    void DebugBindableListener::bindableActivated(Bindable* bindable)
    {
        std::cout << "DEBUG: activated '" + bindable->getBindableName() + "'" << std::endl;
    }

    void DebugBindableListener::bindableDeactivated(Bindable* bindable)
    {
        std::cout << "DEBUG: deactivated '" + bindable->getBindableName() + "'" << std::endl;
    }

    Bindable::~Bindable()
    {}

    void Bindable::addListener(BindableListener* listener)
    {
        ListenerList::const_iterator it = std::find(mListeners.begin(), mListeners.end(), listener);

        if (it != mListeners.end())
        {
            OIS_EXCEPT(OIS::E_Duplicate, "Such listener has already been added to this bindable!");
        }

        mListeners.push_back(listener);
    }


    void Bindable::removeListener(BindableListener* listener)
    {
        ListenerList::iterator it = std::find(mListeners.begin(), mListeners.end(), listener);

        if (it == mListeners.end())
        {
            OIS_EXCEPT(OIS::E_General, "No such listener found in this bindable!");
        }

        mListeners.erase(it);
    }

	void Bindable::notifyActivated()
    {
        for (ListenerList::const_iterator it = mListeners.begin(); it != mListeners.end(); ++it)
        {
            BindableListener* listener = *it;
            listener->bindableActivated(this);
        }
    }
            
    void Bindable::notifyDeactivated()
    {
        for (ListenerList::const_iterator it = mListeners.begin(); it != mListeners.end(); ++it)
        {
            BindableListener* listener = *it;
            listener->bindableDeactivated(this);
        }
    }

    void Bindable::notifyProcessed()
    {
        for (ListenerList::const_iterator it = mListeners.begin(); it != mListeners.end(); ++it)
        {
            BindableListener* listener = *it;
            listener->bindableProcessed(this);
        }
    }
}
