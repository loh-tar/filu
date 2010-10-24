# - Find muParser
# Find the muParser library
#
#  This module defines the following variables:
#     MUPARSER_FOUND        - True if MUPARSER_INCLUDE_DIR & MUPARSER_LIBRARY are found
#     MUPARSER_LIBRARIES    - Set when MUPARSER_LIBRARY is found
#     MUPARSER_INCLUDE_DIRS - Set when MUPARSER_INCLUDE_DIR is found
#
#     MUPARSER_INCLUDE_DIR  - where to find muParser.h etc.
#     MUPARSER_LIBRARY      - the muParser library
#

#
# This file based on FindALSA.cmake
#

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(MUPARSER_INCLUDE_DIR NAMES muParser.h
#          PATH /usr/local/include/ /usr/include/
          DOC "The muParser include directory"
)

find_library(MUPARSER_LIBRARY NAMES muparser
          DOC "The muParser library"
)

# handle the QUIETLY and REQUIRED arguments and set MUPARSER_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(muparser DEFAULT_MSG MUPARSER_LIBRARY MUPARSER_INCLUDE_DIR)

if(MUPARSER_FOUND)
  set(MUPARSER_LIBRARIES ${MUPARSER_LIBRARY})
  set(MUPARSER_INCLUDE_DIRS ${MUPARSER_INCLUDE_DIR})
endif()

mark_as_advanced(MUPARSER_INCLUDE_DIR MUPARSER_LIBRARY)
