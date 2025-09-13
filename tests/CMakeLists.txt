add_executable(test_harness test.cpp)
target_link_libraries(test_harness PRIVATE CNum)
if (MSVC)
   target_compile_options(test_harness PRIVATE /fsanitize=address)
   target_link_options(test_harness PRIVATE /fsanitize=address)
else()
   target_compile_options(test_harness PRIVATE
   		       -fsanitize=address
		       -fsanitize=leak
		       -O2 -g -fno-omit-frame-pointer)

  target_link_options(test_harness PRIVATE
  		      -fsanitize=address
		      -fsanitize=leak) 
endif()
