cmake_minimum_required(VERSION 3.10)
project(core CXX)

if (X64)
	set(REHLDS_PIC ON)
else()
	set(REHLDS_PIC OFF)
endif()

option(DEBUG "Build with debug information." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid -fPIC option
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")

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
	set(COMPILE_FLAGS "${M32_FLAG} -U_FORTIFY_SOURCE -DFORTIFY_SOURCE=0")
	set(LINK_FLAGS "${M32_FLAG}")
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall -fno-exceptions -fno-strict-aliasing -fno-strict-overflow")

	if (DEBUG)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g3 -O3 -ggdb")
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g0 -O3 -fno-stack-protector")
	endif()

	if ("$ENV{CXX}" MATCHES "icpc")
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -Qoption,cpp,--treat_func_as_string_literal_cpp")
		set(LINK_FLAGS "${LINK_FLAGS} -static-intel -no-intel-extensions")
		if (NOT DEBUG)
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -ipo")
			set(LINK_FLAGS "${LINK_FLAGS} -ipo")
		endif()
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -mtune=generic -msse3 -flto\
			-fpermissive -fno-sized-deallocation\
			-Wno-unused-result -Wno-unknown-pragmas -Wno-unused-variable\
			-Wno-sign-compare -Wno-write-strings -Wno-strict-aliasing")

		if ("$ENV{CXX}" MATCHES "clang")
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-unused-private-field")
		else()
			if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
				set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-stringop-truncation -Wno-format-truncation")
			endif()
		endif()
	endif()

	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -fcf-protection=none")
	endif()
endif()

set(PROJECT_SRC_DIR
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/../"
)

set(PROJECT_BZIP2_DIR
	"${PROJECT_SOURCE_DIR}/../../../dep/bzip2/include"
)

set(PROJECT_PUBLIC_DIR
	"${PROJECT_SOURCE_DIR}/../.."
	"${PROJECT_SOURCE_DIR}/../../engine"
	"${PROJECT_SOURCE_DIR}/../../common"
	"${PROJECT_SOURCE_DIR}/../../pm_shared"
	"${PROJECT_SOURCE_DIR}/../../public"
	"${PROJECT_SOURCE_DIR}/../../public/rehlds"
)

set(CORE_SRCS
	"src/BSPModel.cpp"
	"src/Delta.cpp"
	"src/NetSocket.cpp"
	"src/Network.cpp"
	"src/Server.cpp"
	"src/World.cpp"
	"src/public_amalgamation.cpp"
)

set(COMMON_SRCS
	"../../common/BaseSystemModule.cpp"
	"../../common/ObjectDictionary.cpp"
	"../../common/ObjectList.cpp"
	"../../common/TokenLine.cpp"
	"../../HLTV/common/BitBuffer.cpp"
	"../../HLTV/common/byteorder.cpp"
	"../../HLTV/common/common.cpp"
	"../../HLTV/common/DemoFile.cpp"
	"../../HLTV/common/DirectorCmd.cpp"
	"../../HLTV/common/InfoString.cpp"
	"../../HLTV/common/mathlib.cpp"
	"../../HLTV/common/md5.cpp"
	"../../HLTV/common/munge.cpp"
	"../../HLTV/common/NetAddress.cpp"
	"../../HLTV/common/NetChannel.cpp"
	"../../HLTV/common/random.cpp"
	"../../engine/mem.cpp"
)

if (WIN32)
	list(APPEND CORE_SRCS "src/precompiled.cpp")
endif()

add_library(core SHARED ${CORE_SRCS})

target_include_directories(core PRIVATE
	${PROJECT_SRC_DIR}
	${PROJECT_BZIP2_DIR}
	${PROJECT_PUBLIC_DIR}
)

if (WIN32)
	target_compile_definitions(core PRIVATE
		HLTV
		HLTV_FIXES
		WIN32
		_WINDOWS
		_USRDLL
		CORE_MODULE
		_CRT_SECURE_NO_WARNINGS
	)
else()
	target_compile_definitions(core PRIVATE
		HLTV
		HLTV_FIXES
		_LINUX
		LINUX
		_GLIBCXX_USE_CXX11_ABI=0
		_stricmp=strcasecmp
		_strnicmp=strncasecmp
		_strdup=strdup
		_vsnprintf=vsnprintf
		_snprintf=snprintf
	)
endif()

target_sources(core PRIVATE
	${COMMON_SRCS}
)

if (WIN32)
	target_link_libraries(core PRIVATE
		psapi
		ws2_32
		bzip2
	)
else()
	target_link_libraries(core PRIVATE
		dl
		m
		bzip2
	)
	set(LINK_FLAGS "${LINK_FLAGS} -Wl,-z,notext")
endif()

if (NOT TARGET bzip2)
	add_subdirectory(../../../dep/bzip2 lib)
endif()

set_target_properties(core PROPERTIES
	OUTPUT_NAME core
	PREFIX ""
	COMPILE_FLAGS "${COMPILE_FLAGS}"
	LINK_FLAGS "${LINK_FLAGS}"
	POSITION_INDEPENDENT_CODE ${REHLDS_PIC}
)
