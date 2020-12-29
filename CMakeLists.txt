cmake_minimum_required(VERSION 3.11)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

project(CryptoShloma LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QML_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resources/qml CACHE PATH "QML directory (for QtCtreator qml parser)")
set(QML_IMPORT_PATH ${QML_PATH} CACHE PATH "QML import directory (for QtCtreator qml parser)")
set(QML2_IMPORT_PATH ${QML_PATH} CACHE PATH "QML2 import directory (for QtCtreator qml parser)")

include(cmake/version.cmake)
include(cmake/utils.cmake)

enable_all_warnings()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/share/libs/CuteLogger)
include(cmake/stacktrace.cmake)

if ( UNIX )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2" )
endif ()

find_package(Qt5 5.12.10 REQUIRED COMPONENTS Core Quick Network Svg QuickControls2 Sql WebSockets)
find_package(OpenSSL REQUIRED)
file(GLOB_RECURSE HEADER_FILES "src/*.h" "src/*.hpp" "include/*")
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" )
file(GLOB_RECURSE RESOURCE_FILES "src/*.qrc")

if(WIN32)
    include(cmake/generate_product_version.cmake)
    generate_product_version(
       VersionFilesOutputVariable
       NAME "${PROJECT_NAME}"
       ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/ui_icons/rabbi.ico"
       VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR}
       VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR}
       VERSION_PATCH  ${${PROJECT_NAME}_VERSION_PATCH}
       COMPANY_COPYRIGHT "https://github.com/man-k28/CryptoShloma"
    )
    add_executable(${PROJECT_NAME} WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} ${VersionFilesOutputVariable})
    target_link_options(${TARGET_NAME} PRIVATE -mwindows)
else()
    add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_include_directories(${PROJECT_NAME} PRIVATE src include ${OPENSSL_INCLUDE_DIR} ${BREAKPAD_INCLUDE_DIRS})

link_directories(BEFORE ${OPENSSL_ROOT_DIR/lib} ${BREAKPAD_LIBRARY_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Sql
    Qt5::WebSockets
    CuteLogger
    ${OPENSSL_LIBRARIES}
#    ${BREAKPAD_LIBRARY_DIRS}/lib${BREAKPAD_LIBRARIES}.a
#    -lpthread
)

#find_package(Qt5 QUIET COMPONENTS QmlImportScanner)
#if (QmlImportScanner_FOUND)
#    qt5_import_qml_plugins(${PROJECT_NAME})
#endif()

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data/24hr.json ${CMAKE_BINARY_DIR}/test_data/24hr.json COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data/allOrders.json ${CMAKE_BINARY_DIR}/test_data/allOrders.json COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data/exchangeInfo.json ${CMAKE_BINARY_DIR}/test_data/exchangeInfo.json COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data/myTrades.json ${CMAKE_BINARY_DIR}/test_data/myTrades.json COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data/balance.json ${CMAKE_BINARY_DIR}/test_data/balance.json COPYONLY)

include(cmake/deploy.cmake)

add_subdirectory(launcher)
