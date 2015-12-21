CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Cameradae)

SET(CMAKE_INSTALL_PREFIX "/usr")

# -----------------------------------------------------------------------------
# Required platform modules
# -----------------------------------------------------------------------------
INCLUDE(FindPkgConfig)
INCLUDE(GNUInstallDirs)

find_package(PkgConfig)

PKG_CHECK_MODULES(glib REQUIRED glib-2.0)
PKG_CHECK_MODULES(gio REQUIRED gio-2.0)
PKG_CHECK_MODULES(dbus REQUIRED dbus-1)
# PKG_CHECK_MODULES(dlog REQUIRED dlog)
PKG_CHECK_MODULES(DLT REQUIRED automotive-dlt)
PKG_CHECK_MODULES(gstreamer REQUIRED gstreamer-1.0)

INCLUDE_DIRECTORIES(
  ${glib_INCLUDE_DIRS}
  ${gio_INCLUDE_DIRS}
  ${dbus_INCLUDE_DIRS}
 # ${dlog_INCLUDE_DIRS}
  ${DLT_INCLUDE_DIRS}
  ${gstreamer_INCLUDE_DIRS}
 include
)

# -----------------------------------------------------------------------------
# Determine the time tracing option
# -----------------------------------------------------------------------------

OPTION(ENABLE_TIME_TRACER "TIME TRACING" OFF)

IF(ENABLE_TIME_TRACER)
    MESSAGE(STATUS "Time tracer enabled")
    ADD_DEFINITIONS("-DENABLE_TIME_TRACER")
ELSE(ENABLE_TIME_TRACER)
    MESSAGE(STATUS "Time tracer disabled")
ENDIF(ENABLE_TIME_TRACER)

# -----------------------------------------------------------------------------
# Set build type (Release by default)
# -----------------------------------------------------------------------------
IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
     SET(CMAKE_BUILD_TYPE Release)
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

# -----------------------------------------------------------------------------
# CFlags
# -----------------------------------------------------------------------------
SET(CMAKE_C_FLAGS_PROFILING    "-O0 -g -pg")
SET(CMAKE_CXX_FLAGS_PROFILING  "-O0 -std=c++0x -g -pg")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG      "-O0 -std=c++0x -g")
SET(CMAKE_C_FLAGS_RELEASE      "-O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE    "-O2 -std=c++0x -g")
ADD_DEFINITIONS("-DCLIENT_IPC_THREAD")
ADD_DEFINITIONS("-DEXPORT_API=")
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
#ADD_DEFINITIONS("-Werror")
#ADD_DEFINITIONS("-Wextra")
ADD_DEFINITIONS("-DAPPLICATION_API_BACKWARD_COMPATIBILITY")
ADD_DEFINITIONS("-std=c++11")

# -----------------------------------------------------------------------------
# Global variables
# -----------------------------------------------------------------------------

SET(DESTINATION_PREFIX bin)
SET(DBUS_SERVICE_PREFIX share/dbus-1/services)
SET(SYSTEMD_SERVICE_PREFIX lib/systemd/user)
#SET(DESTINATION_PREFIX lib)

# -----------------------------------------------------------------------------
# Macros for pkgconfig
# -----------------------------------------------------------------------------
SET(PKGCONFIG_DIR ${CMAKE_SOURCE_DIR}/pkgconfig)

MACRO(configure_and_install_pkg PKG_FILE)
    CONFIGURE_FILE(${PKGCONFIG_DIR}/${PKG_FILE}.in
               ${PKGCONFIG_DIR}/${PKG_FILE} @ONLY)
    INSTALL(FILES ${PKGCONFIG_DIR}/${PKG_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
ENDMACRO(configure_and_install_pkg)

SET(TARGET_NAME Cameradae)

configure_and_install_pkg(systemupdate.pc)

SET(SRCS src/command-adapter-thread.c
         src/dbus-api.c
         src/fsm-thread.c
         src/gst-pipeline-thread.c
         src/ivos-cameradae-service.c
         src/send-signal.c
)

ADD_EXECUTABLE(${TARGET_NAME} ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_NAME}
                      ${glib_LDFLAGS}
                      ${gio_LDFLAGS}
                      ${dbus_LDFLAGS}
                      ${DLT_LIBRARIES}
                      
        -lgstreamer-1.0
        -lgstvideo-1.0
			  -lpthread
		      -lgcrypt 
		      -lgpg-error
		     # -lwebsockets
)
INSTALL(TARGETS ${TARGET_NAME} DESTINATION ${DESTINATION_PREFIX})


