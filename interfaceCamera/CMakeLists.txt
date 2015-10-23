INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include
					 ${PROJECT_SOURCE_DIR}/commonInclude
					 ${OpenCV_INCLUDE_DIR})

# build and link library
add_library (camera STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaceCamera.cpp)
target_link_libraries (camera ${OpenCV_LIBRARIES})

# build and link executable and add to tests
add_executable (runcamera ${CMAKE_CURRENT_SOURCE_DIR}/src/runcamera.c++)
target_link_libraries (runcamera camera)
add_test (INTERFACECAMERA runcamera)

# for make install
SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
INSTALL (TARGETS camera ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)