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
 *  \brief Special function/variable attribute macros.
 *
 *  \copydetails Group_FuncVarAttributes
 *
 *  \note Do not include this file directly, rather include the Common.h header file instead to gain this file's
 *        functionality.
 */

/** \ingroup Group_Common
 *  \defgroup Group_FuncVarAttributes Function/Variable Attributes
 *  \brief Special function/variable attribute macros.
 *
 *  This module contains macros for applying specific attributes to functions and variables to control various
 *  optimizer and code generation features of the compiler. Attributes may be placed in the function prototype
 *  or variable declaration in any order, and multiple attributes can be specified for a single item via a space
 *  separated list.
 *
 *  On incompatible versions of GCC or on other compilers, these macros evaluate to nothing unless they are
 *  critical to the code's function and thus must throw a compile error when used.
 *
 *  @{
 */

#ifndef __LUFA_ATTR_H__
#define __LUFA_ATTR_H__

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_COMMON_H)
			#error Do not include this file directly. Include LUFA/Common/Common.h instead to gain this functionality.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */

			/** Marks a function as an alias for another function.
			 *
			 *  \param[in] Func  Name of the function which the given function name should alias.
			 */
			#define ATTR_ALIAS(Func)                 __attribute__ ((alias( #Func )))

			/** Marks a variable or struct element for packing into the smallest space available, omitting any
			 *  alignment bytes usually added between fields to optimize field accesses.
			 */
			#define ATTR_PACKED                      __attribute__ ((packed))
			
			
			
			
	/* GNU3 */

				/** Indicates that the function returns a value which should not be ignored by the user code. When
				 *  applied, any ignored return value from calling the function will produce a compiler warning.
				 */
				#define ATTR_WARN_UNUSED_RESULT      __attribute__ ((warn_unused_result))

				/** Indicates that the specified parameters of the function are pointers which should never be \c NULL.
				 *  When applied as a 1-based comma separated list the compiler will emit a warning if the specified
				 *  parameters are known at compiler time to be \c NULL at the point of calling the function.
				 */
				#define ATTR_NON_NULL_PTR_ARG(...)   __attribute__ ((nonnull (__VA_ARGS__)))




				/** Forces the compiler to inline the specified function. When applied, the given function will be
				 *  in-lined under all circumstances.
				 */
				#define ATTR_ALWAYS_INLINE           __attribute__ ((always_inline))

				/** Indicates that the specified function is pure, in that it has no side-effects other than global
				 *  or parameter variable access.
				 */
				#define ATTR_PURE                    __attribute__ ((pure))

				/** Indicates that the specified function is constant, in that it has no side effects other than
				 *  parameter access.
				 */
				#define ATTR_CONST                   __attribute__ ((const))

				/** Marks a given function as deprecated, which produces a warning if the function is called. */
				#define ATTR_DEPRECATED              __attribute__ ((deprecated))

				/** Marks a function as a weak reference, which can be overridden by other functions with an
				 *  identical name (in which case the weak reference is discarded at link time).
				 */
				#define ATTR_WEAK                    __attribute__ ((weak))

#endif

/** @} */

