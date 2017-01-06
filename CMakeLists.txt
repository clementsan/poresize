project(poresize)

cmake_minimum_required(VERSION 3.2.3)  
cmake_policy(VERSION 3.2.3)
 
find_package(ITK REQUIRED)
if (ITK_FOUND)
 include(${ITK_USE_FILE})
endif(ITK_FOUND)

add_executable(porosity porosity.cxx)
target_link_libraries(porosity ${ITK_LIBRARIES})
install( TARGETS porosity RUNTIME DESTINATION bin )

add_executable(coveringradius coveringradius.cxx)
target_link_libraries(coveringradius ${ITK_LIBRARIES})
install( TARGETS coveringradius RUNTIME DESTINATION bin )

add_executable(histogram histogram.cxx)
target_link_libraries(histogram ${ITK_LIBRARIES})
install( TARGETS histogram RUNTIME DESTINATION bin )
