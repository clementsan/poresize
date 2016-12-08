project(poresize)

cmake_minimum_required(VERSION 3.2.3)  
cmake_policy(VERSION 3.2.3)

## Match everything when analyzing dependencies
#INCLUDE_REGULAR_EXPRESSION("^.*$")

cmake_minimum_required(VERSION 2.8)
 
find_package(ITK REQUIRED)
if (ITK_FOUND)
 include(${ITK_USE_FILE})
endif(ITK_FOUND)


# Configure VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
message ("Vtk_use_file = " ${VTK_USE_FILE})
message(STATUS "VTK_LIBRARIES = " ${VTK_LIBRARIES})

add_executable(poresize poresize.cxx)
 
target_link_libraries(poresize
	${ITK_LIBRARIES}
	${VTK_LIBRARIES})

add_executable(coveringradius coveringradius.cxx)
 
target_link_libraries(coveringradius
	${ITK_LIBRARIES}
	${VTK_LIBRARIES})

add_executable(histogram histogram.cxx)
 
target_link_libraries(histogram
	${ITK_LIBRARIES}
	${VTK_LIBRARIES})
