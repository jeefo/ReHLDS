cmake_minimum_required(VERSION 3.10)
project(hlds CXX)

include(CheckCXXCompilerFlag)

option(DEBUG "Build with debug information." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid -rdynamic -fPIC options
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(COMPILE_FLAGS "")
set(LINK_FLAGS "")

if (MSVC)
	set(COMPILE_FLAGS "${COMPILE_FLAGS} /W3 /MP /bigobj")
	if (DEBUG)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} /Zi /Od")
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} /O2")
	endif()
else()
	if (X64)
		set(M32_FLAG "")
	else()
		set(M32_FLAG "-m32")
	endif()
	set(COMPILE_FLAGS "${M32_FLAG} -fno-pic -fno-pie -U_FORTIFY_SOURCE -DFORTIFY_SOURCE=0")
	set(LINK_FLAGS "${M32_FLAG} -no-pie -Wl,--no-export-dynamic")
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall -fno-exceptions -fno-strict-aliasing -fno-strict-overflow")

	check_cxx_compiler_flag("-Wa,--execstack" HAS_EXECSTACK_ASM)
	check_cxx_compiler_flag("-Wl,-z,execstack" HAS_EXECSTACK_LINK)

	# Glibc >=2.41 fixes
	if(HAS_EXECSTACK_ASM)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wa,--execstack")
	endif()

	if(HAS_EXECSTACK_LINK)
		set(LINK_FLAGS "${LINK_FLAGS} -Wl,-z,execstack")
	endif()

	if (DEBUG)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g3 -O3 -ggdb")
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g0 -O3 -fno-stack-protector")
	endif()

	# Check Intel C++ compiler
	if ("$ENV{CXX}" MATCHES "icpc")
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -Qoption,cpp,--treat_func_as_string_literal_cpp")
		set(LINK_FLAGS "${LINK_FLAGS} -static-intel -no-intel-extensions")

		if (NOT DEBUG)
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -ipo")
			set(LINK_FLAGS "${COMPILE_FLAGS} -ipo")
		endif()
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} \
			-mtune=generic -msse3\
			-fpermissive -fno-sized-deallocation\
			-Wno-unused-result")

		# Check if not Clang compiler AND GCC >= 8.3
		if (NOT "$ENV{CXX}" MATCHES "clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-stringop-truncation -Wno-format-truncation")
		endif()
	endif()

	# GCC >= 8.3
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -fcf-protection=none")
	endif()
endif()

set(PROJECT_SRC_DIR
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/../"
)

set(PROJECT_PUBLIC_DIR
	"${PROJECT_SOURCE_DIR}/../engine"
	"${PROJECT_SOURCE_DIR}/../common"
	"${PROJECT_SOURCE_DIR}/../public"
	"${PROJECT_SOURCE_DIR}/../public/rehlds"
)

if (WIN32)
	set(DEDICATED_SRCS
		"src/conproc.cpp"
		"src/dedicated_exports.cpp"
		"src/precompiled.cpp"
		"src/public_amalgamation.cpp"
		"src/sys_ded.cpp"
		"src/sys_window.cpp"
		"src/vgui/vguihelpers.cpp"
	)
	set(COMMON_SRCS
		"../common/commandline.cpp"
		"../common/minidump.cpp"
		"../common/ObjectList.cpp"
		"../common/SteamAppStartUp.cpp"
		"../common/textconsole.cpp"
		"../common/TextConsoleWin32.cpp"
		"../engine/mem.cpp"
	)
else()
	set(DEDICATED_SRCS
		"src/dbg.cpp"
		"src/dedicated_exports.cpp"
		"src/public_amalgamation.cpp"
		"src/sys_ded.cpp"
		"src/sys_linux.cpp"
		"src/vgui/vguihelpers.cpp"
	)
	set(COMMON_SRCS
		"../common/textconsole.cpp"
		"../common/TextConsoleUnix.cpp"
		"../common/SteamAppStartUp.cpp"
		"../common/ObjectList.cpp"
		"../common/commandline.cpp"
		"../common/minidump.cpp"
		"../engine/mem.cpp"
	)
endif()

if (NOT TARGET appversion)
	if (WIN32)
		add_custom_target(appversion
			COMMAND cmd /c "${PROJECT_SOURCE_DIR}/../version/appversion.bat" "${PROJECT_SOURCE_DIR}/../version/" "${PROJECT_SOURCE_DIR}/../")
	else()
		add_custom_target(appversion DEPENDS COMMAND "${PROJECT_SOURCE_DIR}/../version/appversion.sh" "${PROJECT_SOURCE_DIR}/../..")
	endif()
endif()

if (WIN32)
	add_executable(hlds WIN32 ${DEDICATED_SRCS})
else()
	add_executable(hlds ${DEDICATED_SRCS})
endif()
add_dependencies(hlds appversion)

target_include_directories(hlds PRIVATE
	${PROJECT_SRC_DIR}
	${PROJECT_PUBLIC_DIR}
)

if (WIN32)
	target_compile_definitions(hlds PRIVATE
		WIN32
		LAUNCHER_FIXES
		DEDICATED
		_CONSOLE
		_CRT_SECURE_NO_WARNINGS
		USE_BREAKPAD_HANDLER
	)
else()
	target_compile_definitions(hlds PRIVATE
		LAUNCHER_FIXES
		_CONSOLE
		_LINUX
		LINUX
		_GLIBCXX_USE_CXX11_ABI=0
		_stricmp=strcasecmp
		_strnicmp=strncasecmp
		_strdup=strdup
		_vsnprintf=vsnprintf
	)
endif()

target_sources(hlds PRIVATE
	${COMMON_SRCS}
)

if (WIN32)
	target_link_libraries(hlds PRIVATE
		ws2_32
		winmm
		steam_api
	)
else()
	target_link_libraries(hlds PRIVATE
		dl
	)
endif()

if (WIN32)
	set_target_properties(hlds PROPERTIES
		OUTPUT_NAME hlds
		COMPILE_FLAGS "${COMPILE_FLAGS}"
	)
else()
	set_target_properties(hlds PROPERTIES
		OUTPUT_NAME hlds_linux
		PREFIX ""
		COMPILE_FLAGS "${COMPILE_FLAGS}"
		LINK_FLAGS "${LINK_FLAGS}"
		POSITION_INDEPENDENT_CODE OFF
	)
endif()
