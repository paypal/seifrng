INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include
					 ${PROJECT_SOURCE_DIR}/commonInclude
					 ${CRYPTO++_INCLUDE_DIR})


# build and link library
add_library (osrng STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaceOSRNG.cpp)
target_link_libraries (osrng ${CRYPTO++_LIBRARIES})

# build and link executable and add to tests
add_executable (runosrng ${CMAKE_CURRENT_SOURCE_DIR}/src/runosrng.c++)
target_link_libraries (runosrng osrng)
add_test (INTERFACEOSRNG runosrng)

# for make install
SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
INSTALL (TARGETS osrng ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)