#[=======================================================================[

This module defines IMPORTED` target ``TBB::TBB``, ``TBB::MALLOC`` if TBB has been found.

---

This module will set the following variables in your project:

``TBB_FOUND``
  True if TBB is found.
``TBB_LIBRARIES``
  the TBB libraries needed for linking
``TBB_INCLUDE_DIRS``
  the directories of the TBB headers

]=======================================================================]

find_path(TBB_INCLUDE_DIR
  NAMES tbb.h
  HINTS ${TBB_DIR}/include
  PATH_SUFFIXES tbb
)

find_library(TBB_LIBRARY
  NAMES tbb
  HINTS ${TBB_DIR}/lib/intel64/gcc4.7/
  PATH_SUFFIXES tbb
)

find_library(TBB_MALLOC_LIBRARY
  NAMES tbbmalloc
  HINTS ${TBB_DIR}/lib/intel64/gcc4.7/
  PATH_SUFFIXES tbb
)

mark_as_advanced(TBB_FOUND)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TBB
  REQUIRED_VARS TBB_INCLUDE_DIR TBB_LIBRARY TBB_MALLOC_LIBRARY
  VERSION_VAR TBB_VERSION
)

if(TBB_FOUND)
  set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})
  set(TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY})

  if (NOT TARGET TBB::MALLOC)
    add_library(TBB::MALLOC SHARED IMPORTED)
    set_target_properties(TBB::MALLOC PROPERTIES
      IMPORTED_LOCATION "${TBB_MALLOC_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${TBB_INCLUDE_DIR}")
  endif ()

  if (NOT TARGET TBB::TBB)
    add_library(TBB::TBB SHARED IMPORTED)
    set_target_properties(TBB::TBB PROPERTIES
      IMPORTED_LOCATION "${TBB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${TBB_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES TBB::MALLOC)
  endif ()
endif()
