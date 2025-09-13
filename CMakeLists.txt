cmake_minimum_required(VERSION 3.20)

project(CNumLibrary
	VERSION 0.1.0
	LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

option(MSVC "Using MSVC compiler" OFF)
option(BUILD_TESTS "Build test harness" ON)

file(GLOB_RECURSE CNUM_HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.tpp")
file(GLOB_RECURSE CNUM_SOURCE CONFIGURE_DEPENDS "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cpp")

add_library(CNum SHARED
	    ${CNUM_HEADERS}
	    ${CNUM_SOURCE})

add_library(CNum::CNum ALIAS CNum)

if (MSVC)
	target_compile_options(CNum PUBLIC /fp:fast)
else()	
	target_compile_options(CNum PUBLIC -Ofast -fno-math-errno -fno-trapping-math -ffp-contract=fast -Wno-terminate)
	target_link_libraries(CNum PUBLIC pthread atomic)
endif()	

include(GNUInstallDirs)
target_include_directories(CNum PUBLIC
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

include(CMakePackageConfigHelpers)
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

set_target_properties(CNum PROPERTIES
		      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cnum)

if (BUILD_TESTS)
   add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()
