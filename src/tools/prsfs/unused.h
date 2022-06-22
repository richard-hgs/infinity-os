/**
 * @file unused.h
 * @author Infinity Technology
 * @date 02/29/2020
 * @brief Simple header file for unused variables and functions.
 **********************************************************************
 */

#ifndef _UNUSED_H_
#define _UNUSED_H_

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ##x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

#ifdef __GNUC__
#define UNUSED_FUNC(x) __attribute__((unused)) UNUSED_ ##x
#else
#define UNUSED(x) x
#endif

#endif
