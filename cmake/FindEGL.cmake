 # Once done this will define
 #
 #  EGL_FOUND        - system has EGL
 #  EGL_INCLUDE_DIRS - the include directory
 #  EGL_LIBRARY_DIRS - the directory containing the libraries
 #  EGL_LIBRARIES    - Link these to use EGL

find_package(PkgConfig)
if( PKG_CONFIG_FOUND )
   pkg_check_modules( PKG_EGL REQUIRED "egl" )

   set ( EGL_INCLUDE_DIRS ${PKG_EGL_INCLUDE_DIRS} )
   foreach ( i ${PKG_EGL_LIBRARIES} )
     string ( REGEX MATCH "[^-]*" ibase "${i}" )
     find_library ( ${ibase}_LIBRARY
       NAMES ${i}
       PATHS ${PKG_EGL_LIBRARY_DIRS}
     )
     if ( ${ibase}_LIBRARY )
       list ( APPEND EGL_LIBRARIES ${${ibase}_LIBRARY} )
     endif ( ${ibase}_LIBRARY )
     mark_as_advanced ( ${ibase}_LIBRARY )
   endforeach ( i )

elseif(ANDROID)
   FIND_PATH(EGL_INCLUDE_DIRS
       NAMES EGL/egl.h
        "${ANDROID_STANDALONE_TOOLCHAIN}/usr/include"
   )
   
   FIND_LIBRARY(EGL_LIBRARIES
       NAMES EGL
        PATHS
            "${ANDROID_STANDALONE_TOOLCHAIN}/usr/lib"
   )

else (PKG_CONFIG_FOUND)
   FIND_PATH(EGL_INCLUDE_DIRS
       NAMES EGL/egl.h
   )
   
   FIND_LIBRARY(EGL_LIBRARIES
       NAMES EGL
   )
   
endif()
 
IF(EGL_LIBRARIES AND EGL_INCLUDE_DIRS)
  SET(EGL_FOUND "YES")
ELSE()
  SET(EGL_FOUND "NO")
ENDIF()

MARK_AS_ADVANCED(
  EGL_INCLUDE_DIRS
  EGL_LIBRARY_DIRS
  EGL_LIBRARIES
)

