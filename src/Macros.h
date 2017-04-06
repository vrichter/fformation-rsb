/********************************************************************
**                                                                 **
** File   : src/Macros.h                                           **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
********************************************************************/

#pragma once

#include <fformation/Exception.h>
#include <iostream>
#include <sstream>

#define CERR_LOG(m, x)                                                         \
  {                                                                            \
    std::cerr << "[" << m << "]" << __FILE__ << "[" << __LINE__ << "]:\t" x    \
              << std::endl;                                                    \
  }

#define NO_LOG(m, x) // noop

//#define DEBUG_LOG(x) CERR_LOG("DEBUG", x)
#define DEBUG_LOG(x) NO_LOG("DEBUG", x)

//#define WARNING_LOG(x) CERR_LOG("WARNING", x)
#define WARNING_LOG(x) NO_LOG("WARNING", x)

//#define TRACE_LOG(x) CERR_LOG("TRACE", x)
#define TRACE_LOG(x) NO_LOG("TRACE", x)

#define ASSERT(test, message)                                                  \
  {                                                                            \
    if (!(test)) {                                                             \
      std::stringstream str;                                                   \
      str << message;                                                          \
      throw fformation::Exception(str.str());                                  \
    }                                                                          \
  }
