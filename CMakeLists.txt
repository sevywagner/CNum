cmake_minimum_required(VERSION 3.20...3.31)

project(CNumLibrary
	VERSION 0.2.1
	LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

option(BUILD_TESTS "Build test harness" OFF)
option(INSTALL_GTEST "Install google test to run the test harness" OFF)
option(DEPLOY_TOOLS "Include dependencies for the Deploy namespace (REST API tools)" OFF)

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(allowed_build_types Release)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${allowed_build_types})

if (NOT CMAKE_BUILD_TYPE IN_LIST allowed_build_types)
   message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (DEPLOY_TOOLS)
	set(ASIO_HEADER_DIR "" CACHE PATH "Dir that contains asio.hpp")
	find_path(ASIO_INCLUDE_DIR
	  	NAMES asio.hpp
	  	HINTS ${ASIO_HEADER_DIR}
	  	DOC "Path to directory containing asio.hpp for Crow")

	if (NOT ASIO_INCLUDE_DIR)
		message(FATAL_ERROR "asio.hpp couldn't be found. Set ASIO_HEADER_DIR, turn off DEPLOY_TOOLS, or install the package if you haven't already")
	endif()

	add_library(CNum_Deploy INTERFACE)
	add_library(CNum::Deploy ALIAS CNum_Deploy)

	target_include_directories(CNum_Deploy 
			   		  INTERFACE
			   		       $<BUILD_INTERFACE:${INCLUDE_DIR}>
			   		       $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
					       $<BUILD_INTERFACE:${ASIO_INCLUDE_DIR}>)

	install(TARGETS CNum_Deploy
		EXPORT CNumTargets)
endif()

if (MSVC)
	set(LIB_TYPE STATIC)
else()
	set(LIB_TYPE SHARED)
endif()

add_library(CNum ${LIB_TYPE})

if (DEPLOY_TOOLS)
   target_compile_definitions(CNum PUBLIC DEPLOY_TOOLS)
endif()

add_subdirectory(src)

add_library(CNum::CNum ALIAS CNum)

if (MSVC)
	target_compile_options(CNum PRIVATE /fp:fast)
else()	
	target_compile_options(CNum PRIVATE -Ofast -fno-math-errno -fno-trapping-math -ffp-contract=fast -ffast-math)
endif()	


target_include_directories(CNum 
			   PUBLIC
			   	$<BUILD_INTERFACE:${INCLUDE_DIR}>
			   	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS CNum
	EXPORT CNumTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/CNum)


write_basic_package_version_file("${CMAKE_BINARY_DIR}/CNumConfigVersion.cmake"
				 VERSION ${PROJECT_VERSION}
				 COMPATIBILITY SameMajorVersion)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CNumConfig.cmake.in"
			      "${CMAKE_BINARY_DIR}/CNumConfig.cmake"
			      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CNum)

install(FILES
	"${CMAKE_BINARY_DIR}/CNumConfigVersion.cmake"
	"${CMAKE_BINARY_DIR}/CNumConfig.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CNum)


install(EXPORT CNumTargets
	FILE CNumTargets.cmake
	NAMESPACE CNum::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CNum)

if (NOT MSVC)
	set_target_properties(CNum PROPERTIES
		      		   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cnum)
endif()

if (BUILD_TESTS)
   add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()
