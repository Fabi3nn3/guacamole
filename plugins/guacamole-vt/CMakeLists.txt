###############################################################################
# additional dependencies
###############################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(LAMURE_VT_ROOT "/opt/lamure/install" CACHE PATH "Set to your lamure install path.")
include(find_lamure_virtual_texturing)

###############################################################################
# determine source files
###############################################################################
FILE(GLOB         GUA_VT_RENDERER_SRC       RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../../src/gua/renderer/*.cpp)
FILE(GLOB         GUA_VT_RENDERER_INC       RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../../include/gua/renderer/*.hpp)

FILE(GLOB_RECURSE GUA_VT_SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} resources/*)

SOURCE_GROUP("renderer" FILES ${GUA_VT_RENDERER_SRC})
SOURCE_GROUP("renderer" FILES ${GUA_VT_RENDERER_INC})

SOURCE_GROUP("resources" FILES ${GUA_VT_SHADERS})

SET(GUACAMOLE_VT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Path to guacamole-vt includes." )

add_library( guacamole-vt SHARED
    ${GUA_VT_RENDERER_SRC}
    ${GUA_VT_RENDERER_INC}
    ${GUA_VT_DETAILS_SRC}
    ${GUA_VT_DETAILS_INC}
    ${GUA_VT_SHADERS}
)

target_include_directories(guacamole-vt PUBLIC
                     ${GUACAMOLE_VT_INCLUDE_DIR}
                     ${LAMURE_VT_INCLUDE_DIRS}
)
#find_package(LOD REQUIRED)

###############################################################################
# compiling and linking
###############################################################################
IF (MSVC)
	set_target_properties(guacamole-vt PROPERTIES COMPILE_FLAGS "-D GUA_VT_LIBRARY")
ENDIF (MSVC)

IF (UNIX)
    LIST(APPEND LIBS udev Xinerama guacamole)
ELSEIF (MSVC)
    LIST(APPEND LIBS winmm.lib guacamole)
ENDIF(UNIX)

IF (MSVC)
  target_link_libraries( guacamole-vt optimized ${LAMURE_VT_LIBRARY}
                                        #optimized ${LAMURE_COMMON_LIBRARY}
                                        #debug ${LAMURE_RENDERING_LIBRARY_DEBUG}
                                        #debug ${LAMURE_COMMON_LIBRARY_DEBUG} 
                                        general guacamole
                       )
ELSEIF (UNIX)
  target_link_libraries( guacamole-vt 
                         guacamole 
                         ${LAMURE_VT_LIBRARY}
                         #${LAMURE_COMMON_LIBRARY}
                       )

ENDIF (MSVC)

###############################################################################
# install 
###############################################################################

IF (MSVC)
  install (TARGETS guacamole-vt
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin/Release
    LIBRARY DESTINATION lib/Release
    ARCHIVE DESTINATION lib/Release
  )

  install (TARGETS guacamole-vt
    CONFIGURATIONS Debug
    RUNTIME DESTINATION bin/Debug
    LIBRARY DESTINATION lib/Debug
    ARCHIVE DESTINATION lib/Debug
  )
ELSEIF (UNIX)
  install (TARGETS guacamole-vt 
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
ENDIF (MSVC)

# header files 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gua/ DESTINATION include/gua FILES_MATCHING PATTERN "*.hpp")

# shader and material resources
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION resources 
        FILES_MATCHING 
        PATTERN "*.frag" 
        PATTERN "*.vert" 
        PATTERN "*.glsl"
        PATTERN "*.geom"
        PATTERN "*.tctrl"
        PATTERN "*.teval"
        )
