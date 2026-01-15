add_executable(test_harness test.cpp)

if (INSTALL_GTEST)
	include(FetchContent)
	FetchContent_Declare(googletest
		GIT_REPOSITORY https://github.com/google/googletest
		GIT_TAG v1.15.2)

	FetchContent_GetProperties(googletest)
	if (NOT googletest_POPULATED)
		FetchContent_MakeAvailable(googletest)
	endif()		     	  
endif()

target_link_libraries(test_harness CNum gtest_main gmock_main)

if (NOT MSVC)
   	target_compile_options(test_harness PRIVATE
   		       	    -fsanitize=address -O2)
	target_link_options(test_harness PRIVATE
  		      	    -fsanitize=address)
endif()
