# - Find Crypto++

if(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)
   set(CRYPTO++_FOUND TRUE)

else(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)
  find_path(CRYPTO++_INCLUDE_DIR 
  	NAMES
  		cryptlib.h
  	PATHS
	    /usr/include/crypto++
		/usr/include/cryptopp
		/usr/local/include/crypto++
		/usr/local/include/cryptopp
		/opt/local/include/crypto++
		/opt/local/include/cryptopp
		$ENV{SystemDrive}/Crypto++/include
  )
  

  find_library(CRYPTO++_LIBRARIES 
  	NAMES 
  		libcryptopp.a 
  		cryptopp 
  		cryptlib
    PATHS
      	/usr/lib
      	/usr/local/lib
      	/opt/local/lib
      	$ENV{SystemDrive}/Crypto++/lib
      	${CRYPTO++_LIBRARIES_DIR}
  )
  

  if(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)
    set(CRYPTO++_FOUND TRUE)
    message(STATUS "Found Crypto++: ${CRYPTO++_INCLUDE_DIR}, ${CRYPTO++_LIBRARIES}")
  else(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)
    message(STATUS "Found: ${CRYPTO++_INCLUDE_DIR}")
    set(CRYPTO++_FOUND FALSE)
    message(STATUS "Crypto++ not found.")
  endif(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)

  mark_as_advanced(CRYPTO++_INCLUDE_DIR CRYPTO++_LIBRARIES)

endif(CRYPTO++_INCLUDE_DIR AND CRYPTO++_LIBRARIES)