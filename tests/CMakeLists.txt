add_executable(testgecko testgecko.cpp)
target_link_libraries(testgecko gecko)
if(HAVE_LIBM_MATH)
  target_link_libraries(testgecko m)
endif()
add_test(NAME basic-test COMMAND testgecko)
