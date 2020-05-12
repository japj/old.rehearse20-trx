vcpkg_from_github(
  OUT_SOURCE_PATH
  SOURCE_PATH
  REPO
  BelledonneCommunications/bctoolbox
  REF
  4.3.1
  SHA512
  89d4ead8f08fa6e20735e4cfb29666273e1cb47c23e8ed8c3992b53eae966188a57e68259a224b2857fdf8b1f829784c8ad661cac9fbee6f99b8e2493c067a72
  HEAD_REF
  master)

if (WIN32)
    set(ONLY_STATIC_CRT "ONLY_STATIC_CRT")
else()
    set(ONLY_STATIC_CRT)
endif()

vcpkg_check_linkage(
    ONLY_STATIC_LIBRARY
    ${ONLY_STATIC_CRT}
)

if (WIN32)
    # disable treat warnings as errors on windows for now
    set(FEATURE_OPTIONS -DENABLE_STRICT=OFF)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DENABLE_SHARED=OFF
        -DENABLE_POLARSSL=OFF
        -DENABLE_TESTS_COMPONENT=OFF
        -DENABLE_TESTS=OFF 
        ${FEATURE_OPTIONS}
    PREFER_NINJA)
vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/bctoolbox/cmake)
vcpkg_copy_pdbs()

file(INSTALL
     ${SOURCE_PATH}/COPYING
     DESTINATION
     ${CURRENT_PACKAGES_DIR}/share/bctoolbox
     RENAME copyright)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)