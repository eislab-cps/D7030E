# List all .cc files in this folder
file(GLOB exec_sources CONFIGURE_DEPENDS *.cc)

# If none found, note it
if(NOT exec_sources)
  message(STATUS "scratch/exec: no .cc files found")
endif()

# Print each discovered source (filename + full path)
foreach(_s ${exec_sources})
  get_filename_component(_s_name ${_s} NAME)
  message(STATUS "scratch/exec: discovered source file: ${_s_name} (path: ${_s})")
endforeach()

list(LENGTH exec_sources _n_sources)
if(_n_sources EQUAL 1)
  list(GET exec_sources 0 single_src)
  set(scratch_src)

  get_filename_component(scratch_src ${single_src} ABSOLUTE)
  get_filename_component(scratch_name ${single_src} NAME_WLE)
  get_filename_component(scratch_src ${scratch_src} ABSOLUTE)
  get_filename_component(scratch_absolute_directory ${scratch_src} DIRECTORY)

  string(REPLACE "${PROJECT_SOURCE_DIR}" "${CMAKE_OUTPUT_DIRECTORY}"
    scratch_directory ${scratch_absolute_directory}
  )

  build_exec(
    EXECNAME exec
    EXECNAME_PREFIX ""
    SOURCE_FILES "${single_src}"
    LIBRARIES_TO_LINK "${ns3-libs}" "${ns3-contrib-libs}"
    EXECUTABLE_DIRECTORY_PATH ${scratch_directory}/
  )

  return()
endif()

message(STATUS "scratch/exec: more than one .cc file in the directory")
# Optionally list them again (useful for debugging)
foreach(_s ${exec_sources})
  get_filename_component(_s_name ${_s} NAME)
  message(STATUS "scratch/exec: - ${_s_name}")
endforeach()