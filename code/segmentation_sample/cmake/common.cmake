# BUILD system for /grundmann2 source tree.
# Copyright 2009 Matthias Grundmann. All rights reserved.

# Common flags for all projects
if (UNIX)
endif (UNIX)

if (APPLE)
  set(CMAKE_HOST_SYSTEM_PROCESSOR "x86_64")
  set(CMAKE_CXX_FLAGS "-fasm-blocks")
endif (APPLE)

# Checks for each cpp file if header exists and adds it to HEADERS
function(headers_from_sources_cpp HEADERS SOURCES)
set(HEADERS)
  foreach(SOURCE_FILE ${SOURCES})
    string(REPLACE ".cpp" ".h" HEADER_FILE ${SOURCE_FILE})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_FILE}")
           list(APPEND HEADERS ${HEADER_FILE})
    endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_FILE}")
  endforeach(SOURCE_FILE)

set(HEADERS "${HEADERS}" PARENT_SCOPE)

endfunction(headers_from_sources_cpp)

# Reads the depend.cmake file in the current source folder and processes 
# dependent libraries and packages in the source tree.
function(apply_dependencies TARGET)

  # Use property APPLY_DEPENDENCIES_CALLED to use add_subdirectory 
  # only on the main CMakeLists.txt
  # This is set by the first call of apply_dependencies.
  get_property(CALL_PROPERTY_DEFINED
               GLOBAL
               PROPERTY APPLY_DEPENDENCIES_CALLED
               SET)
  
  if (${CALL_PROPERTY_DEFINED})
    # Only add include and dependency
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")
     set(DEPENDENT_PACKAGES)
     include("${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")
     if(DEPENDENT_PACKAGES)
        foreach(PACKAGE "${DEPENDENT_PACKAGES}")
         # Add package directory to include path.
         include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../${PACKAGE})
         # Add package binary path to include path. 
         # Temporary files, like protobuffer or qt's moc are placed here.
         include_directories("${CMAKE_BINARY_DIR}/${PACKAGE}")
         add_dependencies(${TARGET} ${PACKAGE})
        endforeach(PACKAGE)
      endif(DEPENDENT_PACKAGES)

      include_directories(${DEPENDENT_INCLUDES})

   endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")

  else (${CALL_PROPERTY_DEFINED})
    # First call of apply_dependencies.
    set_property(GLOBAL PROPERTY APPLY_DEPENDENCIES_CALLED TRUE)

    # Get list of dependencies.
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")
      set(DEPENDENT_PACKAGES)
      include("${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")

      # Each package is unique but can have duplicate dependent libraries.
      set(MY_DEPENDENT_LIBRARIES "${DEPENDENT_LIBRARIES}")
      set(MY_DEPENDENT_LINK_DIRECTORIES "${DEPENDENT_LINK_DIRECTORIES}")
      set(MY_DEPENDENT_INCLUDES "${DEPENDENT_INCLUDES}" )

      if(DEPENDENT_PACKAGES)
        # Recursively get all dependent packages. Depth first.
        set(PACKAGES_TO_PROCESS "${DEPENDENT_PACKAGES}")
        recursive_dependency_retrieve(PACKAGES_TO_PROCESS "${PACKAGES_TO_PROCESS}")
        # Remove duplicates, keep first one. Does not alter the depth first property.
        list(REMOVE_DUPLICATES PACKAGES_TO_PROCESS)
        message("Adding the following dependent packages: ${PACKAGES_TO_PROCESS}")

        # The dependent packages binary dirs and created libraries.
        # We need to reverse the order before adding them.
        set(PACKAGE_LINK_DIRS)
        set(PACKAGE_LINK_LIBS)

        foreach(PACKAGE ${PACKAGES_TO_PROCESS})
          message("Adding dependency  ../${PACKAGE} ${PACKAGE}")
          add_subdirectory(../${PACKAGE} ${PACKAGE})
          include_directories(../${PACKAGE})
          # For temporary generated files like moc or protoc.
          include_directories("${CMAKE_BINARY_DIR}/${PACKAGE}")
          add_dependencies(${TARGET} ${PACKAGE})

          # Add linker information
          set(DEPENDENT_PACKAGES)
          set(DEPENDENT_INCLUDES)
          set(DEPENDENT_LIBRARIES)
          set(DEPENDENT_LINK_DIRECTORIES)
          set(CREATED_PACKAGES)
          
          include("${CMAKE_CURRENT_SOURCE_DIR}/../${PACKAGE}/depend.cmake")

          if (CREATED_PACKAGES)
            list(APPEND PACKAGE_LINK_DIRS ${PACKAGE})
            list(APPEND PACKAGE_LINK_LIBS ${CREATED_PACKAGES})
          endif(CREATED_PACKAGES)

          if (DEPENDENT_LIBRARIES)
            list(APPEND MY_DEPENDENT_LIBRARIES "${DEPENDENT_LIBRARIES}")
            list(APPEND MY_DEPENDENT_LINK_DIRECTORIES "${DEPENDENT_LINK_DIRECTORIES}")
            list(APPEND MY_DEPENDENT_INCLUDES "${DEPENDENT_INCLUDES}")
          endif(DEPENDENT_LIBRARIES)
        endforeach(PACKAGE)

        # Add package libraries and binary directories.
        if (PACKAGE_LINK_DIRS)
          # Reverse to adhere linking order.
          list(REVERSE PACKAGE_LINK_DIRS)
          list(REVERSE PACKAGE_LINK_LIBS)
          
          foreach(LINK_DIR ${PACKAGE_LINK_DIRS})
            link_directories(${LINK_DIR})
          endforeach(LINK_DIR)

          foreach(LINK_LIB ${PACKAGE_LINK_LIBS})
            target_link_libraries(${TARGET} ${LINK_LIB})
          endforeach(LINK_LIB)

        endif(PACKAGE_LINK_DIRS)
      endif(DEPENDENT_PACKAGES)
     
      if (MY_DEPENDENT_LINK_DIRECTORIES)
        list(REMOVE_DUPLICATES MY_DEPENDENT_LINK_DIRECTORIES)
        link_directories(${MY_DEPENDENT_LINK_DIRECTORIES})
      endif(MY_DEPENDENT_LINK_DIRECTORIES)


      if (MY_DEPENDENT_LIBRARIES)
        list(REMOVE_DUPLICATES MY_DEPENDENT_LIBRARIES)    
        target_link_libraries(${TARGET} ${MY_DEPENDENT_LIBRARIES})
      endif(MY_DEPENDENT_LIBRARIES)

      if (MY_DEPENDENT_INCLUDES)
        list(REMOVE_DUPLICATES MY_DEPENDENT_INCLUDES)
        include_directories(${MY_DEPENDENT_INCLUDES})
      endif(MY_DEPENDENT_INCLUDES)
  
    endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depend.cmake")

  endif(${CALL_PROPERTY_DEFINED})
endfunction(apply_dependencies)

# Finds recursively dependencies of packages in the source tree.
# Implementation function called by apply_dependencies.
function(recursive_dependency_retrieve ADD_LIST PACKAGES)
  set(LOCAL_LIST ${PACKAGES})
  foreach(item ${PACKAGES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${item}/depend.cmake")
      SET(DEPENDENT_PACKAGES)
      include ( "${CMAKE_CURRENT_SOURCE_DIR}/../${item}/depend.cmake")
      if(DEPENDENT_PACKAGES)
        SET(MY_PACKAGES "${DEPENDENT_PACKAGES}")
        SET(MY_LIST)
        recursive_dependency_retrieve(MY_LIST "${MY_PACKAGES}")
        # Build order is important. This is like depth first search. 
        list(INSERT LOCAL_LIST 0 "${MY_LIST}")
      endif(DEPENDENT_PACKAGES)
    endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${item}/depend.cmake")
  endforeach(item)
  set(${ADD_LIST} "${LOCAL_LIST}" PARENT_SCOPE)
endfunction(recursive_dependency_retrieve)
