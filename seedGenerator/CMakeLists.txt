INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include
					 ${PROJECT_SOURCE_DIR}/commonInclude
					 ${CRYPTO++_INCLUDE_DIR})

#build and link library
add_library (seedGenerator STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/seedGenerator.cpp)
target_link_libraries (seedGenerator ${CRYPTO++_LIBRARIES})

#for make install
SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
INSTALL (TARGETS seedGenerator ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)