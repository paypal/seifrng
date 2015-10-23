INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include ${CRYPTO++_INCLUDE_DIR})

# build and link library
add_library (fileCryptopp STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/fileCryptopp.cpp)
target_link_libraries (fileCryptopp ${CRYPTO++_LIBRARIES})

# build and link executable and add to tests
add_executable (runfilecrypto ${CMAKE_CURRENT_SOURCE_DIR}/src/runfilecrypto.c++ )
target_link_libraries (runfilecrypto fileCryptopp)
add_test (FILECRYPTO runfilecrypto)

# for make install
SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
INSTALL (TARGETS fileCryptopp ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)



