find_package(OpenCV REQUIRED)

set(DEPENDENT_INCLUDES ${OpenCV_INCLUDE_DIRS}
                      )

set(DEPENDENT_LIBRARIES ${OpenCV_LIBRARIES}
                       )
set(DEPENDENT_LINK_DIRECTORIES ${OpenCV_LINK_DIRECTORIES} 
                              )

set(DEPENDENT_PACKAGES assert_log segment_util)
