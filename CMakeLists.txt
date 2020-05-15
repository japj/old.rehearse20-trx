cmake_minimum_required(VERSION 3.10)

# Integrate with vcpkg installed ports
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_CXX_STANDARD 17)

# on windows we need to use static (crt) linking
if (WIN32)
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
endif()

# workaround for cmake-js not setting MSVC multi-threaded statically-linked runtime library.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project (rehearse20-trx)

#find_package(portaudio CONFIG REQUIRED)
#find_package(ALSA CONFIG REQUIRED)
find_package(Opus CONFIG REQUIRED)
find_package(bctoolbox CONFIG REQUIRED)
find_package(ortp CONFIG REQUIRED)

# SET(PA_PRIVATE_INCLUDE_PATHS ${PA_PRIVATE_INCLUDE_PATHS} ${ALSA_INCLUDE_DIRS})
# SET(PA_LIBRARY_DEPENDENCIES ${PA_LIBRARY_DEPENDENCIES} ${ALSA_LIBRARIES})
# SET(PA_PKGCONFIG_LDFLAGS "${PA_PKGCONFIG_LDFLAGS} -lasound")

add_executable(rx rx.c device.c sched.c rx_rtplib.c rx_alsalib.c payload_type_opus.c)
target_link_libraries(rx PRIVATE asound)
target_link_libraries(rx PRIVATE ${OPUS_LIBRARIES})
target_link_libraries(rx PRIVATE ${PORTAUDIO_LIBRARIES})
target_link_libraries(rx PRIVATE ${ORTP_LIBRARIES})
target_link_libraries(rx PRIVATE ${BCTOOLBOX_LIBRARIES})

add_executable(tx tx.c device.c sched.c tx_rtplib.c tx_alsalib.c payload_type_opus.c)
target_link_libraries(tx PRIVATE asound)
target_link_libraries(tx PRIVATE ${OPUS_LIBRARIES})
target_link_libraries(tx PRIVATE ${PORTAUDIO_LIBRARIES})
target_link_libraries(tx PRIVATE ${ORTP_LIBRARIES})
target_link_libraries(tx PRIVATE ${BCTOOLBOX_LIBRARIES})

add_executable(trx trx.cpp device.c sched.c tx_rtplib.c tx_alsalib.c rx_rtplib.c rx_alsalib.c trx_rtplib.c payload_type_opus.c)
target_link_libraries(trx PRIVATE asound)
target_link_libraries(trx PRIVATE ${OPUS_LIBRARIES})
target_link_libraries(trx PRIVATE ${PORTAUDIO_LIBRARIES})
target_link_libraries(trx PRIVATE ${ORTP_LIBRARIES})
target_link_libraries(trx PRIVATE ${BCTOOLBOX_LIBRARIES})
