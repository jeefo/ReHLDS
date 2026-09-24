cmake_minimum_required(VERSION 3.10)
project(bzip2 C)

if (NOT MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0 -O1 -fno-stack-protector -fno-strict-aliasing -fno-strict-overflow")
endif()

set(BZIP2_SRCS
	"src/blocksort.c"
	"src/bzlib.c"
	"src/compress.c"
	"src/crctable.c"
	"src/decompress.c"
	"src/huffman.c"
	"src/precompiled.c"
	"src/randtable.c"
	"src/bzlib_private.h"
)

include_directories(
	"include"
)

add_library(bzip2 STATIC ${BZIP2_SRCS})

if (NOT MSVC)
	if (X64)
		set_target_properties(bzip2 PROPERTIES
			POSITION_INDEPENDENT_CODE ON
		)
	elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_target_properties(bzip2 PROPERTIES
			COMPILE_FLAGS "-m32"
		)
	endif()
endif()
