/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Several useful debugging functions/annotations for code.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#ifndef DEBUG_H__
#define DEBUG_H__

/*************************************************************************/

#ifdef _MSC_VER
// Microsoft provides some handy static analysis keywords, pull those in.
# include <sal.h>
#else
// For non MS platforms, we'll just define these as blank.
# define _Out_
# define _Out_opt_
# define _In_
# define _In_opt_
# define _Inout_
# define _Inout_opt_
# define _Outptr_
# define _Outptr_opt_
#endif

/*************************************************************************/

#include <assert.h>

#ifdef _DEBUG
// Just mapping this to the C assert() function for now.
# define ASSERT(X_) assert(X_)
#else
# define ASSERT(X_) (void)(X_)
#endif

#define VERIFY(X_) assert(X_)

/*************************************************************************/

#endif /* DEBUG_H__ */

/*************************************************************************/

