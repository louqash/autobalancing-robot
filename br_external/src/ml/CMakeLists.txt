cmake_minimum_required(VERSION 3.16)

project(ML C CXX)

set(TENSORFLOW_DEPS "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/fft2d-build/libfft2d_fftsg.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/fft2d-build/libfft2d_fftsg2d.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/farmhash-build/libfarmhash.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/ruy-build/libruy.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/numeric/libabsl_int128.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/container/libabsl_raw_hash_set.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/container/libabsl_hashtablez_sampler.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/synchronization/libabsl_graphcycles_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/synchronization/libabsl_synchronization.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_base.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_malloc_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_log_severity.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_throw_delegate.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_exponential_biased.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_raw_logging_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/base/libabsl_spinlock_wait.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/time/libabsl_civil_time.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/time/libabsl_time.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/time/libabsl_time_zone.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/debugging/libabsl_stacktrace.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/debugging/libabsl_symbolize.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/debugging/libabsl_debugging_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/debugging/libabsl_demangle_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_program_name.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_config.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_commandlineflag_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_private_handle_accessor.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_marshalling.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_commandlineflag.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_reflection.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/status/libabsl_status.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/strings/libabsl_str_format_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/strings/libabsl_strings.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/strings/libabsl_cord.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/strings/libabsl_strings_internal.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/hash/libabsl_city.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/hash/libabsl_wyhash.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/hash/libabsl_hash.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/types/libabsl_bad_variant_access.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/abseil-cpp-build/absl/types/libabsl_bad_optional_access.a"
                    "${CMAKE_SOURCE_DIR}/tensorflow_src/build/_deps/flatbuffers-build/libflatbuffers.a")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

execute_process(COMMAND "./build-tensorflow.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(ml
        ${CMAKE_SOURCE_DIR}/tensorflow_src/build/flatbuffers/include
        ${CMAKE_SOURCE_DIR}/tensorflow_src
        )

add_executable(ml main.cpp imu.cpp i2cDevice.cpp timer.cpp)
target_link_libraries(ml i2c rt dl pthread ${CMAKE_SOURCE_DIR}/tensorflow_src/build/libtensorflow-lite.a ${TENSORFLOW_DEPS})

install(TARGETS ml)
