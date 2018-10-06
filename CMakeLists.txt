project(guard)
cmake_minimum_required(VERSION 2.6)

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")

file(GLOB SOURCE
    "guard/*.cpp"
    "guard/*.c"
)
file(GLOB HEADERS
    "guard/*.h"
)
set(CMAKE_MODULE_PATH CMake/Modules)

add_definitions( -DBOOST_ALL_DYN_LINK )

add_executable(guard ${SOURCE} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME guard)

if(UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
	set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)
endif()

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
	target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
	set(BOOST_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/boost/include")
	set(BOOST_LIBRARYDIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/boost/lib")
endif()

if (NOT WIN32)
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread" )
	SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl" )
endif()

find_package(BNCSUTIL REQUIRED)
find_package(MYSQL REQUIRED)
find_package(Boost COMPONENTS filesystem system date_time thread chrono REQUIRED)

include_directories(guard ${BNCSUTIL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${BNCSUTIL_LIBRARIES} ${MYSQL_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS guard RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

#CPack configurtion
SET(CPACK_GENERATOR "DEB" "RPM")
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Battle.Net DotA league management bot")
SET(CPACK_PACKAGE_VENDOR "eurobattle.net")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")

#DEB configuration
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/cen1/bnet-guard")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "imbacen@gmail.com")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "bncsutil, libboost-filesystem-dev, libboost-system-dev, libboost-thread-dev, libboost-chrono-dev, libboost-date-time-dev, libmysql++-dev")

#RPM configuration
SET(CPACK_RPM_PACKAGE_RELEASE 1)
SET(CPACK_RPM_PACKAGE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_RPM_PACKAGE_GROUP "${PROJECT_NAME}")
SET(CPACK_RPM_PACKAGE_URL "https://github.com/cen1/bnet-guard")
SET(CPACK_RPM_PACKAGE_REQUIRES "bncsutil, libboost-filesystem-dev, libboost-system-dev, libboost-thread-dev, libboost-chrono-dev, libboost-date-time-dev, libmysql++-dev")
SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${ARCH})

INCLUDE(CPack)
