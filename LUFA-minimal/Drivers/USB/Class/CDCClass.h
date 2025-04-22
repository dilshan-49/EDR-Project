/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  \brief Master include file for the library USB CDC-ACM Class driver.
 *
 *  Master include file for the library USB CDC Class driver, for both host and device modes, where available.
 *
 *  This file should be included in all user projects making use of this optional class driver, instead of
 *  including any headers in the USB/ClassDriver/Device, USB/ClassDriver/Host or USB/ClassDriver/Common subdirectories.
 */

/** \ingroup Group_USBClassDrivers
 *  \defgroup Group_USBClassCDC CDC-ACM (Virtual Serial) Class Driver
 *  \brief USB class driver for the USB-IF CDC-ACM (Virtual Serial) class standard.
 *
 *  \section Sec_USBClassCDC_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - LUFA/Drivers/USB/Class/Device/CDCClassDevice.c <i>(Makefile source module name: LUFA_SRC_USBCLASS)</i>
 *    - LUFA/Drivers/USB/Class/Host/CDCClassHost.c <i>(Makefile source module name: LUFA_SRC_USBCLASS)</i>
 *
 *  \section Sec_USBClassCDC_ModDescription Module Description
 *  CDC Class Driver module. This module contains an internal implementation of the USB CDC-ACM class Virtual Serial
 *  Ports, for both Device and Host USB modes. User applications can use this class driver instead of implementing the
 *  CDC class manually via the low-level LUFA APIs.
 *
 *  This module is designed to simplify the user code by exposing only the required interface needed to interface with
 *  Hosts or Devices using the USB CDC Class.
 *
 *  @{
 */

#ifndef _CDC_CLASS_H_
#define _CDC_CLASS_H_

	/* Macros: */
	#define __INCLUDE_FROM_USB_DRIVER
	#define __INCLUDE_FROM_CDC_DRIVER

	/* Includes: */
	#include "../Core/USBMode.h"

	#include "../USB.h"
	#include "../Core/StdDescriptors.h"

	#include <stdio.h>
	
/******************* Common **************************/

	/* Macros: */
		/** \name Virtual Control Line Masks */
		//@{
		/** Mask for the DTR handshake line for use with the \ref CDC_REQ_SetControlLineState class-specific request
		 *  from the host, to indicate that the DTR line state should be high.
		 */
		#define CDC_CONTROL_LINE_OUT_DTR         (1 << 0)

		/** Mask for the RTS handshake line for use with the \ref CDC_REQ_SetControlLineState class-specific request
		 *  from the host, to indicate that the RTS line state should be high.
		 */
		#define CDC_CONTROL_LINE_OUT_RTS         (1 << 1)

		/** Mask for the DCD handshake line for use with the \ref CDC_NOTIF_SerialState class-specific notification
		 *  from the device to the host, to indicate that the DCD line state is currently high.
		 */
		#define CDC_CONTROL_LINE_IN_DCD          (1 << 0)

		/** Mask for the DSR handshake line for use with the \ref CDC_NOTIF_SerialState class-specific notification
		 *  from the device to the host, to indicate that the DSR line state is currently high.
		 */
		#define CDC_CONTROL_LINE_IN_DSR          (1 << 1)

		/** Mask for the BREAK handshake line for use with the \ref CDC_NOTIF_SerialState class-specific notification
		 *  from the device to the host, to indicate that the BREAK line state is currently high.
		 */
		#define CDC_CONTROL_LINE_IN_BREAK        (1 << 2)

		/** Mask for the RING handshake line for use with the \ref CDC_NOTIF_SerialState class-specific notification
		 *  from the device to the host, to indicate that the RING line state is currently high.
		 */
		#define CDC_CONTROL_LINE_IN_RING         (1 << 3)

		/** Mask for use with the \ref CDC_NOTIF_SerialState class-specific notification from the device to the host,
		 *  to indicate that a framing error has occurred on the virtual serial port.
		 */
		#define CDC_CONTROL_LINE_IN_FRAMEERROR   (1 << 4)

		/** Mask for use with the \ref CDC_NOTIF_SerialState class-specific notification from the device to the host,
		 *  to indicate that a parity error has occurred on the virtual serial port.
		 */
		#define CDC_CONTROL_LINE_IN_PARITYERROR  (1 << 5)

		/** Mask for use with the \ref CDC_NOTIF_SerialState class-specific notification from the device to the host,
		 *  to indicate that a data overrun error has occurred on the virtual serial port.
		 */
		#define CDC_CONTROL_LINE_IN_OVERRUNERROR (1 << 6)
		//@}

		/** Macro to define a CDC class-specific functional descriptor. CDC functional descriptors have a
		 *  uniform structure but variable sized data payloads, thus cannot be represented accurately by
		 *  a single \c typedef \c struct. A macro is used instead so that functional descriptors can be created
		 *  easily by specifying the size of the payload. This allows \c sizeof() to work correctly.
		 *
		 *  \param[in] DataSize  Size in bytes of the CDC functional descriptor's data payload.
		 */
		#define CDC_FUNCTIONAL_DESCRIPTOR(DataSize)        \
		     struct                                        \
		     {                                             \
		          USB_Descriptor_Header_t Header;          \
			      uint8_t                 SubType;         \
		          uint8_t                 Data[DataSize];  \
		     }

	/* Enums: */
		/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the CDC
		 *  device class.
		 */
		enum CDC_Descriptor_ClassSubclassProtocol_t
		{
			CDC_CSCP_CDCClass               = 0x02, /**< Descriptor Class value indicating that the device or interface
			                                         *   belongs to the CDC class.
			                                         */
			CDC_CSCP_NoSpecificSubclass     = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to no specific subclass of the CDC class.
			                                         */
			CDC_CSCP_ACMSubclass            = 0x02, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to the Abstract Control Model CDC subclass.
			                                         */
			CDC_CSCP_ATCommandProtocol      = 0x01, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to the AT Command protocol of the CDC class.
			                                         */
			CDC_CSCP_NoSpecificProtocol     = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to no specific protocol of the CDC class.
			                                         */
			CDC_CSCP_VendorSpecificProtocol = 0xFF, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to a vendor-specific protocol of the CDC class.
			                                         */
			CDC_CSCP_CDCDataClass           = 0x0A, /**< Descriptor Class value indicating that the device or interface
			                                         *   belongs to the CDC Data class.
			                                         */
			CDC_CSCP_NoDataSubclass         = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                         *   belongs to no specific subclass of the CDC data class.
			                                         */
			CDC_CSCP_NoDataProtocol         = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                         *   belongs to no specific protocol of the CDC data class.
			                                         */
		};

		/** Enum for the CDC class specific control requests that can be issued by the USB bus host. */
		enum CDC_ClassRequests_t
		{
			CDC_REQ_SendEncapsulatedCommand = 0x00, /**< CDC class-specific request to send an encapsulated command to the device. */
			CDC_REQ_GetEncapsulatedResponse = 0x01, /**< CDC class-specific request to retrieve an encapsulated command response from the device. */
			CDC_REQ_SetLineEncoding         = 0x20, /**< CDC class-specific request to set the current virtual serial port configuration settings. */
			CDC_REQ_GetLineEncoding         = 0x21, /**< CDC class-specific request to get the current virtual serial port configuration settings. */
			CDC_REQ_SetControlLineState     = 0x22, /**< CDC class-specific request to set the current virtual serial port handshake line states. */
			CDC_REQ_SendBreak               = 0x23, /**< CDC class-specific request to send a break to the receiver via the carrier channel. */
		};

		/** Enum for the CDC class specific notification requests that can be issued by a CDC device to a host. */
		enum CDC_ClassNotifications_t
		{
			CDC_NOTIF_SerialState = 0x20, /**< Notification type constant for a change in the virtual serial port
			                               *   handshake line states, for use with a \ref USB_Request_Header_t
			                               *   notification structure when sent to the host via the CDC notification
			                               *   endpoint.
			                               */
		};

		/** Enum for the CDC class specific interface descriptor subtypes. */
		enum CDC_DescriptorSubtypes_t
		{
			CDC_DSUBTYPE_CSInterface_Header           = 0x00, /**< CDC class-specific Header functional descriptor. */
			CDC_DSUBTYPE_CSInterface_CallManagement   = 0x01, /**< CDC class-specific Call Management functional descriptor. */
			CDC_DSUBTYPE_CSInterface_ACM              = 0x02, /**< CDC class-specific Abstract Control Model functional descriptor. */
			CDC_DSUBTYPE_CSInterface_DirectLine       = 0x03, /**< CDC class-specific Direct Line functional descriptor. */
			CDC_DSUBTYPE_CSInterface_TelephoneRinger  = 0x04, /**< CDC class-specific Telephone Ringer functional descriptor. */
			CDC_DSUBTYPE_CSInterface_TelephoneCall    = 0x05, /**< CDC class-specific Telephone Call functional descriptor. */
			CDC_DSUBTYPE_CSInterface_Union            = 0x06, /**< CDC class-specific Union functional descriptor. */
			CDC_DSUBTYPE_CSInterface_CountrySelection = 0x07, /**< CDC class-specific Country Selection functional descriptor. */
			CDC_DSUBTYPE_CSInterface_TelephoneOpModes = 0x08, /**< CDC class-specific Telephone Operation Modes functional descriptor. */
			CDC_DSUBTYPE_CSInterface_USBTerminal      = 0x09, /**< CDC class-specific USB Terminal functional descriptor. */
			CDC_DSUBTYPE_CSInterface_NetworkChannel   = 0x0A, /**< CDC class-specific Network Channel functional descriptor. */
			CDC_DSUBTYPE_CSInterface_ProtocolUnit     = 0x0B, /**< CDC class-specific Protocol Unit functional descriptor. */
			CDC_DSUBTYPE_CSInterface_ExtensionUnit    = 0x0C, /**< CDC class-specific Extension Unit functional descriptor. */
			CDC_DSUBTYPE_CSInterface_MultiChannel     = 0x0D, /**< CDC class-specific Multi-Channel Management functional descriptor. */
			CDC_DSUBTYPE_CSInterface_CAPI             = 0x0E, /**< CDC class-specific Common ISDN API functional descriptor. */
			CDC_DSUBTYPE_CSInterface_Ethernet         = 0x0F, /**< CDC class-specific Ethernet functional descriptor. */
			CDC_DSUBTYPE_CSInterface_ATM              = 0x10, /**< CDC class-specific Asynchronous Transfer Mode functional descriptor. */
		};

		/** Enum for the possible line encoding formats of a virtual serial port. */
		enum CDC_LineEncodingFormats_t
		{
			CDC_LINEENCODING_OneStopBit          = 0, /**< Each frame contains one stop bit. */
			CDC_LINEENCODING_OneAndAHalfStopBits = 1, /**< Each frame contains one and a half stop bits. */
			CDC_LINEENCODING_TwoStopBits         = 2, /**< Each frame contains two stop bits. */
		};

		/** Enum for the possible line encoding parity settings of a virtual serial port. */
		enum CDC_LineEncodingParity_t
		{
			CDC_PARITY_None  = 0, /**< No parity bit mode on each frame. */
			CDC_PARITY_Odd   = 1, /**< Odd parity bit mode on each frame. */
			CDC_PARITY_Even  = 2, /**< Even parity bit mode on each frame. */
			CDC_PARITY_Mark  = 3, /**< Mark parity bit mode on each frame. */
			CDC_PARITY_Space = 4, /**< Space parity bit mode on each frame. */
		};

	/* Type Defines: */
		/** \brief CDC class-specific Functional Header Descriptor (LUFA naming conventions).
		 *
		 *  Type define for a CDC class-specific functional header descriptor. This indicates to the host that the device
		 *  contains one or more CDC functional data descriptors, which give the CDC interface's capabilities and configuration.
		 *  See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_StdDescriptor_FunctionalHeader_t for the version of this type with standard element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                 Subtype; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                                  *   must be \ref CDC_DSUBTYPE_CSInterface_Header.
			                                  */
			uint16_t                CDCSpecification; /**< Version number of the CDC specification implemented by the device,
			                                           *   encoded in BCD format.
			                                           *
			                                           *   \see \ref VERSION_BCD() utility macro.
			                                           */
		} ATTR_PACKED USB_CDC_Descriptor_FunctionalHeader_t;

		/** \brief CDC class-specific Functional Header Descriptor (USB-IF naming conventions).
		 *
		 *  Type define for a CDC class-specific functional header descriptor. This indicates to the host that the device
		 *  contains one or more CDC functional data descriptors, which give the CDC interface's capabilities and configuration.
		 *  See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_Descriptor_FunctionalHeader_t for the version of this type with non-standard LUFA specific
		 *       element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			uint8_t  bFunctionLength; /**< Size of the descriptor, in bytes. */
			uint8_t  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
			                           *   given by the specific class.
			                           */
			uint8_t  bDescriptorSubType; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                              *   must be \ref CDC_DSUBTYPE_CSInterface_Header.
			                              */
			uint16_t bcdCDC; /**< Version number of the CDC specification implemented by the device, encoded in BCD format.
			                  *
			                  *   \see \ref VERSION_BCD() utility macro.
			                  */
		} ATTR_PACKED USB_CDC_StdDescriptor_FunctionalHeader_t;

		/** \brief CDC class-specific Functional ACM Descriptor (LUFA naming conventions).
		 *
		 *  Type define for a CDC class-specific functional ACM descriptor. This indicates to the host that the CDC interface
		 *  supports the CDC ACM subclass of the CDC specification. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_StdDescriptor_FunctionalACM_t for the version of this type with standard element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                 Subtype; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                                  *   must be \ref CDC_DSUBTYPE_CSInterface_ACM.
			                                  */
			uint8_t                 Capabilities; /**< Capabilities of the ACM interface, given as a bit mask. For most devices,
			                                       *   this should be set to a fixed value of \c 0x06 - for other capabilities, refer
			                                       *   to the CDC ACM specification.
			                                       */
		} ATTR_PACKED USB_CDC_Descriptor_FunctionalACM_t;

		/** \brief CDC class-specific Functional ACM Descriptor (USB-IF naming conventions).
		 *
		 *  Type define for a CDC class-specific functional ACM descriptor. This indicates to the host that the CDC interface
		 *  supports the CDC ACM subclass of the CDC specification. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_Descriptor_FunctionalACM_t for the version of this type with non-standard LUFA specific
		 *       element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			uint8_t bFunctionLength; /**< Size of the descriptor, in bytes. */
			uint8_t bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
			                          *   given by the specific class.
			                          */
			uint8_t bDescriptorSubType; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                             *   must be \ref CDC_DSUBTYPE_CSInterface_ACM.
			                             */
			uint8_t bmCapabilities; /**< Capabilities of the ACM interface, given as a bit mask. For most devices,
			                         *   this should be set to a fixed value of 0x06 - for other capabilities, refer
			                         *   to the CDC ACM specification.
			                         */
		} ATTR_PACKED USB_CDC_StdDescriptor_FunctionalACM_t;

		/** \brief CDC class-specific Functional Union Descriptor (LUFA naming conventions).
		 *
		 *  Type define for a CDC class-specific functional Union descriptor. This indicates to the host that specific
		 *  CDC control and data interfaces are related. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_StdDescriptor_FunctionalUnion_t for the version of this type with standard element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                 Subtype; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                                  *   must be \ref CDC_DSUBTYPE_CSInterface_Union.
			                                  */
			uint8_t                 MasterInterfaceNumber; /**< Interface number of the CDC Control interface. */
			uint8_t                 SlaveInterfaceNumber; /**< Interface number of the CDC Data interface. */
		} ATTR_PACKED USB_CDC_Descriptor_FunctionalUnion_t;

		/** \brief CDC class-specific Functional Union Descriptor (USB-IF naming conventions).
		 *
		 *  Type define for a CDC class-specific functional Union descriptor. This indicates to the host that specific
		 *  CDC control and data interfaces are related. See the CDC class specification for more details.
		 *
		 *  \see \ref USB_CDC_Descriptor_FunctionalUnion_t for the version of this type with non-standard LUFA specific
		 *       element names.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			uint8_t bFunctionLength; /**< Size of the descriptor, in bytes. */
			uint8_t bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
			                          *   given by the specific class.
			                          */
			uint8_t bDescriptorSubType; /**< Sub type value used to distinguish between CDC class-specific descriptors,
			                             *   must be \ref CDC_DSUBTYPE_CSInterface_Union.
			                             */
			uint8_t bMasterInterface; /**< Interface number of the CDC Control interface. */
			uint8_t bSlaveInterface0; /**< Interface number of the CDC Data interface. */
		} ATTR_PACKED USB_CDC_StdDescriptor_FunctionalUnion_t;

		/** \brief CDC Virtual Serial Port Line Encoding Settings Structure.
		 *
		 *  Type define for a CDC Line Encoding structure, used to hold the various encoding parameters for a virtual
		 *  serial port.
		 *
		 *  \note Regardless of CPU architecture, these values should be stored as little endian.
		 */
		typedef struct
		{
			uint32_t BaudRateBPS; /**< Baud rate of the virtual serial port, in bits per second. */
			uint8_t  CharFormat; /**< Character format of the virtual serial port, a value from the
								  *   \ref CDC_LineEncodingFormats_t enum.
								  */
			uint8_t  ParityType; /**< Parity setting of the virtual serial port, a value from the
								  *   \ref CDC_LineEncodingParity_t enum.
								  */
			uint8_t  DataBits; /**< Bits of data per character of the virtual serial port. */
		} ATTR_PACKED CDC_LineEncoding_t;

/******************* End of Common ***************************/



	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_CDC_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Type Defines: */
			/** \brief CDC Class Device Mode Configuration and State Structure.
			 *
			 *  Class state structure. An instance of this structure should be made for each CDC interface
			 *  within the user application, and passed to each of the CDC class driver functions as the
			 *  CDCInterfaceInfo parameter. This stores each CDC interface's configuration and state information.
			 */
			typedef struct
			{
				struct
				{
					uint8_t ControlInterfaceNumber; /**< Interface number of the CDC control interface within the device. */

					USB_Endpoint_Table_t DataINEndpoint; /**< Data IN endpoint configuration table. */
					USB_Endpoint_Table_t DataOUTEndpoint; /**< Data OUT endpoint configuration table. */
					USB_Endpoint_Table_t NotificationEndpoint; /**< Notification IN Endpoint configuration table. */
				} Config; /**< Config data for the USB class interface within the device. All elements in this section
				           *   <b>must</b> be set or the interface will fail to enumerate and operate correctly.
				           */
				struct
				{
					struct
					{
						uint16_t HostToDevice; /**< Control line states from the host to device, as a set of \c CDC_CONTROL_LINE_OUT_*
											    *   masks. This value is updated each time \ref CDC_Device_USBTask() is called.
											    */
						uint16_t DeviceToHost; /**< Control line states from the device to host, as a set of \c CDC_CONTROL_LINE_IN_*
											    *   masks - to notify the host of changes to these values, call the
											    *   \ref CDC_Device_SendControlLineStateChange() function.
											    */
					} ControlLineStates; /**< Current states of the virtual serial port's control lines between the device and host. */

					CDC_LineEncoding_t LineEncoding; /**< Line encoding used in the virtual serial port, for the device's information.
					                                  *   This is generally only used if the virtual serial port data is to be
					                                  *   reconstructed on a physical UART.
					                                  */
				} State; /**< State data for the USB class interface within the device. All elements in this section
				          *   are reset to their defaults when the interface is enumerated.
				          */
			} USB_ClassInfo_CDC_Device_t;

		/* Function Prototypes: */
			/** Configures the endpoints of a given CDC interface, ready for use. This should be linked to the library
			 *  \ref EVENT_USB_Device_ConfigurationChanged() event so that the endpoints are configured when the configuration containing
			 *  the given CDC interface is selected.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *
			 *  \return Boolean \c true if the endpoints were successfully configured, \c false otherwise.
			 */
			bool CDC_Device_ConfigureEndpoints(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** Processes incoming control requests from the host, that are directed to the given CDC class interface. This should be
			 *  linked to the library \ref EVENT_USB_Device_ControlRequest() event.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 */
			void CDC_Device_ProcessControlRequest(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** General management task for a given CDC class interface, required for the correct operation of the interface. This should
			 *  be called frequently in the main program loop, before the master USB management task \ref USB_USBTask().
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 */
			void CDC_Device_USBTask(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** CDC class driver event for a line encoding change on a CDC interface. This event fires each time the host requests a
			 *  line encoding change (containing the serial parity, baud and other configuration information) and may be hooked in the
			 *  user program by declaring a handler function with the same name and parameters listed here. The new line encoding
			 *  settings are available in the \c LineEncoding structure inside the CDC interface structure passed as a parameter.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 */
			void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** CDC class driver event for a control line state change on a CDC interface. This event fires each time the host requests a
			 *  control line state change (containing the virtual serial control line states, such as DTR) and may be hooked in the
			 *  user program by declaring a handler function with the same name and parameters listed here. The new control line states
			 *  are available in the \c ControlLineStates.HostToDevice value inside the CDC interface structure passed as a parameter, set as
			 *  a mask of \c CDC_CONTROL_LINE_OUT_* masks.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 */
			void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** CDC class driver event for a send break request sent to the device from the host. This is generally used to separate
			 *  data or to indicate a special condition to the receiving device.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     Duration          Duration of the break that has been sent by the host, in milliseconds.
			 */
			void EVENT_CDC_Device_BreakSent(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                                const uint8_t Duration) ATTR_NON_NULL_PTR_ARG(1);

			/** Sends a given data buffer to the attached USB host, if connected. If a host is not connected when the function is
			 *  called, the string is discarded. Bytes will be queued for transmission to the host until either the endpoint bank
			 *  becomes full, or the \ref CDC_Device_Flush() function is called to flush the pending data to the host. This allows
			 *  for multiple bytes to be packed into a single endpoint packet, increasing data throughput.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     Buffer            Pointer to a buffer containing the data to send to the device.
			 *  \param[in]     Length            Length of the data to send to the host.
			 *
			 *  \return A value from the \ref Endpoint_Stream_RW_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_SendData(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                            const void* const Buffer,
			                            const uint16_t Length) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);

			/** Sends a given data buffer from PROGMEM space to the attached USB host, if connected. If a host is not connected when the
			 *  function is called, the string is discarded. Bytes will be queued for transmission to the host until either the endpoint
			 *  bank becomes full, or the \ref CDC_Device_Flush() function is called to flush the pending data to the host. This allows
			 *  for multiple bytes to be packed into a single endpoint packet, increasing data throughput.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     Buffer            Pointer to a buffer containing the data to send to the device.
			 *  \param[in]     Length            Length of the data to send to the host.
			 *
			 *  \return A value from the \ref Endpoint_Stream_RW_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_SendData_P(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                            const void* const Buffer,
			                            const uint16_t Length) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);

			/** Sends a given null terminated string to the attached USB host, if connected. If a host is not connected when
			 *  the function is called, the string is discarded. Bytes will be queued for transmission to the host until either
			 *  the endpoint bank becomes full, or the \ref CDC_Device_Flush() function is called to flush the pending data to
			 *  the host. This allows for multiple bytes to be packed into a single endpoint packet, increasing data throughput.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     String            Pointer to the null terminated string to send to the host.
			 *
			 *  \return A value from the \ref Endpoint_Stream_RW_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_SendString(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                              const char* const String) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);

			/** Sends a given null terminated string from PROGMEM space to the attached USB host, if connected. If a host is not connected
			 *  when the function is called, the string is discarded. Bytes will be queued for transmission to the host until either
			 *  the endpoint bank becomes full, or the \ref CDC_Device_Flush() function is called to flush the pending data to
			 *  the host. This allows for multiple bytes to be packed into a single endpoint packet, increasing data throughput.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     String            Pointer to the null terminated string to send to the host.
			 *
			 *  \return A value from the \ref Endpoint_Stream_RW_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_SendString_P(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                              const char* const String) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);

			/** Sends a given byte to the attached USB host, if connected. If a host is not connected when the function is called, the
			 *  byte is discarded. Bytes will be queued for transmission to the host until either the endpoint bank becomes full, or the
			 *  \ref CDC_Device_Flush() function is called to flush the pending data to the host. This allows for multiple bytes to be
			 *  packed into a single endpoint packet, increasing data throughput.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in]     Data              Byte of data to send to the host.
			 *
			 *  \return A value from the \ref Endpoint_WaitUntilReady_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_SendByte(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                            const uint8_t Data) ATTR_NON_NULL_PTR_ARG(1);

			/** Determines the number of bytes received by the CDC interface from the host, waiting to be read. This indicates the number
			 *  of bytes in the OUT endpoint bank only, and thus the number of calls to \ref CDC_Device_ReceiveByte() which are guaranteed to
			 *  succeed immediately. If multiple bytes are to be received, they should be buffered by the user application, as the endpoint
			 *  bank will not be released back to the USB controller until all bytes are read.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *
			 *  \return Total number of buffered bytes received from the host.
			 */
			uint16_t CDC_Device_BytesReceived(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** Reads a byte of data from the host. If no data is waiting to be read of if a USB host is not connected, the function
			 *  returns a negative value. The \ref CDC_Device_BytesReceived() function may be queried in advance to determine how many
			 *  bytes are currently buffered in the CDC interface's data receive endpoint bank, and thus how many repeated calls to this
			 *  function which are guaranteed to succeed.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *
			 *  \return Next received byte from the host, or a negative value if no data received.
			 */
			int16_t CDC_Device_ReceiveByte(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** Flushes any data waiting to be sent, ensuring that the send buffer is cleared.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *
			 *  \return A value from the \ref Endpoint_WaitUntilReady_ErrorCodes_t enum.
			 */
			uint8_t CDC_Device_Flush(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			/** Sends a Serial Control Line State Change notification to the host. This should be called when the virtual serial
			 *  control lines (DCD, DSR, etc.) have changed states, or to give BREAK notifications to the host. Line states persist
			 *  until they are cleared via a second notification. This should be called each time the CDC class driver's
			 *  \c ControlLineStates.DeviceToHost value is updated to push the new states to the USB host.
			 *
			 *  \pre This function must only be called when the Device state machine is in the \ref DEVICE_STATE_Configured state or
			 *       the call will fail.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 */
			void CDC_Device_SendControlLineStateChange(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) ATTR_NON_NULL_PTR_ARG(1);

			#if defined(FDEV_SETUP_STREAM) || defined(__DOXYGEN__)
			/** Creates a standard character stream for the given CDC Device instance so that it can be used with all the regular
			 *  functions in the standard <stdio.h> library that accept a \c FILE stream as a destination (e.g. \c fprintf()). The created
			 *  stream is bidirectional and can be used for both input and output functions.
			 *
			 *  Reading data from this stream is non-blocking, i.e. in most instances, complete strings cannot be read in by a single
			 *  fetch, as the endpoint will not be ready at some point in the transmission, aborting the transfer. However, this may
			 *  be used when the read data is processed byte-per-bye (via \c getc()) or when the user application will implement its own
			 *  line buffering.
			 *
			 *  \note The created stream can be given as \c stdout if desired to direct the standard output from all \c <stdio.h> functions
			 *        to the given CDC interface.
			 *        \n\n
			 *
			 *  \note This function is not available on all microcontroller architectures.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in,out] Stream            Pointer to a FILE structure where the created stream should be placed.
			 */
			void CDC_Device_CreateStream(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                             FILE* const Stream) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);

			/** Identical to \ref CDC_Device_CreateStream(), except that reads are blocking until the calling stream function terminates
			 *  the transfer. While blocking, the USB and CDC service tasks are called repeatedly to maintain USB communications.
			 *
			 *  \note This function is not available on all microcontroller architectures.
			 *
			 *  \param[in,out] CDCInterfaceInfo  Pointer to a structure containing a CDC Class configuration and state.
			 *  \param[in,out] Stream            Pointer to a FILE structure where the created stream should be placed.
			 */
			void CDC_Device_CreateBlockingStream(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
			                                     FILE* const Stream) ATTR_NON_NULL_PTR_ARG(1) ATTR_NON_NULL_PTR_ARG(2);
			#endif


		/* Function Prototypes: */
		#if defined(__INCLUDE_FROM_CDC_DEVICE_C)
			#if defined(FDEV_SETUP_STREAM)
			static int CDC_Device_putchar(char c,
				                            FILE* Stream) ATTR_NON_NULL_PTR_ARG(2);
			static int CDC_Device_getchar(FILE* Stream) ATTR_NON_NULL_PTR_ARG(1);
			static int CDC_Device_getchar_Blocking(FILE* Stream) ATTR_NON_NULL_PTR_ARG(1);
			#endif

			void CDC_Device_Event_Stub(void) ATTR_CONST;

			void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
				                                        ATTR_WEAK ATTR_NON_NULL_PTR_ARG(1) ATTR_ALIAS(CDC_Device_Event_Stub);
			void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
				                                            ATTR_WEAK ATTR_NON_NULL_PTR_ARG(1) ATTR_ALIAS(CDC_Device_Event_Stub);
			void EVENT_CDC_Device_BreakSent(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo,
				                            const uint8_t Duration) ATTR_WEAK ATTR_NON_NULL_PTR_ARG(1)
				                            ATTR_ALIAS(CDC_Device_Event_Stub);
		#endif

#endif



/** @} */

