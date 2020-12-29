cmake_minimum_required(VERSION 3.11)

project(Launcher LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../cmake/utils.cmake)
enable_all_warnings()

find_package(Qt5 5.12.10 REQUIRED COMPONENTS Core)
file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp" )

if(WIN32)
    include(../cmake/generate_product_version.cmake)

    get_filename_component(ICON_PATH
                           "${CMAKE_CURRENT_SOURCE_DIR}/../resources/ui_icons/rabbi.ico"
                           ABSOLUTE)

    generate_product_version(
       VersionFilesOutputVariableLauncher
       NAME "${PROJECT_NAME}"
       ICON "${ICON_PATH}"
       VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR}
       VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR}
       VERSION_PATCH  ${${PROJECT_NAME}_VERSION_PATCH}
       COMPANY_COPYRIGHT "https://github.com/man-k28/CryptoShloma"
    )
    add_executable(${PROJECT_NAME} WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${VersionFilesOutputVariableLauncher})
    target_link_options(${TARGET_NAME} PRIVATE -mwindows)
else()
    add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_include_directories(${PROJECT_NAME} PRIVATE ../src ../include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    CuteLogger
)
