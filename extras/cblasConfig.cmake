# Finds and configures cblas if it exists on the system. 
# Andre Anjos - 02.july.2010
set(cblas_LIBDIRS /usr/lib;/usr/lib/atlas CACHE INTERNAL "libdirs")
find_path(cblas_INCLUDE NAMES cblas.h)
find_path(lapack_INCLUDE NAMES clapack.h)

find_library(lapack_LIBRARY NAMES lapack lapack PATHS ${cblas_LIBDIRS})
find_library(cblas_LIBRARY NAMES cblas)
find_library(atlas_LIBRARY NAMES atlas)
set(cblas_LIBRARIES ${lapack_LIBRARY};${cblas_LIBRARY};${atlas_LIBRARY} CACHE
    INTERNAL "libraries")

if(cblas_INCLUDE AND lapack_INCLUDE AND cblas_LIBRARY AND lapack_LIBRARY)
  set(cblas_FOUND "YES" CACHE INTERNAL "package")
else(cblas_INCLUDE AND lapack_INCLUDE AND cblas_LIBRARY AND lapack_LIBRARY)
  set(cblas_FOUND "NO")
  if (NOT cblas_INCLUDE)
    message("--   cblas INCLUDE not found!")
  endif (NOT cblas_INCLUDE)
  if (NOT lapack_INCLUDE)
    message("--   lapack INCLUDE not found!")
  endif (NOT lapack_INCLUDE)
  if (NOT lapack_LIBRARY)
    message("--   lapack LIBRARY not found!")
  endif (NOT lapack_LIBRARY)
  if (NOT cblas_LIBRARY)
    message("--   cblas LIBRARY not found!")
  endif (NOT cblas_LIBRARY)
  if (NOT atlas_LIBRARY)
    message("--   atlas LIBRARY not found!")
  endif (NOT atlas_LIBRARY)
endif(cblas_INCLUDE AND lapack_INCLUDE AND cblas_LIBRARY AND lapack_LIBRARY)
