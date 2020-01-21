// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and other
// gecko project contributors. See the top-level LICENSE file for details.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GECKO_H
#define GECKO_H

// stringification
#define _gecko_str_(x) # x
#define _gecko_str(x) _gecko_str_(x)

#define GECKO_VERSION_MAJOR 1 // library major version number
#define GECKO_VERSION_MINOR 0 // library minor version number
#define GECKO_VERSION_PATCH 0 // library patch version number

// library version number (see also gecko_version)
#define GECKO_VERSION \
  ((GECKO_VERSION_MAJOR << 8) + \
   (GECKO_VERSION_MINOR << 4) + \
   (GECKO_VERSION_PATCH << 0))

// library version string (see also gecko_version_string)
#define GECKO_VERSION_STRING \
  _gecko_str(GECKO_VERSION_MAJOR) "." \
  _gecko_str(GECKO_VERSION_MINOR) "." \
  _gecko_str(GECKO_VERSION_PATCH)

// extern_ macro for exporting and importing symbols
#if defined(_MSC_VER) && defined(GECKO_SHARED_LIBS)
  // export (import) symbols when GECKO_SOURCE is (is not) defined
  #ifdef GECKO_SOURCE
    // export symbols */
    #define extern_ extern __declspec(dllexport)
  #else
    // import symbols
    #define extern_ extern __declspec(dllimport)
  #endif
#else // !(_MSC_VER && GECKO_SHARED_LIBS)
  #define extern_ extern
#endif

namespace Gecko {

// public data
extern_ const unsigned int version; // library version GECKO_VERSION
extern_ const char* const version_string; // verbose version string

}

#endif
