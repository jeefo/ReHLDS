cmake_minimum_required(VERSION 3.10)
project(hltv CXX)

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
			-mtune=generic -msse3 -flto\
			-fpermissive -fno-sized-deallocation\
			-Wno-unused-result -Wno-unknown-pragmas -Wno-write-strings")

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
	"${PROJECT_SOURCE_DIR}/../.."
	"${PROJECT_SOURCE_DIR}/../../engine"
	"${PROJECT_SOURCE_DIR}/../../common"
	"${PROJECT_SOURCE_DIR}/../../public"
	"${PROJECT_SOURCE_DIR}/../../public/rehlds"
)

set(HLTV_SRCS
	"src/System.cpp"
	"src/public_amalgamation.cpp"
)

if (WIN32)
	list(APPEND HLTV_SRCS "src/precompiled.cpp")
	set(TEXT_CONSOLE_SRC "../../common/TextConsoleWin32.cpp")
else()
	set(TEXT_CONSOLE_SRC "../../common/TextConsoleUnix.cpp")
endif()

set(COMMON_SRCS
	"../../common/BaseSystemModule.cpp"
	"../../common/TokenLine.cpp"
	"../../common/ObjectList.cpp"
	"../../common/textconsole.cpp"
	"${TEXT_CONSOLE_SRC}"
	"../../common/minidump.cpp"
	"../../HLTV/common/random.cpp"
	"../../HLTV/common/common.cpp"
	"../../engine/mem.cpp"
)

if (NOT TARGET appversion)
	if (WIN32)
		add_custom_target(appversion
			COMMAND cmd /c "${PROJECT_SOURCE_DIR}/../../version/appversion.bat" "${PROJECT_SOURCE_DIR}/../../version/" "${PROJECT_SOURCE_DIR}/../../")
	else()
		add_custom_target(appversion DEPENDS COMMAND "${PROJECT_SOURCE_DIR}/../../version/appversion.sh" "${PROJECT_SOURCE_DIR}/../../..")
	endif()
endif()

if (WIN32)
	add_executable(hltv WIN32 ${appversion.sh} ${HLTV_SRCS})
else()
	add_executable(hltv ${appversion.sh} ${HLTV_SRCS})
endif()
add_dependencies(hltv appversion)

target_include_directories(hltv PRIVATE
	${PROJECT_SRC_DIR}
	${PROJECT_PUBLIC_DIR}
)

if (WIN32)
	target_compile_definitions(hltv PRIVATE
		HLTV
		HLTV_FIXES
		WIN32
		_CONSOLE
		_CRT_SECURE_NO_WARNINGS
	)
	target_sources(hltv PRIVATE ${COMMON_SRCS})
	target_link_libraries(hltv PRIVATE
		user32
	)
else()
	target_compile_definitions(hltv PRIVATE
		LAUNCHER_FIXES
		_CONSOLE
		_LINUX
		LINUX
		_GLIBCXX_USE_CXX11_ABI=0
		_stricmp=strcasecmp
		_strnicmp=strncasecmp
		_strdup=strdup
		_vsnprintf=vsnprintf
		_snprintf=snprintf
	)
	target_sources(hltv PRIVATE ${COMMON_SRCS})
	target_link_libraries(hltv PRIVATE
		dl
	)
endif()

set_target_properties(hltv PROPERTIES
	OUTPUT_NAME hltv
	PREFIX ""
	COMPILE_FLAGS "${COMPILE_FLAGS}"
	LINK_FLAGS "${LINK_FLAGS}"
	POSITION_INDEPENDENT_CODE OFF
)
