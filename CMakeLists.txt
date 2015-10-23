cmake_minimum_required (VERSION 2.8)
project (isaacrng)

SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

find_package (OpenCV REQUIRED)
find_package (Crypto++ REQUIRED)
find_package (Portaudio REQUIRED)

SET (CMAKE_CXX_FLAGS "-fPIC -std=c++11")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

enable_testing()

add_subdirectory (interfaceOSRNG)
add_subdirectory (interfaceCamera)
add_subdirectory (interfaceMicrophone)
add_subdirectory (fileCryptopp)
add_subdirectory (seedGenerator)
add_subdirectory (isaacRandomPool)