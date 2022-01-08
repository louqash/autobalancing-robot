cmake_minimum_required(VERSION 3.16)

project(ML C CXX)

set(TENSORFLOW_DEPS "$ENV{tensorflow}/build/_deps/fft2d-build/libfft2d_fftsg.a"
                    "$ENV{tensorflow}/build/_deps/fft2d-build/libfft2d_fftsg2d.a"
                    "$ENV{tensorflow}/build/_deps/farmhash-build/libfarmhash.a"
                    "$ENV{tensorflow}/build/_deps/ruy-build/libruy.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/numeric/libabsl_int128.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/container/libabsl_raw_hash_set.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/container/libabsl_hashtablez_sampler.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/synchronization/libabsl_graphcycles_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/synchronization/libabsl_synchronization.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_base.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_malloc_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_log_severity.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_throw_delegate.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_exponential_biased.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_raw_logging_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/base/libabsl_spinlock_wait.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/time/libabsl_civil_time.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/time/libabsl_time.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/time/libabsl_time_zone.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/debugging/libabsl_stacktrace.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/debugging/libabsl_symbolize.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/debugging/libabsl_debugging_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/debugging/libabsl_demangle_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_program_name.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_config.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_commandlineflag_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_private_handle_accessor.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_marshalling.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_commandlineflag.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/flags/libabsl_flags_reflection.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/status/libabsl_status.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/strings/libabsl_str_format_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/strings/libabsl_strings.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/strings/libabsl_cord.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/strings/libabsl_strings_internal.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/hash/libabsl_city.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/hash/libabsl_wyhash.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/hash/libabsl_hash.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/types/libabsl_bad_variant_access.a"
                    "$ENV{tensorflow}/build/_deps/abseil-cpp-build/absl/types/libabsl_bad_optional_access.a"
                    "$ENV{tensorflow}/build/_deps/flatbuffers-build/libflatbuffers.a")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

execute_process(COMMAND "./build-tensorflow.sh" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(
    "$ENV{tensorflow}/build/flatbuffers/include"
    "$ENV{tensorflow}"
)

add_executable(ml main.cpp imu.cpp i2cDevice.cpp timer.cpp)
target_link_libraries(ml i2c rt dl pthread "$ENV{tensorflow}/build/libtensorflow-lite.a" ${TENSORFLOW_DEPS})
target_compile_options(ml PRIVATE -Wall -Wextra -Wpedantic -Werror)

install(TARGETS ml)
