 # Once done this will define
 #
 #  OPENGLES_FOUND        - system has OPENGLES v2
 #  OPENGLES_INCLUDE_DIRS - the include directory
 #  OPENGLES_LIBRARY_DIRS - the directory containing the libraries
 #  OPENGLES_LIBRARIES    - Link these to use OPENGLES

find_package(PkgConfig)
if( PKG_CONFIG_FOUND )
   pkg_check_modules( PKG_GLESV2 REQUIRED "glesv2" )

   set ( OPENGLES_INCLUDE_DIRS ${PKG_GLESV2_INCLUDE_DIRS} )
   foreach ( i ${PKG_GLESV2_LIBRARIES} )
     string ( REGEX MATCH "[^-]*" ibase "${i}" )
     find_library ( ${ibase}_LIBRARY
       NAMES ${i}
       PATHS ${PKG_GLESV2_LIBRARY_DIRS}
     )
     if ( ${ibase}_LIBRARY )
       list ( APPEND OPENGLES_LIBRARIES ${${ibase}_LIBRARY} )
     endif ( ${ibase}_LIBRARY )
     mark_as_advanced ( ${ibase}_LIBRARY )
   endforeach ( i )

elseif(ANDROID)
   FIND_PATH(OPENGLES_INCLUDE_DIRS
       NAMES GLES2/gl2.h
        "${ANDROID_STANDALONE_TOOLCHAIN}/usr/include"
   )
   
   FIND_LIBRARY(OPENGLES_LIBRARIES
       NAMES GLESv2
        PATHS
            "${ANDROID_STANDALONE_TOOLCHAIN}/usr/lib"
   )

else (PKG_CONFIG_FOUND)
   FIND_PATH(OPENGLES_INCLUDE_DIRS
       NAMES GLES2/gl2.h
   )
   
   FIND_LIBRARY(OPENGLES_LIBRARIES
       NAMES GLESv2
   )
   
endif()
 
IF(OPENGLES_LIBRARIES AND OPENGLES_INCLUDE_DIRS)
  SET(OPENGLES_FOUND "YES")
ELSE()
  SET(OPENGLES_FOUND "NO")
ENDIF()

MARK_AS_ADVANCED(
  OPENGLES_INCLUDE_DIRS
  OPENGLES_LIBRARY_DIRS
  OPENGLES_LIBRARIES
)
