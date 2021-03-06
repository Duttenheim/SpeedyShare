CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(speedyshare)

# The version number.
SET(speedyshare_VERSION_MAJOR 1)
SET(speedyshare_VERSION_MINOR 0)
 
# configure a header file to pass some of the CMake settings
# to the source code
CONFIGURE_FILE(
  "code/version.h.in"
  "../code/version.h"
)
  
SET(QT_USE_QTNETWORK TRUE)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
OPTION(USE_QT5 "Uses Qt5 instead of Qt4" FALSE)
SET(PROJECTBINDIR "../bin/")
SET(TARGETNAME speedyshare)


IF(USE_QT5)
	SET(CMAKE_INCLUDE_CURRENT_DIR ON)
	SET(CMAKE_AUTOMOC ON)
	SET(CMAKE_PREFIX_PATH $ENV{QT5DIR})
	FIND_PACKAGE(Qt5Core REQUIRED)
	FIND_PACKAGE(Qt5Widgets REQUIRED)
	FIND_PACKAGE(Qt5Network REQUIRED)
ELSE()
	FIND_PACKAGE(Qt4 REQUIRED)
ENDIF()

FILE(GLOB SRC "code/*.cpp")
FILE(GLOB HEAD "code/*.h")
SOURCE_GROUP(speedyshare FILES ${SRC} ${HEAD})
LIST(APPEND CODE ${SRC} ${HEAD})

SET(MOC     "code/datareceiver.h"
            "code/datasender.h"
            "code/datareceiverhandler.h"
            "code/receiverthread.h"
            "code/senderthread.h"
            "code/speedyshare.h"
)

FILE(GLOB UI "ui/*.ui")

IF(USE_QT5)
	QT5_WRAP_CPP(speedyshare_headers_moc ${MOC})
	QT5_WRAP_UI(speedyshare_ui ${UI})
	QT5_ADD_RESOURCES(speedyshare_res "ui/resources.qrc")
	ADD_DEFINITIONS(__QT5__)
ELSE()
	QT4_WRAP_CPP(speedyshare_headers_moc ${MOC})
	QT4_WRAP_UI(speedyshare_ui ${UI})
	QT4_ADD_RESOURCES(speedyshare_res "ui/resources.qrc")
	INCLUDE(${QT_USE_FILE})
	ADD_DEFINITIONS(${QT_DEFINITIONS})
ENDIF()


ADD_EXECUTABLE(speedyshare ${CODE} ${UI} ${speedyshare_headers_moc})
TARGET_LINK_LIBRARIES(speedyshare ${QT_LIBRARIES})

SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES DEBUG_POSTFIX ".debug")

SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECTBINDIR})
SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECTBINDIR})
SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECTBINDIR})
SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECTBINDIR})
SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECTBINDIR})	

IF(WIN32)
	SET_TARGET_PROPERTIES(${TARGETNAME} PROPERTIES WIN32_EXECUTABLE ON)
ENDIF()

IF(MSVC)
	SET(CMAKE_EXE_LINKER_FLAGS
		"/MANIFEST:NO"
		CACHE STRING "Flags used by the linker when building an executable." FORCE
	)
	
	# C++ compiler		
	SET(CMAKE_CXX_FLAGS 
		"/D__WIN32__ /DWIN32 /D_HAS_EXCEPTIONS=0 /GF /fp:fast /W1 /WX /MP /nologo /errorReport:prompt" 
		CACHE STRING "Flags used by the compiler during all build types." FORCE)
	SET(CMAKE_CXX_FLAGS_DEBUG
		"/D_DEBUG /Od /RTC1 /MTd /Gy /ZI"
		CACHE STRING "Flags used by the compiler during debug builds." FORCE)
	SET(CMAKE_CXX_FLAGS_RELEASE
		"/Ob2 /Oi /Ot /Oy /GL /FD /MT /GS-"
		CACHE STRING "Flags used by the compiler during release builds." FORCE)
ENDIF()	