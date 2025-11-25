add_executable(test_harness test.cpp)
target_link_libraries(test_harness PRIVATE CNum)
target_include_directories(test_harness PRIVATE ${ASIO_INCLUDE_DIR})

if (NOT MSVC)
   	target_compile_options(test_harness PRIVATE
   		       -fsanitize=address
		       -O2 -g -fno-omit-frame-pointer)
	target_link_options(test_harness PRIVATE
  		      -fsanitize=address) 
endif()
