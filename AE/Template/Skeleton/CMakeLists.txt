cmake_minimum_required(VERSION 3.15)

project(Skeleton)

set(CMAKE_CXX_STANDARD 17)

# Set the path to the Adobe After Effects SDK
set(AE_SDK_PATH "../../")

# Set the path to Python include and library directories
set(PYTHON_INCLUDE_DIR "C:/Users/tjerf/AppData/Local/Programs/Python/Python311/include")
set(PYTHON_LIBRARY "C:/Users/tjerf/AppData/Local/Programs/Python/Python311/libs")
set(VCPKG_INCLUDE_DIR "C:/Users/tjerf/vcpkg/installed/x64-windows-static/include")

# Specify your source files
file(GLOB_RECURSE SOURCE_FILES 
  ${AE_SDK_PATH}/Util/*.cpp
  ./Skeleton_Strings.cpp
  ./Skeleton.cpp
)

set(AESDK_INCLUDE
	${AE_SDK_PATH}/Headers
	${AE_SDK_PATH}/Headers/SP
	${AE_SDK_PATH}/Util
	${AE_SDK_PATH}/Resources
)

include_directories(
  ${AESDK_INCLUDE}
  ${PYTHON_INCLUDE_DIR}  # Include Python headers
  ${VCPKG_INCLUDE_DIR} # Include VCPKG headers
  ./
)

# Link Python library
link_directories(${PYTHON_LIBRARY})

# Specify the plugin target
add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})

# Set runtime library options for MSVC
if(MSVC)
  # Create options to select runtime library
  option(USE_MT "Use Multi-threaded (/MT) runtime library" OFF)
  option(USE_MTD "Use Multi-threaded Debug (/MTd) runtime library" OFF)

  # Set the runtime library based on the selected option
  if(USE_MT)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
  elseif(USE_MTD)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
  else()
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()
endif()

# If we are generating an xcode project 
if(CMAKE_GENERATOR STREQUAL "Xcode")
	set( CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum MacOs Version" )
	set( CMAKE_OSX_ARCHITECTURES "arm64;x86_64" )

  target_link_libraries(${PROJECT_NAME} "-framework Cocoa")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )

    find_program( AFX_REZ rez /Developer/Tools )

    add_custom_command(
  		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SkeletonPiPL.rsrc
  		COMMAND ${AFX_REZ}
  		ARGS ${CMAKE_CURRENT_SOURCE_DIR}/SkeletonPiPL.r
  			-o ${CMAKE_CURRENT_BINARY_DIR}/SkeletonPiPl.rsrc
  			-useDF
			${PIPL_INCLUDES}
      	-i ../${AE_SDK_PATH}/Headers
      	-i ../${AE_SDK_PATH}/Headers/SP
      	-i ../${AE_SDK_PATH}/Util
      	-i ../${AE_SDK_PATH}/Resources
  			-D __MACH__
  	)

	set_source_files_properties(
		${CMAKE_CURRENT_BINARY_DIR}/SkeletonPiPL.rsrc
		PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources/
	)
    
  target_sources( ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/SkeletonPiPL.rsrc )

	set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )
	set_target_properties( ${PROJECT_NAME} PROPERTIES SUFFIX "" )
	string( TIMESTAMP BUILD_YEAR "%Y" )
	set_target_properties( ${PROJECT_NAME} PROPERTIES 
    BUNDLE True
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    BUNDLE_EXTENSION "plugin"
    MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.adobe.AfterEffects.${PROJECT_NAME}"
    MACOSX_BUNDLE_COPYRIGHT "MIT LICENSED ${BUILD_YEAR}."
  )

	# PkgInfo
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo
		COMMAND echo "eFKTFXTC" >> ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo
	)

	set_source_files_properties(
		${CMAKE_CURRENT_BINARY_DIR}/PkgInfo
		PROPERTIES
		MACOSX_PACKAGE_LOCATION ""
	)

	target_sources( ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/PkgInfo )
endif()

# For Visual Studio project generation
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
	)

	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".aex")
	get_filename_component(
		AFX_REZ
		../../Resources/PiPLtool.exe
		ABSOLUTE
		CACHE
	)

	# Adobe is disgusting so we have to compile a resource format from the 90's into our project
	add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
			COMMAND
			cl 
			   /I \"../${AE_SDK_PATH}/Headers\" 
			   /I \"../${AE_SDK_PATH}/Headers/SP\" 
			   /I \"../${AE_SDK_PATH}/Util\" 
			   /I \"../${AE_SDK_PATH}/Resources\" 
			   /EP \"${CMAKE_CURRENT_SOURCE_DIR}/SkeletonPiPL.r\" > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
	)

	add_custom_command(
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
		COMMAND ${AFX_REZ} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
	)

	add_custom_command(
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
		COMMAND cl /D "MSWindows" /EP ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
	)

	target_sources(
		${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
	)
endif()

# generate project with [cmake .. -G "Visual Studio 17 2022" -DUSE_MT=ON]

# build binary with [& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Skeleton.sln]