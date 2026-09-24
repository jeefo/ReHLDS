#----------------------------------------
# 1. Preparing build:
#    rm -rf build
#    mkdir build && cd build
#
# 2. Select compiler and build it
# - Compile with Clang:
#    CC="clang" CXX="clang++" cmake ..
#    make
#
# - Compile with Intel C++ Compiler:
#    CC="icc" CXX="icpc" cmake ..
#    make
#
# - Compile with GCC Compiler:
#    cmake ..
#    make
#
# 64-bit (Linux):
#    cmake -DX64=ON ..
# Windows (MSVC, x64):
#    cmake -G "Visual Studio 17 2022" -A x64 ..
#----------------------------------------

cmake_minimum_required(VERSION 3.10)
project(engine CXX)

option(DEBUG "Build with debug information." OFF)
option(USE_STATIC_LIBSTDC "Enables static linking libstdc++." OFF)
option(X64 "Build 64-bit target on Linux." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid -rdynamic -fPIC options
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(REHLDS_X64 OFF)
if (X64)
	set(REHLDS_X64 ON)
endif()
if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(REHLDS_X64 ON)
endif()

if (REHLDS_X64)
	set(REHLDS_PIC ON)
else()
	set(REHLDS_PIC OFF)
endif()

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
	if (NOT REHLDS_X64)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -m32")
		set(LINK_FLAGS "${LINK_FLAGS} -m32")
	endif()

	set(COMPILE_FLAGS "${COMPILE_FLAGS} -U_FORTIFY_SOURCE -DFORTIFY_SOURCE=0")
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall -fno-rtti -fno-exceptions -fno-strict-aliasing -fno-strict-overflow")

	# Remove noxref code and data
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -ffunction-sections -fdata-sections")

	if (DEBUG)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g3 -O3 -ggdb")
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -g0 -O3 -fno-stack-protector")
	endif()

	# Check Intel C++ compiler
	if ("$ENV{CXX}" MATCHES "icpc")
		set(COMPILE_FLAGS "${COMPILE_FLAGS} \
			-fp-model=precise\
			-fasm-blocks\
			-Qoption,cpp,--treat_func_as_string_literal_cpp")
		set(LINK_FLAGS "${LINK_FLAGS} -static-intel -no-intel-extensions")
		if (NOT DEBUG)
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -ipo")
			set(LINK_FLAGS "${LINK_FLAGS} -ipo")
		endif()
	else()
		set(COMPILE_FLAGS "${COMPILE_FLAGS} \
			-mtune=generic -msse3\
			-fpermissive -fno-sized-deallocation\
			-Wno-unknown-pragmas -Wno-invalid-offsetof\
			-Wno-unused-variable -Wno-unused-result -Wno-unused-function -Wno-delete-non-virtual-dtor\
			-Wno-write-strings -Wno-format\
			-Wno-sign-compare -Wno-strict-aliasing -Wno-ignored-attributes")

		# Check if not Clang compiler
		if (NOT "$ENV{CXX}" MATCHES "clang")
			set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-plt -Wno-unused-but-set-variable")

			# GCC >= 8.3
			if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
				set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-stringop-truncation -Wno-format-truncation -Wno-class-memaccess")
			endif()
		endif()
	endif()

	# GCC >= 8.3
	if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
		set(COMPILE_FLAGS "${COMPILE_FLAGS} -fcf-protection=none")
	endif()

	if (NOT DEBUG)
		set(LINK_FLAGS "${LINK_FLAGS} \
			-Wl,-gc-sections -Wl,--version-script=\"${PROJECT_SOURCE_DIR}/../version_script.lds\"")
	endif()
endif()

set(PROJECT_SRC_DIR
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/engine"
	"${PROJECT_SOURCE_DIR}/common"
	"${PROJECT_SOURCE_DIR}/pm_shared"
	"${PROJECT_SOURCE_DIR}/rehlds"
	"${PROJECT_SOURCE_DIR}/testsuite"
	"${PROJECT_SOURCE_DIR}/unittests"
)

set(PROJECT_BZIP2_DIR
	"${PROJECT_SOURCE_DIR}/../dep/bzip2/include"
)

set(PROJECT_CPPUNITLITE_DIR
	"${PROJECT_SOURCE_DIR}/../dep/cppunitlite/include"
)

set(PROJECT_PUBLIC_DIR
	"${PROJECT_SOURCE_DIR}/public"
	"${PROJECT_SOURCE_DIR}/public/rehlds"
)

set(ENGINE_SRCS
	engine/sv_main.cpp
	engine/sv_user.cpp
	engine/sv_phys.cpp
	engine/sv_move.cpp
	engine/sv_pmove.cpp
	engine/sv_log.cpp
	engine/sv_remoteaccess.cpp
	engine/sv_steam3.cpp
	engine/sv_upld.cpp
	engine/sys_dll.cpp
	engine/sys_dll2.cpp
	engine/sys_engine.cpp
	engine/sys_linuxwind.cpp
	engine/SystemWrapper.cpp
	engine/host.cpp
	engine/host_cmd.cpp
	engine/net_chan.cpp
	engine/net_ws.cpp
	engine/pmove.cpp
	engine/pmovetst.cpp
	engine/pr_cmds.cpp
	engine/pr_edict.cpp
	engine/wad.cpp
	engine/model.cpp
	engine/world.cpp
	engine/zone.cpp
	engine/cmd.cpp
	engine/cmodel.cpp
	engine/com_custom.cpp
	engine/common.cpp
	engine/crc.cpp
	engine/cvar.cpp
	engine/decals.cpp
	engine/delta.cpp
	engine/delta_jit.cpp
	engine/ed_strpool.cpp
	engine/filesystem.cpp
	engine/filesystem_internal.cpp
	engine/hashpak.cpp
	engine/info.cpp
	engine/ipratelimit.cpp
	engine/l_studio.cpp
	engine/textures.cpp
	engine/tmessage.cpp
	engine/traceinit.cpp
	engine/unicode_strtools.cpp
	engine/buildnum.cpp
	engine/mathlib.cpp
	engine/mathlib_sse.cpp
	engine/md5.cpp
	engine/mem.cpp
	engine/module.cpp
	engine/r_studio.cpp
	engine/vid_null.cpp
	engine/cl_null.cpp
	engine/snd_null.cpp
	engine/sse_mathfun.cpp
	engine/public_amalgamation.cpp
	rehlds/flight_recorder.cpp
	rehlds/FlightRecorderImpl.cpp
	rehlds/hookchains_impl.cpp
	rehlds/main.cpp
	rehlds/platform.cpp
	rehlds/public_amalgamation.cpp
	rehlds/rehlds_api_impl.cpp
	rehlds/rehlds_interfaces_impl.cpp
	rehlds/rehlds_messagemngr_impl.cpp
	rehlds/rehlds_security.cpp
)

if (REHLDS_X64)
	list(APPEND ENGINE_SRCS asmlib_stub/asmlib_stub.cpp)
endif()

if (WIN32)
	list(APPEND ENGINE_SRCS
		rehlds/RehldsRuntimeConfig.cpp
		rehlds/structSizeCheck.cpp
		testsuite/anonymizer.cpp
		testsuite/funccalls.cpp
		testsuite/memory.cpp
		testsuite/player.cpp
		testsuite/recorder.cpp
		testsuite/testsuite.cpp
	)
endif()

set(UNITTESTS_SRCS
	unittests/common_tests.cpp
	unittests/crc32c_tests.cpp
	unittests/delta_tests.cpp
	unittests/download_tests.cpp
	unittests/info_tests.cpp
	unittests/mathlib_tests.cpp
	unittests/rehlds_tests_shared.cpp
	unittests/rehlds_tests_shared.h
	unittests/security_tests.cpp
	unittests/struct_offsets_tests.cpp
	unittests/TestRunner.cpp
	unittests/tmessage_tests.cpp
	unittests/unicode_tests.cpp
)

set(COMMON_SRCS
	"common/BaseSystemModule.cpp"
	"common/ObjectList.cpp"
	"common/TokenLine.cpp"
)

set(PUBLIC_SRCS
	"public/tier0/dbg.cpp"
	"public/registry.cpp"
	"public/steamid.cpp"
	"public/utlbuffer.cpp"
)

if (CMAKE_BUILD_TYPE MATCHES Unittests)
	if (NOT TARGET cppunitlite)
		add_subdirectory(../dep/cppunitlite cppunitlite)
	endif()

	add_executable(engine ${ENGINE_SRCS})
else()
	add_library(engine SHARED ${ENGINE_SRCS})
endif()

if (WIN32)
	add_library(steam_api INTERFACE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		target_link_libraries(steam_api INTERFACE "${PROJECT_SOURCE_DIR}/lib/steam_api.lib")
	else()
		target_link_libraries(steam_api INTERFACE "${PROJECT_SOURCE_DIR}/lib/win64/steam_api64.lib")
	endif()
elseif (REHLDS_X64)
	add_library(steam_api INTERFACE)
	target_link_libraries(steam_api INTERFACE "${PROJECT_SOURCE_DIR}/lib/linux64/libsteam_api.so")
endif()

if (NOT TARGET bzip2)
	add_subdirectory(../dep/bzip2 bzip2)
endif()

if (NOT TARGET appversion)
	if (WIN32)
		add_custom_target(appversion
			COMMAND cmd /c "\"${PROJECT_SOURCE_DIR}/version/appversion.bat\" \"${PROJECT_SOURCE_DIR}/version/\" \"${PROJECT_SOURCE_DIR}/\"")
	else()
		add_custom_target(appversion DEPENDS COMMAND "${PROJECT_SOURCE_DIR}/version/appversion.sh" "${PROJECT_SOURCE_DIR}/..")
	endif()
endif()

add_dependencies(engine appversion)

target_include_directories(engine PRIVATE
	${PROJECT_SRC_DIR}
	${PROJECT_BZIP2_DIR}
	${PROJECT_CPPUNITLITE_DIR}
	${PROJECT_PUBLIC_DIR}
)

if (REHLDS_X64)
	set(REHLDS_JIT_DEFS "")
else()
	set(REHLDS_JIT_DEFS REHLDS_JIT)
endif()

if (WIN32)
	target_compile_definitions(engine PRIVATE
		WIN32
		${REHLDS_JIT_DEFS}
		REHLDS_SSE
		REHLDS_FIXES
		REHLDS_CHECKS
		REHLDS_API
		REHLDS_SELF
		REHLDS_OPT_PEDANTIC
		HAVE_OPT_STRTOOLS
		USE_BREAKPAD_HANDLER
		DEDICATED
		SWDS
		_CRT_SECURE_NO_WARNINGS

		$<$<CONFIG:Unittests>:
		REHLDS_UNIT_TESTS>
	)
else()
	target_compile_definitions(engine PRIVATE
		SWDS
		${REHLDS_JIT_DEFS}
		REHLDS_SSE
		REHLDS_FIXES
		REHLDS_CHECKS
		REHLDS_API
		REHLDS_SELF
		REHLDS_OPT_PEDANTIC
		HAVE_OPT_STRTOOLS
		USE_BREAKPAD_HANDLER
		_LINUX
		LINUX
		_GLIBCXX_USE_CXX11_ABI=0
		_stricmp=strcasecmp
		_strnicmp=strncasecmp
		_strdup=strdup
		_unlink=unlink
		_vsnprintf=vsnprintf
		_vsnwprintf=vswprintf

		$<$<CONFIG:Unittests>:
		REHLDS_UNIT_TESTS REHLDS_SSE REHLDS_JIT>
	)
endif()

target_sources(engine PRIVATE
	${COMMON_SRCS}
	${PUBLIC_SRCS}

	$<$<CONFIG:Unittests>:
	${UNITTESTS_SRCS}>
)

if (WIN32)
	target_link_libraries(engine PRIVATE
		psapi
		ws2_32
		bzip2
		steam_api
	)
else()
	set(LINUX_LIBS dl rt m bzip2 steam_api)
	if (NOT REHLDS_X64)
		list(APPEND LINUX_LIBS aelf32)
	endif()
	target_link_libraries(engine PRIVATE ${LINUX_LIBS})
endif()

if (USE_STATIC_LIBSTDC)
	target_compile_definitions(engine PRIVATE BUILD_STATIC_LIBSTDC)
	if (NOT MSVC)
		set(LINK_FLAGS "${LINK_FLAGS} -static-libgcc -static-libstdc++")
	endif()
endif()

if (WIN32)
	set_target_properties(engine PROPERTIES
		OUTPUT_NAME swds
		PREFIX ""
		COMPILE_FLAGS "${COMPILE_FLAGS}"
	)
else()
	if (REHLDS_X64)
		set(ENGINE_OUTPUT_NAME engine_amd64)
	else()
		set(ENGINE_OUTPUT_NAME engine_i486)
	endif()

	set(LINK_FLAGS "${LINK_FLAGS} \
		-Wl,-rpath,'$ORIGIN/.' \
		-L${PROJECT_SOURCE_DIR}/lib/linux32")

	if (REHLDS_X64)
		set(LINK_FLAGS "${LINK_FLAGS} -Wl,-z,notext")
	endif()

	set_target_properties(engine PROPERTIES
		OUTPUT_NAME ${ENGINE_OUTPUT_NAME}
		PREFIX ""
		COMPILE_FLAGS "${COMPILE_FLAGS}"
		LINK_FLAGS "${LINK_FLAGS}"
		POSITION_INDEPENDENT_CODE ${REHLDS_PIC}
	)
endif()
