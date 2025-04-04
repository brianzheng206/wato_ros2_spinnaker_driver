include(CMakeFindDependencyMacro)

# find_dependency(SPINNAKER)
find_dependency(rclcpp)
find_dependency(rclcpp_components)
find_dependency(camera_info_manager)
find_dependency(image_transport)
find_dependency(flir_camera_msgs)
find_dependency(sensor_msgs)
find_dependency(std_msgs)

# Add the targets file
include("${CMAKE_CURRENT_LIST_DIR}/spinnaker_camera_driverTargets.cmake")
