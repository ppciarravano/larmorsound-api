CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

Project(LarmorSoundAPI)

#SET(CMAKE_CXX_WARNING_LEVEL 4)
SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#SET(CMAKE_CXX_COMPILER /usr/bin/g++)
SET(CMAKE_BUILD_TYPE Release)

SET( WINDOWS FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
    SET( WINDOWS TRUE )
    SET( LIB_OS Windows )
ENDIF()

SET( DARWIN FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
    SET( DARWIN TRUE )
    SET( LIB_OS OSx )
ENDIF()

SET( LINUX FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux" )
    SET( LINUX TRUE )
    SET( LIB_OS Linux-x86_64 )
ENDIF()

IF( ${WINDOWS} )
    ADD_DEFINITIONS( -DPLATFORM_WINDOWS -DPLATFORM=WINDOWS )
ELSEIF( ${DARWIN} )
    ADD_DEFINITIONS( -DPLATFORM_DARWIN -DPLATFORM=DARWIN )
ELSE()
    ADD_DEFINITIONS( -DPLATFORM_LINUX -DPLATFORM=LINUX )
ENDIF()

IF ( NOT WINDOWS )
  IF ( NOT DARWIN )
    # LINUX SECTION
    SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords" )
    SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic" )
  ELSE()
    # DARWIN SECTION
    # NOT TESTED: TODO
    SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -DCC_GNU_ -DOSMac_ -DOSMacOSX_ -DOSMac_MachO_ -DREQUIRE_IOSTREAM -fPIC -fno-gnu-keywords -ftemplate-depth=1024 -D_LANGUAGE_C_PLUS_PLUS" )
    SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -pthread -pipe -framework System  -framework SystemConfiguration -framework CoreServices -framework Carbon -framework Cocoa -framework ApplicationServices -framework IOKit -bundle -fPIC -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic" )
  ENDIF()
ELSE()
    # WINDOWS SECTION
    SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -Os -Wall -c -fmessage-length=0 --param ggc-min-heapsize=2008192" )
    SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s" )
ENDIF()

MESSAGE(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}" )
MESSAGE("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
MESSAGE("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
MESSAGE("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})


# Include directories
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/FabricSound_proj/aubio/aubio-0.4.3/build/src)
INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/FabricSound_proj/aubio/aubio-0.4.3/src)

# Linker libraries directories
LINK_DIRECTORIES(/home/larmor/DEVELOP/FabricSound_proj/aubio/aubio-0.4.3/build/src)

# Linker libraries
SET(PRJ_LIBRARIES
    aubio
    SDL2
)

# Source header files
SET(H_FILES
    ../LarmorSoundAPI/LarmorSoundAPI.h
)

# Source cpp files
SET(CXX_FILES 
    ../LarmorSoundAPI/LarmorSoundAPI.cpp
)

SET( SOURCE_FILES ${CXX_FILES} ${H_FILES} )

ADD_LIBRARY(libLarmorSoundAPI-${LIB_OS} SHARED ${SOURCE_FILES})

SET_TARGET_PROPERTIES( libLarmorSoundAPI-${LIB_OS}
    PROPERTIES
    COMPILE_FLAGS ${PRJ_COMPILE_FLAGS}
    LINK_FLAGS ${PRJ_LINK_FLAGS}
    PREFIX "" )

# To link staticly: still TODO
# TARGET_LINK_LIBRARIES(libLarmorSoundAPI-${LIB_OS} ${PRJ_LIBRARIES})


