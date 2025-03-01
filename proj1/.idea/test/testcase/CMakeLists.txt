include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#file(GLOB SRCS *.cpp)
add_definitions(-Wno-old-style-cast)
add_definitions(-Wreturn-type)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

IF (${CMAKE_CXX_COMPILER_ID} MATCHES ".*Clang")
    message( STATUS "" )
    message( STATUS "cmake for Clang " )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wreturn-type -D_GLIBCXX_USE_NANOSLEEP")
    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ -pthread")
    ELSE()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    ENDIF()
    IF (${CMAKE_SYSTEM} MATCHES "FreeBSD-([0-9]*)\\.(.*)")
        IF (${CMAKE_MATCH_1} GREATER 9)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
            set(PLATFORM_LINK_LIBRIES execinfo)
        ENDIF()
    ELSEIF (APPLE)
        set(PLATFORM_LINK_LIBRIES c++abi)
    ELSEIF (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
        set(PLATFORM_LINK_LIBRIES rt c++abi)
    ELSE()
        set(PLATFORM_LINK_LIBRIES rt)
    ENDIF()



ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message( STATUS "cmake for GCC " )
    IF (APPLE)
        set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -std=c++14 -Wreturn-type -pthread -D_GLIBCXX_USE_NANOSLEEP")
    ELSEIF (MINGW)
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -Wreturn-type -pthread -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD")
        set(PLATFORM_LINK_LIBRIES dbghelp)

        # deal with ERROR level conflicts with windows.h
        ADD_DEFINITIONS (-DNOGDI)
    ELSE()
        set(PLATFORM_LINK_LIBRIES rt)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wreturn-type -std=c++14 -pthread -lrt -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD")
    ENDIF()
ELSEIF(MSVC)
    set(PLATFORM_LINK_LIBRIES dbghelp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8") # source code already in utf-8, force it for compilers in non-utf8_windows_locale
    # ERROR level conflicts with windows.h
    ADD_DEFINITIONS (-DNOGDI)
    # support AMD proc on vc2015
    if(${CMAKE_CL_64} STREQUAL "0")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:IA32")
    endif()
endif()
#add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
#add_link_options(-fprofile-instr-generate)
set(testcase-target testcase)
aux_source_directory(.               testcase-src)

aux_source_directory(../../bptree/src testcase-src)
add_executable(${testcase-target} ${testcase-src})

target_include_directories(${testcase-target} PUBLIC .)
target_include_directories(${testcase-target} PUBLIC ../../bptree/include)

target_include_directories(${testcase-target} PUBLIC /usr/local/include)
IF(APPLE)
    target_link_directories(${testcase-target} PUBLIC /usr/local/lib)
ENDIF()
target_compile_definitions(testcase PRIVATE GOOGLE_TEST)
TARGET_LINK_LIBRARIES(testcase
        gtest_main
)

include(GoogleTest)
set(TestCaseArgs "")
#gtest_add_tests(testcase "${TestCaseArgs}" AUTO)

gtest_discover_tests(testcase)
