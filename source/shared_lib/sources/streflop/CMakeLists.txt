#########################################################################################
# streflop
#if (NOT $ENV{CXX} MATCHES "icpc")
#	SET(cxxflags "${cxxflags} -mfpmath=sse -msse")
#endif (NOT $ENV{CXX} MATCHES "icpc")

FILE(GLOB_RECURSE STREFLOP_GLOBBED_CPP libm/flt-32/*.cpp)

INCLUDE_DIRECTORIES(
	../../include/streflop/libm/flt-32
	../../include/streflop/libm/headers
	)

SET(STREFLOP_SRC ${STREFLOP_GLOBBED_CPP})

IF(MSVC)
	ENABLE_LANGUAGE(ASM_MASM)
	SET(STREFLOP_SRC ${STREFLOP_SRC} FPUSettings.asm)
ENDIF()

ADD_LIBRARY(streflop STATIC EXCLUDE_FROM_ALL
	${STREFLOP_SRC}
)

IF(NOT MSVC)
	set_target_properties(streflop PROPERTIES COMPILE_FLAGS "${PIC_FLAG}")
ENDIF()
