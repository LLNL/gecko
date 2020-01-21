set(gecko_source
  drawing.cpp
  graph.cpp
  heap.h
  options.h
  subgraph.cpp
  subgraph.h
  version.cpp)

add_library(gecko ${gecko_source})
add_library(gecko::gecko ALIAS gecko)

if(HAVE_LIBM_MATH)
  target_link_libraries(gecko PRIVATE m)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
  # Define GECKO_SOURCE when compiling libgecko to export symbols to Windows DLL
  list(APPEND gecko_public_defs GECKO_SHARED_LIBS)
  list(APPEND gecko_private_defs GECKO_SOURCE)
endif()

target_compile_definitions(gecko
  PRIVATE ${gecko_private_defs}
  PUBLIC ${gecko_public_defs}
)

target_include_directories(gecko
  PUBLIC
    $<BUILD_INTERFACE:${GECKO_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_property(TARGET gecko PROPERTY VERSION ${GECKO_VERSION})
set_property(TARGET gecko PROPERTY SOVERSION ${GECKO_VERSION_MAJOR})
set_property(TARGET gecko PROPERTY OUTPUT_NAME ${GECKO_LIBRARY_PREFIX}gecko)

install(TARGETS gecko EXPORT gecko-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
