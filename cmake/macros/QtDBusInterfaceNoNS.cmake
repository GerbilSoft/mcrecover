# Based on Qt4Macros.cmake from CMake 2.8.4-r1 (Gentoo).
# Main change is disabling namespaces in output, since this causes problems
# when using e.g. org.freedesktop.UDisks and org.freedesktop.UDisks.Device
# in the same program.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

MACRO(QT4_ADD_DBUS_INTERFACE_NONS _sources _interface _basename _include)
  GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  # handling more arguments (as in FindQt4.cmake from KDE4) will come soon, then
  # _params will be used for more than just -m
  SET(_params -m -N)
  IF(NOT "${_include}" STREQUAL "")
    SET(_params ${_params} -i "${_include}")
  ENDIF()

  ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
      DEPENDS ${_infile})

  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC(${_header} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

ENDMACRO(QT4_ADD_DBUS_INTERFACE_NONS)

INCLUDE(CMakeParseArguments)
MACRO(QT4_ADD_DBUS_INTERFACES_NONS _sources)
  CMAKE_PARSE_ARGUMENTS(DBUSXML2CPP "" "INCLUDE" "" ${ARGN})
  FOREACH (_current_FILE ${DBUSXML2CPP_UNPARSED_ARGUMENTS})
    GET_FILENAME_COMPONENT(_infile ${_current_FILE} ABSOLUTE)
    # get the part before the ".xml" suffix
    STRING(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2" _basename ${_current_FILE})
    STRING(TOLOWER ${_basename} _basename)
    QT4_ADD_DBUS_INTERFACE_NONS(${_sources} ${_infile} ${_basename}interface "${DBUSXML2CPP_INCLUDE}")
  ENDFOREACH (_current_FILE)
ENDMACRO(QT4_ADD_DBUS_INTERFACES_NONS)
