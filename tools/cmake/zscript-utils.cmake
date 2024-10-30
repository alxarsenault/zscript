
# add_sub_directories
MACRO(add_sub_directories directory)
  file(GLOB FOLDERS ${directory})

  foreach(folder ${FOLDERS})
    if(EXISTS ${folder}/CMakeLists.txt)
      add_subdirectory(${folder})
    endif()
  endforeach()
ENDMACRO()