# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(can_use_assembler TRUE)
enable_language(ASM)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

file(GLOB_RECURSE ffmpeg_src LIST_DIRECTORIES true "src/main/cpp/ffmpeg/*.c")
file(GLOB_RECURSE dracarys_srcs_jni LIST_DIRECTORIES true "src/main/cpp/dracarys/*.cpp")
file(GLOB_RECURSE dracarys_srcs_decoder LIST_DIRECTORIES true "src/main/cpp/dracarys/decoders/*.c")
file(GLOB_RECURSE libyuv_srcs LIST_DIRECTORIES true "src/main/cpp/libyuv/common/*.cc")
SET(dracarys_srcs ${dracarys_srcs_jni} ${dracarys_srcs_decoder})

if((${ANDROID_ABI} STREQUAL "armeabi") OR (${ANDROID_ABI} STREQUAL "armeabi-v7a"))
    list(APPEND dracarys_srcs src/main/cpp/dracarys/decoders/gif/memset.arm.S)
    list(APPEND libyuv_srcs src/main/cpp/libyuv/armv7/compare_neon.cc)
    list(APPEND libyuv_srcs src/main/cpp/libyuv/armv7/row_neon.cc)
endif ((${ANDROID_ABI} STREQUAL "armeabi") OR (${ANDROID_ABI} STREQUAL "armeabi-v7a"))

if((${ANDROID_ABI} STREQUAL "armeabi") OR (${ANDROID_ABI} STREQUAL "arm64-v8a"))
    list(APPEND libyuv_srcs src/main/cpp/libyuv/aarch64/compare_neon64.cc)
    list(APPEND libyuv_srcs src/main/cpp/libyuv/aarch64/row_neon64.cc)
endif ((${ANDROID_ABI} STREQUAL "armeabi") OR (${ANDROID_ABI} STREQUAL "arm64-v8a"))

add_library( # Sets the name of the library.
        yuv

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${libyuv_srcs})

add_library( # Sets the name of the library.
        ffmpeg

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${ffmpeg_src})


add_library( # Sets the name of the library.
        dracarys

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${dracarys_srcs})

add_library(
            avcodec
            SHARED
            IMPORTED)
set_target_properties(
            avcodec
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavcodec.so)

add_library(
            avdevice
            SHARED
            IMPORTED)
set_target_properties(
            avdevice
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavdevice.so)

add_library(
            avfilter
            SHARED
            IMPORTED)
set_target_properties(
            avfilter
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavfilter.so)

add_library(
            avformat
            SHARED
            IMPORTED)
set_target_properties(
            avformat
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavformat.so)

add_library(
            avresample
            SHARED
            IMPORTED)
set_target_properties(
            avresample
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavresample.so)

add_library(
            avutil
            SHARED
            IMPORTED)
set_target_properties(
            avutil
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libavutil.so)

add_library(
            postproc
            SHARED
            IMPORTED)
set_target_properties(
            postproc
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libpostproc.so)

add_library(
            swresample
            SHARED
            IMPORTED)
set_target_properties(
            swresample
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libswresample.so)

add_library(
            swscale
            SHARED
            IMPORTED)
set_target_properties(
            swscale
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libswscale.so)

add_library(
            x264-157
            SHARED
            IMPORTED)
set_target_properties(
            x264-157
            PROPERTIES IMPORTED_LOCATION
            ../../../../libs/${ANDROID_ABI}/libx264-157.so)

include_directories(src/main/cpp)
include_directories(src/main/cpp/dracarys)
include_directories(src/main/cpp/ffmpeg)
include_directories(src/main/cpp/include)
include_directories(src/main/cpp/libyuv)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(
        yuv
        ${log-lib}
)

target_link_libraries( # Specifies the target library.
                       ffmpeg
                       avformat
                       avcodec
                       avdevice
                       avfilter
                       avresample
                       avutil
                       postproc
                       swresample
                       swscale
                       x264-157
                       jnigraphics
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

target_link_libraries(
        dracarys
        ffmpeg
        yuv
        ${log-lib} android EGL GLESv3 z)