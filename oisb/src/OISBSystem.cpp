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

#include "OISBSystem.h"
#include "OISBMouse.h"
#include "OISBKeyboard.h"
#include "OISBActionSchema.h"
#include "OISBAction.h"
#include "OISBState.h"

#include "OISException.h"
#include "OISInputManager.h"
#include "OISKeyboard.h"
#include "OISMouse.h"

namespace OISB
{
    // static member singleton pointer
    System* System::msSingleton = 0;

	System::System():
		mInitialized(false),

		mOIS(0),
		mOISMouse(0),
		mOISKeyboard(0),
		mMouse(0),
		mKeyboard(0)
	{
		 msSingleton = this;
	}
 
	System::~System()
	{
		if (mActionSchemas.size() > 0)
		{
			while (mActionSchemas.size() > 0)
			{
				destroyActionSchema(mActionSchemas.begin()->first);
			}
		}

		msSingleton = 0;
	}
 
	void System::initialize(OIS::InputManager* ois)
	{
		if (mInitialized)
		{
			return;
		}

		mOIS = ois;
 
		// If possible create a buffered keyboard
		if (mOIS->getNumberOfDevices(OIS::OISKeyboard) > 0)
		{
			mOISKeyboard = static_cast<OIS::Keyboard*>(mOIS->createInputObject(OIS::OISKeyboard, true));
		}
 
		// If possible create a buffered mouse
		if (mOIS->getNumberOfDevices(OIS::OISMouse) > 0)
		{
			mOISMouse = static_cast<OIS::Mouse*>(mOIS->createInputObject(OIS::OISMouse, true));
		}

		mMouse = new Mouse(mOISMouse);
		addDevice(mMouse);

		mKeyboard = new Keyboard(mOISKeyboard);
		addDevice(mKeyboard);

		mInitialized = true;
	}

	void System::finalize()
	{
		if (!mInitialized)
		{
			return;
		}

		removeDevice(mMouse);
		delete mMouse;
		removeDevice(mKeyboard);
		delete mKeyboard;

		if (mOISMouse)
		{
			mOIS->destroyInputObject(mOISMouse);
			mOISMouse = 0;
		}
 
		if (mOISKeyboard)
		{
			mOIS->destroyInputObject(mOISKeyboard);
			mOISKeyboard = 0;
		}
 
		mOIS->destroyInputSystem(mOIS);
		mOIS = 0;

		mInitialized = false;
	}
 
	void System::process(Real delta)
	{
		mOISMouse->capture();
		mOISKeyboard->capture();

		for (DeviceMap::const_iterator it = mDevices.begin(); it != mDevices.end(); ++it)
		{
			it->second->process(delta);
		}

		for (ActionSchemaMap::const_iterator it = mActionSchemas.begin(); it != mActionSchemas.end(); ++it)
        {
            it->second->process(delta);
        }
	}
	
	Device* System::getDevice(const String& name) const
	{
		DeviceMap::const_iterator it = mDevices.find(name);
		
		if (it == mDevices.end())
		{
			OIS_EXCEPT(OIS::E_General, String("Device '" + name + "' not found!").c_str());
		}
		
		return it->second;
	}

    bool System::hasDevice(const String& name) const
    {
        DeviceMap::const_iterator it = mDevices.find(name);
		
		return it != mDevices.end();
    }

    State* System::lookupState(const String& name) const
    {
        String::size_type i = name.find("/");
        const String deviceName = name.substr(0, i); // -1 because we want to copy just before the "/"

        if (hasDevice(deviceName))
        {
            Device* dev = getDevice(deviceName);
            const String stateName = name.substr(i + 1);

            if (dev->hasState(stateName))
            {
                return dev->getState(stateName);
            }
        }

        // nothing was found
        return 0;
    }

    ActionSchema* System::createActionSchema(const String& name, bool setAsDefault)
	{
		ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		if (it != mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_Duplicate, String("Action schema '" + name + "' already exists").c_str());
		}
		
		ActionSchema* ret = new ActionSchema(name);
		mActionSchemas.insert(std::make_pair(name, ret));

		if (setAsDefault)
		{
			setDefaultActionSchema(ret);
		}

		return ret;
	}

	void System::destroyActionSchema(const String& name)
	{
		ActionSchemaMap::iterator it = mActionSchemas.find(name);
		
		if (it == mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action schema '" + name + "' not found").c_str());
		}

		ActionSchema* schema = it->second;
		mActionSchemas.erase(it);

        if (schema == mDefaultActionSchema)
        {
            setDefaultActionSchema(0);
        }

		delete schema;
	}
	
	void System::destroyActionSchema(ActionSchema* schema)
	{
		destroyActionSchema(schema->getName());
	}

	ActionSchema* System::getActionSchema(const String& name) const
	{
		ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		if (it == mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action schema '" + name + "' not found").c_str());
		}

		return it->second;
	}

    bool System::hasActionSchema(const String& name) const
    {
        ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		return it != mActionSchemas.end();
    }

	void System::setDefaultActionSchema(ActionSchema* schema)
	{
		mDefaultActionSchema = schema;
	}

	void System::setDefaultActionSchema(const String& name)
	{
		setDefaultActionSchema(getActionSchema(name));
	}

	ActionSchema* System::getDefaultActionSchemaAutoCreate()
	{
		if (!mDefaultActionSchema)
		{
			createActionSchema("Default", true);
		}

		return getDefaultActionSchema();
	}

    Action* System::lookupAction(const String& name) const
    {
        String::size_type i = name.find("/");
        const String schemaName = name.substr(0, i); // -1 because we want to copy just before the "/"

        if (hasActionSchema(schemaName))
        {
            ActionSchema* schema = getActionSchema(schemaName);
            const String actionName = name.substr(i + 1);

            if (schema->hasAction(actionName))
            {
                return schema->getAction(actionName);
            }
        }

        // nothing was found
        return 0;
    }

    Bindable* System::lookupBindable(const String& name) const
    {
        State* state = lookupState(name);
        if (state)
        {
            return state;
        }

        Action* action = lookupAction(name);
        if (action)
        {
            return action;
        }

        // nothing was found, return 0
        return 0;
    }
	
	void System::addDevice(Device* device)
	{
		DeviceMap::const_iterator it = mDevices.find(device->getName());
		
		if (it != mDevices.end())
		{
			OIS_EXCEPT(OIS::E_Duplicate, String("Device with name '" + device->getName() + "' already exists!").c_str());
		}
		
		mDevices.insert(std::make_pair(device->getName(), device));
	}
	
	void System::removeDevice(Device* device)
	{
		DeviceMap::iterator it = mDevices.find(device->getName());
		
		if (it == mDevices.end())
		{
			OIS_EXCEPT(OIS::E_General, String("Device '" + device->getName() + "' not found!").c_str());
		}
		
		mDevices.erase(it);
	}
}
