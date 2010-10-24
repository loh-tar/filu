# - Find ta_lib
# Find the ta-lib library
#
#  This module defines the following variables:
#     TA_LIB_FOUND        - True if TA_LIB_INCLUDE_DIR & TA_LIB_LIBRARY are found
#     TA_LIB_LIBRARIES    - Set when TA_LIB_LIBRARY is found
#     TA_LIB_INCLUDE_DIRS - Set when TA_LIB_INCLUDE_DIR is found
#
#     TA_LIB_INCLUDE_DIR  - where to find ta_abstract.h etc.
#     TA_LIB_LIBRARY      - the ta-lib library
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

find_path(TA_LIB_INCLUDE_DIR NAMES ta_abstract.h
          PATH /usr/local/include/ /usr/include/
          PATH_SUFFIXES ta-lib
          DOC "The TA_LIB include directory"
)

find_library(TA_LIB_LIBRARY NAMES ta_lib
          DOC "The TA_LIB library"
)

# handle the QUIETLY and REQUIRED arguments and set TA_LIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ta_lib DEFAULT_MSG TA_LIB_LIBRARY TA_LIB_INCLUDE_DIR)

if(TA_LIB_FOUND)
  set(TA_LIB_LIBRARIES ${TA_LIB_LIBRARY})
  set(TA_LIB_INCLUDE_DIRS ${TA_LIB_INCLUDE_DIR})
endif()

mark_as_advanced(TA_LIB_INCLUDE_DIR TA_LIB_LIBRARY)
