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

#ifndef __OISB_ANALOG_AXIS_ACTION_H__
#define __OISB_ANALOG_AXIS_ACTION_H__

#include "OISBGlobal.h"
#include "OISBAction.h"

namespace OISB
{
	/**
	 * @brief all of the states in at least one binding trigger this action
	 */
	class _OISBExport AnalogAxisAction : public Action
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent action schema
			 * @param name name / identifier
			 */
			AnalogAxisAction(ActionSchema* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~AnalogAxisAction();

            /// @copydoc Action::getActionType
            virtual ActionType getActionType() const;

            void setEmulationDecreaseSpeed(Real speed);
            void setEmulationIncreaseSpeed(Real speed);

            void setEmulationSpeed(Real speed);

            void setEmulationDecreaseReturnSpeed(Real speed);
            void setEmulationIncreaseReturnSpeed(Real speed);

            void setEmulationReturnSpeed(Real speed);

            void setMinimumValue(Real min);

            inline Real getMinimumValue() const
            {
                return mMinimumValue;
            }

            void setMaximumValue(Real max);

            inline Real getMaximumValue() const
            {
                return mMaximumValue;
            }

            inline Real getAbsoluteValue() const
            {
                return mAbsoluteValue;
            }

            inline Real getDeltaValue() const
            {
                return mDeltaValue;
            }

            void setPivotValue(Real pivot);

            inline Real getPivotValue()
            {
                return mPivotValue;
            }

        protected:
            /// @copydoc Action::impl_process
            virtual bool impl_process(Real delta);

        private:
            /// internal function, takes over impl_process in case we do digital analog emulation
            void impl_processEmulation(Real delta, DigitalState* dec, DigitalState* inc);
            /// internal function, takes over impl_process in case we do native analog analog
            void impl_processNative(Real delta, AnalogAxisState* state);

            /// allows digital states to be bound to this action, analog is emulated then
            bool mAllowEmulation;
            /// decreasing speed
            Real mEmulationDecreaseSpeed;
            /// increasing speed
            Real mEmulationIncreaseSpeed;
            /// return from decrease speed
            Real mEmulationDecreaseReturnSpeed;
            /// return from decrease speed
            Real mEmulationIncreaseReturnSpeed;

            /// if true, this action takes the absolute values of the analog states (if any)
            bool mUseAbsoluteValues;

            /// stores current value of this action
			Real mAbsoluteValue;
			/// delta since last processing
			Real mDeltaValue;
			
			/// calibration clamping minimum value
			Real mMinimumValue;
			/// calibration clamping maximum value
			Real mMaximumValue;
            /// pivot value, only used for emulation return code
            Real mPivotValue;
			/// calibration sensitivity
			Real mSensitivity;
	};
}

#endif
