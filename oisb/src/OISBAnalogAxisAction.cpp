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

#include "OISBAnalogAxisAction.h"
#include "OISBBinding.h"
#include "OISBDigitalState.h"
#include "OISBAnalogAxisState.h"

#include "OISException.h"

namespace OISB
{
	AnalogAxisAction::AnalogAxisAction(ActionSchema* parent, const String& name):
		Action(parent, name),

        mAllowEmulation(true),
        mEmulationDecreaseSpeed(1),
        mEmulationIncreaseSpeed(1),

        mUseAbsoluteValues(false),

        mAbsoluteValue(0.0f),
        mDeltaValue(0.0f),

        mMinimumValue(-1.0f),
        mMaximumValue(1.0f),
        mSensitivity(1.0f)
	{}
	
	AnalogAxisAction::~AnalogAxisAction()
	{}
	
	ActionType AnalogAxisAction::getActionType() const
    {
        return AT_ANALOG_AXIS;
    }

    void AnalogAxisAction::setEmulationDecreaseSpeed(Real speed)
    {
        mEmulationDecreaseSpeed = speed;
    }

    void AnalogAxisAction::setEmulationIncreaseSpeed(Real speed)
    {
        mEmulationIncreaseSpeed = speed;
    }

    void AnalogAxisAction::setEmulationSpeed(Real speed)
    {
        setEmulationDecreaseSpeed(speed);
        setEmulationIncreaseSpeed(speed);
    }

    void AnalogAxisAction::setMinimumValue(Real min)
    {
        mMinimumValue = min;
    }

    void AnalogAxisAction::setMaximumValue(Real max)
    {
        mMaximumValue = max;
    }
	
    bool AnalogAxisAction::impl_process(Real delta)
    {
        mDeltaValue = 0.0f;

        if (mUseAbsoluteValues && mBindings.size() > 1)
        {
            OIS_EXCEPT(OIS::E_General, String("Using multiple alternative bindings when mUseAbsoluteValues is "
                "true is not what you wanted to do probably").c_str());
        }

        for (BindingList::const_iterator it = mBindings.begin(); it != mBindings.end(); ++it)
        {
            Binding* binding = *it;

            const Real mOldDeltaValue = mDeltaValue;

            if (mAllowEmulation && binding->getNumBindables() == 2)
            {
                // emulation is enabled and there are 2 states in the binding
                State* decrease = 0;
                State* increase = 0;

                // first check for roles
                if (binding->isBound("decrease") &&
                    binding->isBound("increase"))
                {
                    decrease = binding->getState("decrease");
                    increase = binding->getState("increase");
                }
                // otherwise the first will decrease the value
                // and the second (and last) will increase the value
                else
                {
                    decrease = binding->getState(0);
                    increase = binding->getState(1);
                }

                // now that we have found the states, let's check that they are digital
                if (decrease->getStateType() != ST_DIGITAL ||
                    increase->getStateType() != ST_DIGITAL)
                {
                    OIS_EXCEPT(OIS::E_General, String("Assuming digital analog emulation mode but one of the states "
                        "isn't digital!").c_str());
                }

                impl_processEmulation(delta,
                    static_cast<DigitalState*>(decrease),
                    static_cast<DigitalState*>(increase));
            }
            else
            {
                if (binding->getNumBindables() > 1)
                {
                    OIS_EXCEPT(OIS::E_General, String("Having multiple states in a binding "
                        "is only allowed for emulation (which is disabled!)").c_str());
                }

                State* state = binding->getState(0);
                if (state->getStateType() != ST_ANALOG_AXIS)
                {
                    OIS_EXCEPT(OIS::E_General, String("There is only one state bound and "
                        "it is not analog axis state!").c_str());
                }

                impl_processNative(delta, static_cast<AnalogAxisState*>(state));
            }

            binding->_setActive(mOldDeltaValue != mDeltaValue);
        }

        // clamp the value to limits
        mAbsoluteValue = std::min(mMaximumValue, mAbsoluteValue);
        mAbsoluteValue = std::max(mMinimumValue, mAbsoluteValue);

        return fabs(mDeltaValue) < std::numeric_limits<Real>::epsilon();
    }

    void AnalogAxisAction::impl_processEmulation(Real delta, DigitalState* dec, DigitalState* inc)
    {
        if (mUseAbsoluteValues)
        {
            // both are pressed
            if (dec->isActive() && inc->isActive())
            {
                // bigger speed wins
                if (mEmulationDecreaseSpeed > mEmulationIncreaseSpeed)
                {
                    mDeltaValue = mMinimumValue - mAbsoluteValue;
                    mAbsoluteValue = mMinimumValue;
                }
                else if (mEmulationDecreaseSpeed < mEmulationIncreaseSpeed)
                {
                    mDeltaValue = mMaximumValue - mAbsoluteValue;
                    mAbsoluteValue = mMaximumValue;
                }
                else
                {
                    const Real avg = (mMinimumValue + mMaximumValue) * 0.5f;

                    mDeltaValue = avg - mAbsoluteValue;
                    mAbsoluteValue = avg;
                }
            }
            else if (dec->isActive())
            {
                mDeltaValue = mMinimumValue - mAbsoluteValue;
                mAbsoluteValue = mMinimumValue;
            }
            else if (inc->isActive())
            {
                mDeltaValue = mMaximumValue - mAbsoluteValue;
                mAbsoluteValue = mMaximumValue;
            }
            // the only remaining case is that nothing is pressed
            // in that case, there is nothing to do
        }
        else
        {
            if (dec->isActive())
            {
                // decrease is pressed
                        
                mDeltaValue = ((-1.0f) * mEmulationDecreaseSpeed * delta) * mSensitivity;
                mAbsoluteValue += mDeltaValue;
            }
            if (inc->isActive())
            {
                // increase is pressed
                        
                mDeltaValue = ((+1.0f) * mEmulationIncreaseSpeed * delta) * mSensitivity;
                mAbsoluteValue += mDeltaValue;
            }
        }
    }

    void AnalogAxisAction::impl_processNative(Real delta, AnalogAxisState* state)
    {
        if (mUseAbsoluteValues)
        {
            mDeltaValue = state->getAbsoluteValue() - mAbsoluteValue;
            mAbsoluteValue = state->getAbsoluteValue();
        }
        else
        {
            mDeltaValue = state->getDeltaValue() * mSensitivity;
            mAbsoluteValue += mDeltaValue;
        }
    }
}
