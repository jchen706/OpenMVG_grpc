# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

project(project4)

include(GenerateProtos.cmake)

add_executable(
  mapreducelib #library name
  main.cc             #sources
  gps_distance.h )           #headers
target_link_libraries(mapreducelib p4protolib)
target_link_libraries(mapreducelib ${OPENMVG_LIBRARIES})
TARGET_LINK_LIBRARIES(mapreducelib
  openMVG_system
  openMVG_image
  openMVG_features
  openMVG_sfm
  openMVG_exif
)
target_include_directories(mapreducelib PUBLIC ${MAPREDUCE_INCLUDE_DIR})
add_dependencies(mapreducelib p4protolib)


add_executable(
  node #library name
  node.cc             #sources
  gps_distance.h )           #headers
target_link_libraries(node p4protolib)
target_link_libraries(node ${OPENMVG_LIBRARIES})
target_link_libraries(mapreducelib ${OPENMVG_LIBRARIES})
TARGET_LINK_LIBRARIES(node
  openMVG_system
  openMVG_image
  openMVG_features
  openMVG_sfm
  openMVG_exif
)
target_include_directories(node PUBLIC ${MAPREDUCE_INCLUDE_DIR})
#target_include_directories(node PUBLIC ${Boost_INCLUDE_DIR})
add_dependencies(node p4protolib)
target_link_libraries(node Boost::thread)
target_link_libraries(node ${Boost_LIBRARIES} )

add_executable(
  nodeone #library name
  nodeone.cc             #sources
  gps_distance.h )           #headers
target_link_libraries(nodeone p4protolib)
target_link_libraries(nodeone ${OPENMVG_LIBRARIES})
TARGET_LINK_LIBRARIES(nodeone
  openMVG_system
  openMVG_image
  openMVG_features
  openMVG_sfm
  openMVG_exif
)
target_include_directories(nodeone PUBLIC ${MAPREDUCE_INCLUDE_DIR})
#target_include_directories(nodeone PUBLIC ${Boost_INCLUDE_DIR})
add_dependencies(nodeone p4protolib)
target_link_libraries(nodeone Boost::thread)
target_link_libraries(nodeone ${Boost_LIBRARIES} )


add_executable(
  nodeclient #library name
  nodeclient.cc             #sources
  gps_distance.h )           #headers
target_link_libraries(nodeclient p4protolib)
target_link_libraries(nodeclient ${OPENMVG_LIBRARIES})
TARGET_LINK_LIBRARIES(nodeclient
  openMVG_system
  openMVG_image
  openMVG_features
  openMVG_sfm
  openMVG_exif
)
target_include_directories(nodeclient PUBLIC ${MAPREDUCE_INCLUDE_DIR})
#target_include_directories(nodeclient PUBLIC ${Boost_INCLUDE_DIR})
add_dependencies(nodeclient p4protolib)
target_link_libraries(nodeclient Boost::thread)
target_link_libraries(nodeclient ${Boost_LIBRARIES} )
