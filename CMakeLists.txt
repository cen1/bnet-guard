cmake_minimum_required(VERSION 3.25.1)
project(guard)

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "2")

set(HEADERS
	"guard/bncsutilinterface.h"
	"guard/bnet.h"
	"guard/bnetprotocol.h"
	"guard/bnlsclient.h"
	"guard/bnlsprotocol.h"
	"guard/commandpacket.h"
	"guard/config.h"
	"guard/crc32.h"
	"guard/csvparser.h"
	"guard/ghost.h"
	"guard/ghostdb.h"
	"guard/ghostdbmysql.h"
	"guard/ghostdbsqlite.h"
	"guard/includes.h"
	"guard/language.h"
	"guard/md5.h"
	"guard/next_combination.h"
	"guard/sha1.h"
	"guard/socket.h"
	"guard/sqlite3.h"
	"guard/sqlite3ext.h"
	"guard/util.h"
)

set(SOURCES
	"guard/bncsutilinterface.cpp"
	"guard/bnet.cpp"
	"guard/bnetprotocol.cpp"
	"guard/bnlsclient.cpp"
	"guard/bnlsprotocol.cpp"
	"guard/commandpacket.cpp"
	"guard/config.cpp"
	"guard/crc32.cpp"
	"guard/csvparser.cpp"
	"guard/ghost.cpp"
	"guard/ghostdb.cpp"
	"guard/ghostdbmysql.cpp"
	"guard/ghostdbsqlite.cpp"
	"guard/language.cpp"
	"guard/md5.cpp"
	"guard/sha1.cpp"
	"guard/socket.cpp"
	"guard/sqlite3.c"
	"guard/util.cpp"
)

add_executable(guard guard.cfg ip-to-country.csv mysql_create_tables.sql)

target_sources(guard PRIVATE ${SOURCES} ${HEADERS})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

target_compile_definitions(guard PRIVATE -DBOOST_ALL_DYN_LINK)

set_target_properties(guard PROPERTIES OUTPUT_NAME guard)
set_target_properties(guard PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set_target_properties(guard PROPERTIES SOVERSION 1)

if (WIN32)
	target_compile_options(guard PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_link_libraries(guard PRIVATE wsock32 ws2_32)
	set(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/boost/include")
	set(BOOST_LIBRARYDIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/boost/lib")
endif()

if (NOT WIN32)
	target_link_libraries(guard PRIVATE -lpthread -lm -lstdc++ ${CMAKE_DL_LIBS})
	set(CMAKE_INSTALL_PREFIX "/opt/bnet-guard")
endif()

if (ENABLE_LIFETIME)
	target_compile_options(guard PRIVATE -Wlifetime -Wno-lifetime-pointer-arithmetic -Wno-lifetime-null)
endif()

find_package(BNCSUTIL REQUIRED)
find_package(MYSQL REQUIRED)
find_package(Boost COMPONENTS filesystem system date_time thread chrono REQUIRED)

target_include_directories(guard PRIVATE ${BNCSUTIL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(guard PRIVATE ${BNCSUTIL_LIBRARIES} ${MYSQL_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS guard RUNTIME DESTINATION . COMPONENT binaries)
install(FILES guard.cfg ip-to-country.csv mysql_create_tables.sql DESTINATION . COMPONENT config)

#CPack configurtion
SET(CPACK_GENERATOR "DEB" "RPM")
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Battle.Net DotA league management bot")
SET(CPACK_PACKAGE_VENDOR "eurobattle.net")
SET(CPACK_PACKAGE_DESCRIPTION "This will install binary and configuration files to /opt/bnet-guard")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")

#DEB configuration
SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/cen1/bnet-guard")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "imbacen@gmail.com")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "bncsutil, libboost-filesystem, libboost-system, libboost-thread, libboost-chrono, libboost-date-time, libmysql++")

#RPM configuration
SET(CPACK_RPM_PACKAGE_RELEASE 1)
SET(CPACK_RPM_PACKAGE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_RPM_PACKAGE_GROUP "${PROJECT_NAME}")
SET(CPACK_RPM_PACKAGE_URL "https://github.com/cen1/bnet-guard")
SET(CPACK_RPM_PACKAGE_REQUIRES "bncsutil, boost-filesystem, boost-system, boost-thread, boost-chrono, boost-date-time, mysql++")
SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${ARCH})

INCLUDE(CPack)

#Tests
enable_testing()
add_subdirectory(test)