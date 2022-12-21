cmake_minimum_required(VERSION 3.6)

include(GNUInstallDirs)

find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})


add_library(camerajni SHARED org_libcamera_apps_LibcameraApps.h libcamerajni.cpp)

target_link_libraries(camerajni libcamera_app images)

install(TARGETS images LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
