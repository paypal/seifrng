INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include
					${PROJECT_SOURCE_DIR}/commonInclude
					${PORTAUDIO_INCLUDE_DIRS})

# build and link library
add_library (microphone STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaceMicrophone.cpp)
target_link_libraries (microphone ${PORTAUDIO_LIBRARIES})


# build and link executable and add to tests
add_executable (runmicrophone ${CMAKE_CURRENT_SOURCE_DIR}/src/runmicrophone.c++)
target_link_libraries (runmicrophone microphone)
add_test (INTERFACEMICROPHONE runmicrophone)

# for make install
SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
INSTALL (TARGETS microphone ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
