# This cmake module sets the project version and partial version
# variables by analysing the git tag and commit history.

if (GIT_FOUND)

    if (VERSION_UPDATE_FROM_GIT)
        # Get branch 
        message (STATUS "${GIT_EXECUTABLE} symbolic-ref HEAD")
        
        execute_process(COMMAND ${GIT_EXECUTABLE} symbolic-ref HEAD 
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${PROJECT_NAME}_PARTIAL_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    
        message (STATUS "REPLACE "refs/heads/" "" ${PROJECT_NAME}_VERSION_BRANCH ${${PROJECT_NAME}_PARTIAL_BRANCH}")

        string(REPLACE "refs/heads/" "" ${PROJECT_NAME}_VERSION_BRANCH ${${PROJECT_NAME}_PARTIAL_BRANCH})
        unset(${PROJECT_NAME}_PARTIAL_BRANCH)


        # Get last tag from git
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_STRING
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        #How many commits since last tag
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-list master ${${PROJECT_NAME}_VERSION_STRING}^..HEAD --count
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_AHEAD
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Get current commit SHA from git
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE ${PROJECT_NAME}_VERSION_GIT_SHA
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Get partial versions into a list
        string(REGEX MATCHALL "-.*$|[0-9]+" ${PROJECT_NAME}_PARTIAL_VERSION_LIST
            ${${PROJECT_NAME}_VERSION_STRING})

        # The tweak part is optional, so check if the list contains it
        list(LENGTH ${PROJECT_NAME}_PARTIAL_VERSION_LIST
            ${PROJECT_NAME}_PARTIAL_VERSION_LIST_LEN)

        # Set the version numbers
        list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST
            0 ${PROJECT_NAME}_VERSION_MAJOR)
        list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST
            1 ${PROJECT_NAME}_VERSION_MINOR)
        if (${PROJECT_NAME}_PARTIAL_VERSION_LIST_LEN GREATER 2)
            list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST
                2 ${PROJECT_NAME}_VERSION_PATCH)
        else()
            set(${PROJECT_NAME}_VERSION_PATCH "0")
        endif()


        if (${PROJECT_NAME}_PARTIAL_VERSION_LIST_LEN GREATER 3)
            list(GET ${PROJECT_NAME}_PARTIAL_VERSION_LIST 3 ${PROJECT_NAME}_VERSION_TWEAK)
            string(SUBSTRING ${${PROJECT_NAME}_VERSION_TWEAK} 1 -1 ${PROJECT_NAME}_VERSION_TWEAK)
        endif()

        # Unset the list
        unset(${PROJECT_NAME}_PARTIAL_VERSION_LIST)

        # Set full project version string
        set(${PROJECT_NAME}_VERSION_STRING_FULL
            ${${PROJECT_NAME}_VERSION_BRANCH}-${${PROJECT_NAME}_VERSION_STRING}-${${PROJECT_NAME}_VERSION_AHEAD}.${${PROJECT_NAME}_VERSION_GIT_SHA})

    endif(VERSION_UPDATE_FROM_GIT)

else(GIT_FOUND)
    message( FATAL_ERROR "You need to install GIT")

endif(GIT_FOUND)


# Set project version (without the preceding 'v')
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH})
if (${PROJECT_NAME}_VERSION_TWEAK)
	set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_VERSION}-${${PROJECT_NAME}_VERSION_TWEAK})
endif()
