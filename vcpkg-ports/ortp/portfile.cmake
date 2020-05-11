vcpkg_from_github(
  OUT_SOURCE_PATH
  SOURCE_PATH
  REPO
  BelledonneCommunications/ortp
  REF
  b4b0a6d40f79540a650f67679fb3198414eacfb5
  SHA512
  cf991952d231de98405c35390d03534950246e9a0254c18513e705de9fce4547faf4d9318040cf3a0b56c377cb8085e0fb5d1e1bf15c028c1516ca19a4ce83d4
  HEAD_REF
  master
  PATCHES
    fix-include.patch)

if (WIN32)
    set(ONLY_STATIC_CRT "ONLY_STATIC_CRT")
else()
    set(ONLY_STATIC_CRT)
endif()

vcpkg_check_linkage(
    ONLY_STATIC_LIBRARY
    ${ONLY_STATIC_CRT}
)


vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DENABLE_DOC=NO
        -DENABLE_PERF=YES
        ${FEATURE_OPTIONS}
    PREFER_NINJA)
vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/ortp)
vcpkg_copy_pdbs()

file(INSTALL
     ${SOURCE_PATH}/LICENSE.txt
     DESTINATION
     ${CURRENT_PACKAGES_DIR}/share/ortp
     RENAME copyright)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/lib/cmake
                    ${CURRENT_PACKAGES_DIR}/debug/share
                    ${CURRENT_PACKAGES_DIR}/debug/include)