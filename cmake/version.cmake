option( Use_Hg "Use Mercurial" OFF )

if ( Use_Hg )
    find_package( Hg REQUIRED QUIET )
    HG_WC_INFO( ${PROJECT_SOURCE_DIR} ${PROJECT_NAME} )
    execute_process( COMMAND ${HG_EXECUTABLE} branch
                                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                     OUTPUT_VARIABLE summary_output)
    string( STRIP ${summary_output} ${PROJECT_NAME}_BRANCH )
    set( ${PROJECT_NAME}_VERSION_PATCH ${${PROJECT_NAME}_WC_REVISION} )
    set( ${PROJECT_NAME}_REVISION ${${PROJECT_NAME}_WC_REVISION} )
else ()
    find_package( Git REQUIRED QUIET )

    if (Git_FOUND)
        execute_process( COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                         OUTPUT_VARIABLE branch_output)

        execute_process( COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
                                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                         OUTPUT_VARIABLE revision_output
                                         OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    string( STRIP ${branch_output} ${PROJECT_NAME}_BRANCH )
    set( ${PROJECT_NAME}_VERSION_PATCH ${revision_output} )
    set( ${PROJECT_NAME}_REVISION ${revision_output} )
endif ()



set( ${PROJECT_NAME}_VERSION_MAJOR 0 )
set( ${PROJECT_NAME}_VERSION_MINOR 3 )
set( ${PROJECT_NAME}_DB 0.6.2 )
set( ${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}" )

message( STATUS "version ${${PROJECT_NAME}_VERSION} ( branch '${${PROJECT_NAME}_BRANCH}' )" )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.in
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.generated @ONLY )

if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp )
        execute_process( COMMAND ${CMAKE_COMMAND} -E compare_files
                ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.generated
                ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp RESULT_VARIABLE result )
        if ( ${result} EQUAL 0 )
                file( REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.generated )
                return()
        endif()
        file( REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp )
endif()
file( RENAME ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.generated
        ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp )

