#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "zip::zip" for configuration "Release"
set_property(TARGET zip::zip APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(zip::zip PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/kubazip.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/kubazip.dll"
  )

list(APPEND _cmake_import_check_targets zip::zip )
list(APPEND _cmake_import_check_files_for_zip::zip "${_IMPORT_PREFIX}/lib/kubazip.lib" "${_IMPORT_PREFIX}/bin/kubazip.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
