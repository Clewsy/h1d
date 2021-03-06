/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Keyboard demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "h1d.h"

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Keyboard,
				.ReportINEndpoint             =
					{
						.Address              = KEYBOARD_EPADDR,
						.Size                 = KEYBOARD_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
	};

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */

//Global variables created for this usage (i.e. not  from LUFA keyboard demo).
bool ready = FALSE;			//Initialise the flag used to determine if ready to begin transmitting keystrokes.
uint8_t type_key = NO_KEY;		//Initialise the keystroke to be sent.
uint8_t type_modifier = NO_MODIFIER;	//Initialise the keyboard modifier to be sent.
static char string[] = THE_STRING;	//Initialise the string to be sent as keystrokes.  Defined in "The_String.h".

int main(void)
{
	SetupHardware();		//Initialise the hardware/peripherals.
	GlobalInterruptEnable();	//Globally enable interrupts.

	while(ready == FALSE)		//Loop continuously until the USB device is ready (triggered by function "EVENT_USB_Device_ConfigurationChanged").
	{
		SendKey(NO_KEY, NO_MODIFIER);		//USB polling with a no-key event.
	}

	//The following block give the host a chance to register the device as a keyboard and begin receiving keystrokes.
	//Effectively pauses, taps the control key then pauses again.  Tested on linux and windows.
	_delay_ms(STARTUP_DELAY_MS);
	SendKey(NO_KEY, HID_KEYBOARD_SC_LEFT_CONTROL);
	SendKey(NO_KEY, NO_MODIFIER);
	_delay_ms(STARTUP_DELAY_MS);

	LEDS_SetAllLEDS(LEDMASK_RUNNING);	//Indicate that the device is ready and running the keystroke entry.

	while(1)	//Infinite loop.
	{
		if(ready)	//This will only happen once on powerup, when the device is successfully enumerated.
		{
			for(uint8_t j = 0; j < REPEAT; j++)					//Loop the string entry a defined number of times.
			{
				for(uint8_t i = 0; i < sizeof(string); i++)			//Loop for every character within the defined string.
				{
					SendKey(CharToKey(string[i]), CheckShift(string[i]));	//For each character, send the keystrokes to recreate it.
					SendKey(NO_KEY, NO_MODIFIER);				//Send a no-key after every key to emulate releasing the key.
												//(This is needed to capture double letters or repeated chars.)
				}
			}
			ready = FALSE;				//Once string typing has looped the defined number of times, flag that it's done.
			LEDS_SetAllLEDS(LEDMASK_DONE);		//Indicate the the device is done.
		}

		SendKey(NO_KEY, NO_MODIFIER);	//Infinite loop will continue to send no-key events so that the host continues to recognise it as a keyboard.
	}
}

//This function effectively sends a keystroke by defining the key and modifier then calling the LUFA routines.
//Note, this must be called (more accurately, the LUFA routines must be called) regularly otherwise the HOST connection will fail.
//In this imp[lementation, only a single standard key and a single modifier can be "pressed" simultaneously.
void SendKey(uint8_t k, uint8_t m)
{
	type_key = k;					//Set the global variable k indicating the key to be "pressed" (refer to LUFA file HIDClassCommon.h).
	type_modifier = m;				//Set the global variable k indicating the modifier to be "pressed".
	HID_Device_USBTask(&Keyboard_HID_Interface);	//LUFA defined function.
	USB_USBTask();					//LUFA defined function.
	_delay_ms(KEY_DELAY_MS);			//Minimum delay implemented to avoid missed "keystrokes".
}

//This function takes a character from the string and converts it to the value corresponding to the appropriate keystroke.
//Refer to LUFA driver file LUFA/USB/CLASS/COMMON/HIDClassCommon.h
uint8_t CharToKey(char c)
{
	switch ((uint8_t)c)	//Typecast the character as an integer.
	{
		case 'A' ... 'Z' :	return ((uint8_t)c - 61);
		case 'a' ... 'z' :	return ((uint8_t)c - 93);
		case '1' ... '9' :	return ((uint8_t)c - 19);
		case '0' :		return (HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS);
		case '!' :		return (HID_KEYBOARD_SC_1_AND_EXCLAMATION);
		case '@' :		return (HID_KEYBOARD_SC_2_AND_AT);
		case '#' :		return (HID_KEYBOARD_SC_3_AND_HASHMARK);
		case '$' :		return (HID_KEYBOARD_SC_4_AND_DOLLAR);
		case '%' :		return (HID_KEYBOARD_SC_5_AND_PERCENTAGE);
		case '^' :		return (HID_KEYBOARD_SC_6_AND_CARET);
		case '&' :		return (HID_KEYBOARD_SC_7_AND_AMPERSAND);
		case '*' :		return (HID_KEYBOARD_SC_8_AND_ASTERISK);
		case '(' :		return (HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS);
		case ')' :		return (HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS);
		case '\n':		return (HID_KEYBOARD_SC_ENTER);
		case '\e':		return (HID_KEYBOARD_SC_ESCAPE);
		case '\b':		return (HID_KEYBOARD_SC_BACKSPACE);
		case '\t':		return (HID_KEYBOARD_SC_TAB);
		case ' ' :		return (HID_KEYBOARD_SC_SPACE);
		case '-' :
		case '_' :		return (HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE);
		case '=' :
		case '+' :		return (HID_KEYBOARD_SC_EQUAL_AND_PLUS);
		case '[' :
		case '{' :		return (HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE);
		case ']' :
		case '}' :		return (HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE);
		case '\\':
		case '|' :		return (HID_KEYBOARD_SC_BACKSLASH_AND_PIPE);
		case ';' :
		case ':' :		return (HID_KEYBOARD_SC_SEMICOLON_AND_COLON);
		case '\'':
		case '"' :		return (HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE);
		case '`' :
		case '~' :		return (HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE);
		case ',' :
		case '<' :		return (HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN);
		case '.' :
		case '>' :		return (HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN);
		case '/' :
		case '?' :		return (HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK);

		default :		return NO_KEY;	//If there is no character match "fail" by sending a NO_KEY code.
	}
}

//This function will identify if reproducing the character in the string by a "keystroke" would require a shift modifier.
//For example the characters 'A' and 'a' are both produced by keystroke HID_KEYBOARD_SC_A, so to print the capital 'A', we also need to "hold shift".
//Refer to LUFA driver file LUFA/USB/CLASS/COMMON/HIDClassCommon.h
uint8_t CheckShift(char c)
{
	switch ((uint8_t)c)	//Typecast the character as an integer.
	{
		case '!' ... '&' :
		case '(' ... '+' :
		case ':' :
		case '<' :
		case '>' ... 'Z' :
		case '^' ... '_' :
		case '{' ... '~' :	return (HID_KEYBOARD_MODIFIER_LEFTSHIFT);
		default :		return NO_MODIFIER;
	}
}


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
//	Joystick_Init();
	LEDS_Init();
//	Buttons_Init();
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDS_SetAllLEDS(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDS_SetAllLEDS(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);

	USB_Device_EnableSOFEvents();

//	LEDS_SetAllLEDS(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
	if(ConfigSuccess)
	{
		LEDS_SetAllLEDS(LEDMASK_USB_READY);
		ready = TRUE;
	}
	else
	{
		LEDS_SetAllLEDS(LEDMASK_USB_ERROR);
	}
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

//	uint8_t JoyStatus_LCL    = Joystick_GetStatus();
//	uint8_t ButtonStatus_LCL = Buttons_GetStatus();

	uint8_t UsedKeyCodes = 0;

	if(type_key != NO_KEY)
	{
		KeyboardReport->KeyCode[UsedKeyCodes] = type_key;
	}
	if(type_modifier != NO_MODIFIER)
	{
		KeyboardReport->Modifier = type_modifier;
	}

	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
/*	uint8_t  LEDMask   = LEDS_NO_LEDS;
	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
	  LEDMask |= LEDS_LED1;

	if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
	  LEDMask |= LEDS_LED3;

	if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
	  LEDMask |= LEDS_LED4;
*/
//	LEDs_SetAllLEDs(LEDMask);
}
