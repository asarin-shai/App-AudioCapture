cmake_minimum_required(VERSION 3.25)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum MacOS deployment version")

project(AudioCapture
    DESCRIPTION "Capture audio and stream it over LabStreamingLayer"
    HOMEPAGE_URL "https://github.com/labstreaminglayer/App-AudioCapture/"
    LANGUAGES CXX C
    VERSION 0.1)

# Needed for customized MacOSXBundleInfo.plist.in
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network DBus Multimedia)
qt_standard_project_setup()

# LSL
find_package(LSL REQUIRED
    HINTS ${LSL_INSTALL_ROOT}
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/"
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/install"
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/cmake-build-release/install"
	"${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/cmake-build-release-visual-studio/install"
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/out/build/x64-Release"
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/out/install/x64-Release"
    PATH_SUFFIXES share/LSL)
get_filename_component(LSL_PATH ${LSL_CONFIG} DIRECTORY)
include("${LSL_PATH}/LSLCMake.cmake")

find_package(Threads REQUIRED)

# Create the target
qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE MANUAL_FINALIZATION)
set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.labstreaminglayer.AudioCapture")
target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    reader.h
    reader.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt::Widgets
    Qt::Multimedia
    Qt::DBus
    Threads::Threads
    LSL::lsl
)

if(APPLE)
	# Code signing is not required for debugging, and our deployment script will handle signing for release builds.
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
	set_target_properties(${PROJECT_NAME} PROPERTIES
		XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
	)
endif(APPLE)

if(WIN32)
	# Copy the required dll's into the build folder --> useful for debugging from IDE
	# TODO: More to be done: "The application failed to start because no Qt platform plugin could be initialized."
	# create a list of files to copy
	set(THIRD_PARTY_DLLS
			LSL::lsl
			Qt::Core
			Qt::Gui
			Qt::Widgets
			Qt::DBus
			Qt::Multimedia
			Qt::Network
	)
	foreach(_lib ${THIRD_PARTY_DLLS})
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE:${_lib}>
				$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	endforeach()
endif(WIN32)

qt_finalize_target(${PROJECT_NAME})

install(
    TARGETS ${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(WIN32)
    install(FILES $<TARGET_FILE:LSL::lsl>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${PROJECT_NAME}
    )
endif()

if(APPLE)
	# Install LSL framework into bundle
	get_filename_component(LSL_RESOURCES_DIR ${LSL_PATH} DIRECTORY)
	get_filename_component(LSL_FRAMEWORK_DIR ${LSL_RESOURCES_DIR} DIRECTORY)
	install(DIRECTORY "${LSL_FRAMEWORK_DIR}"
			DESTINATION "${PROJECT_NAME}.app/Contents/Frameworks"
			COMPONENT ${PROJECT_NAME})

	# The following should already be part of the INSTALL_RPATH so we don't need to append it.
	#	set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INSTALL_RPATH "@executable_path/../Frameworks")

	# Additionally, the qt_generate_deploy_app_script() will remove /Library/Frameworks from the rpath,
	#  meaning that we don't have to worry about the LSL framework being found in the system paths.
	#  i.e., it's OK that the install name for lsl.framework is set to @rpath/lsl.framework/Versions/A/lsl.
	#  and not to @executable_path/../Frameworks/lsl.framework/Versions/A/lsl.
endif()

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script_path
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script_path})

if(APPLE)
    # Add a post-install step to codesign the app bundle, but only if the APPLE_CODE_SIGN_IDENTITY_APP is set.
    install(CODE "
        if(NOT \"$ENV{APPLE_CODE_SIGN_IDENTITY_APP}\" STREQUAL \"\")
            message(STATUS \"Codesigning app bundle with identity: $ENV{APPLE_CODE_SIGN_IDENTITY_APP}\")
            execute_process(
                COMMAND codesign -vvv --force --deep --sign \"$ENV{APPLE_CODE_SIGN_IDENTITY_APP}\" --options runtime \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\"
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL 0)
                message(FATAL_ERROR \"codesign failed for ${PROJECT_NAME}.app with exit code ${result}\")
            endif()
        endif()
    " COMPONENT ${PROJECT_NAME})
endif()

#installLSLApp(${PROJECT_NAME})

# TODO: Generate CPack config file for DMG
#LSLGenerateCPackConfig()
