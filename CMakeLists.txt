cmake_minimum_required(VERSION 3.10)
project(rehlds CXX)

	if (WIN32)
	add_custom_target(appversion
		COMMAND cmd /c "${PROJECT_SOURCE_DIR}/rehlds/version/appversion.bat" "${PROJECT_SOURCE_DIR}/rehlds/version/" "${PROJECT_SOURCE_DIR}/rehlds/")
else()
	add_custom_target(appversion DEPENDS
		COMMAND "${PROJECT_SOURCE_DIR}/rehlds/version/appversion.sh" "${PROJECT_SOURCE_DIR}"
	)
endif()

add_subdirectory(rehlds)

if (NOT CMAKE_BUILD_TYPE MATCHES Unittests)
	add_subdirectory(rehlds/dedicated)
	add_subdirectory(rehlds/filesystem)
	add_subdirectory(rehlds/HLTV)
endif()
