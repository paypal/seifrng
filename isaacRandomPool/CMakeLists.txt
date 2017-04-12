INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include
					 ${PROJECT_SOURCE_DIR}/interfaceOSRNG/include
					 ${PROJECT_SOURCE_DIR}/interfaceCamera/include
					 ${PROJECT_SOURCE_DIR}/interfaceMicrophone/include ${PROJECT_SOURCE_DIR}/seedGenerator/include
					 ${PROJECT_SOURCE_DIR}/commonInclude
					 ${PROJECT_SOURCE_DIR}/fileCryptopp/include
					 ${CRYPTO++_INCLUDE_DIR}
					 ${PORTAUDIO_INCLUDE_DIRS}
					 ${OpenCV_INCLUDE_DIR})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS true)
	SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# set rpath for dylib

	SET (CMAKE_MACOSX_RPATH ON)
	SET (CMAKE_SKIP_BUILD_RPATH FALSE)
	SET (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

	SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

	if ("${isSystemDir}" STREQUAL "-1")
		set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
	endif ("${isSystemDir}" STREQUAL "-1")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

SET (CMAKE_INSTALL_PATH "${PROJECT_SOURCE_DIR}/lib")

# Add Compile Definitions

IF (OpenCV_FOUND)
	ADD_DEFINITIONS(-DWITH_OPENCV=1)
ENDIF (OpenCV_FOUND)

IF (PORTAUDIO_FOUND)
	ADD_DEFINITIONS(-DWITH_PORTAUDIO=1)
ENDIF (PORTAUDIO_FOUND)

# build and link library

add_library (isaacrandompool SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/isaacRandomPool.cpp)

IF (OpenCV_FOUND AND PORTAUDIO_FOUND)
	target_link_libraries (isaacrandompool seedGenerator osrng camera microphone fileCryptopp)
ELSE (OpenCV_FOUND AND PORTAUDIO_FOUND)
	IF (OpenCV_FOUND OR PORTAUDIO_FOUND)
		IF (OpenCV_FOUND)
			target_link_libraries (isaacrandompool seedGenerator osrng camera fileCryptopp)
		ELSE (OpenCV_FOUND)
			target_link_libraries (isaacrandompool seedGenerator osrng microphone fileCryptopp)
		ENDIF (OpenCV_FOUND)
	ELSE (OpenCV_FOUND OR PORTAUDIO_FOUND)
		target_link_libraries (isaacrandompool seedGenerator osrng fileCryptopp)
	ENDIF (OpenCV_FOUND OR PORTAUDIO_FOUND)
ENDIF (OpenCV_FOUND AND PORTAUDIO_FOUND)

# build and link executable and add to tests
add_executable (runisaacrandompool ${CMAKE_CURRENT_SOURCE_DIR}/src/runisaacrandompool.c++)
target_link_libraries (runisaacrandompool isaacrandompool)
add_test (ISAACRANDOMPOOL runisaacrandompool)

# for make install
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	INSTALL (TARGETS isaacrandompool RUNTIME DESTINATION ${CMAKE_INSTALL_PATH})
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	INSTALL (TARGETS isaacrandompool LIBRARY DESTINATION ${CMAKE_INSTALL_PATH})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
