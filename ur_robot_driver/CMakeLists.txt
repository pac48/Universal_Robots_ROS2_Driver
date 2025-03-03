cmake_minimum_required(VERSION 3.5)
project(ur_robot_driver)

#add_compile_options(-std=c++11)
#add_compile_options(-Wall)
#add_compile_options(-Wextra)
#add_compile_options(-Wno-unused-parameter)
#
#if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
#  message("${PROJECT_NAME}: You did not request a specific build type: selecting 'RelWithDebInfo'.")
#  set(CMAKE_BUILD_TYPE RelWithDebInfo)
#endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()


find_package(ament_cmake REQUIRED)
find_package(controller_manager REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(ur_client_library REQUIRED)
find_package(ur_dashboard_msgs REQUIRED)
find_package(rclpy REQUIRED)
find_package(controller_interface REQUIRED)


include_directories(include)

set(THIS_PACKAGE_INCLUDE_DEPENDS
  controller_manager
  hardware_interface
  pluginlib
  rclcpp
  std_srvs
  ur_client_library
  ur_dashboard_msgs
        tf2_geometry_msgs
        controller_interface
        rclcpp_lifecycle
)

add_library(ur_robot_driver_plugin
  SHARED
  src/dashboard_client_ros.cpp
  src/hardware_interface.cpp
  src/urcl_log_handler.cpp
)
target_link_libraries(
  ur_robot_driver_plugin
  ur_client_library::urcl
)
target_include_directories(
  ur_robot_driver_plugin
  PRIVATE
  include
)
ament_target_dependencies(
  ur_robot_driver_plugin
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)
pluginlib_export_plugin_description_file(hardware_interface hardware_interface_plugin.xml)

add_executable(dashboard_client
  src/dashboard_client_ros.cpp
  src/dashboard_client_node.cpp
  src/urcl_log_handler.cpp
)

target_link_libraries(dashboard_client ${catkin_LIBRARIES} ur_client_library::urcl)
ament_target_dependencies(dashboard_client ${${PROJECT_NAME}_EXPORTED_TARGETS} ${THIS_PACKAGE_INCLUDE_DEPENDS})

add_executable(ur_ros2_control_node src/ur_ros2_control_node.cpp)
target_include_directories(ur_ros2_control_node PUBLIC ${controller_manager_INCLUDE_DIRS})
target_link_libraries(ur_ros2_control_node ${controller_manager_LIBRARIES})
ament_target_dependencies(ur_ros2_control_node
  controller_interface
  hardware_interface
  rclcpp
        rclcpp_lifecycle
)

install(
  TARGETS dashboard_client ur_ros2_control_node
  DESTINATION lib/${PROJECT_NAME}
)

# INSTALL
install(
  TARGETS ur_robot_driver_plugin
  DESTINATION lib
)
install(
  DIRECTORY include/
  DESTINATION include
)

## EXPORTS
ament_export_include_directories(
  include
)
ament_export_libraries(
  ur_robot_driver_plugin
)

install(DIRECTORY resources
  DESTINATION share/${PROJECT_NAME}
  REGEX "/ursim/" EXCLUDE
)


install(DIRECTORY include/
  DESTINATION include
)

ament_export_dependencies(
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)

# Install Python execs
ament_python_install_package(${PROJECT_NAME})

# Install Python executables
install(PROGRAMS
  scripts/tool_communication.py
  DESTINATION lib/${PROJECT_NAME}
)

install(PROGRAMS scripts/wait_dashboard_server.sh
  DESTINATION bin
)

set(BUILD_TESTING 0)
if(BUILD_TESTING)
  find_package(ur_controllers REQUIRED)
  find_package(ur_description REQUIRED)
  find_package(ur_bringup REQUIRED)
  find_package(ur_msgs REQUIRED)
  find_package(launch_testing_ament_cmake)
  add_launch_test(test/integration_test_1.py)
  add_launch_test(test/integration_test_2.py)
endif()

ament_package()


