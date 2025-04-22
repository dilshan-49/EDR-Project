
 /*  Macros and functions to create and control global interrupts within the device.
 */

#ifndef __LUFA_COMMON_H__
#define __LUFA_COMMON_H__

	/* Macros: */
		#define __INCLUDE_FROM_COMMON_H

	/* Includes: */
		#include <stdint.h>
		#include <stdbool.h>
		#include <string.h>
		#include <stddef.h>

		#include "Architectures.h"
		#include "CompilerSpecific.h"
		#include "Attributes.h"

		#if defined(USE_LUFA_CONFIG_HEADER)
			#include "../../LUFAConfig.h"
		#endif


	/* AVR 8 Architecture specific utility includes: */
			#include <avr/io.h>
			#include <avr/interrupt.h>
			#include <avr/pgmspace.h>
			#include <avr/eeprom.h>
			#include <avr/boot.h>
			#include <math.h>
			#include <util/delay.h>

			typedef uint8_t uint_reg_t;

			#define ARCH_HAS_EEPROM_ADDRESS_SPACE
			#define ARCH_HAS_FLASH_ADDRESS_SPACE
			#define ARCH_HAS_MULTI_ADDRESS_SPACE
			#define ARCH_LITTLE_ENDIAN

			#include "Endianness.h"
		




#endif

/** @} */

