// Copyright 2019 FZI Forschungszentrum Informatik
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lovro Ivanov lovro.ivanov@gmail.com
 * \author  Andy Zelenak zelenak@picknik.ai
 * \author  Marvin Große Besselmann grosse@fzi.de
 * \date    2019-04-11
 *
 */
//----------------------------------------------------------------------
#ifndef UR_ROBOT_DRIVER__HARDWARE_INTERFACE_HPP_
#define UR_ROBOT_DRIVER__HARDWARE_INTERFACE_HPP_

// System
#include <memory>
#include <string>
#include <vector>
#include <limits>

// ros2_control hardware_interface
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/visibility_control.h"

// UR stuff
#include "ur_client_library/ur/ur_driver.h"
#include "ur_robot_driver/dashboard_client_ros.hpp"
#include "ur_dashboard_msgs/msg/robot_mode.hpp"

// ROS
#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"

namespace ur_robot_driver
{
enum class PausingState
{
  PAUSED,
  RUNNING,
  RAMPUP
};

enum StoppingInterface
{
  NONE,
  STOP_POSITION,
  STOP_VELOCITY
};

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

/*!
 * \brief The HardwareInterface class handles the interface between the ROS system and the main
 * driver. It contains the read and write methods of the main control loop and registers various ROS
 * topics and services.
 */
class URPositionHardwareInterface : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(URPositionHardwareInterface);

  CallbackReturn on_init(const hardware_interface::HardwareInfo& system_info) final;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() final;

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() final;

  CallbackReturn on_activate(const rclcpp_lifecycle::State& previous_state) final;
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State& previous_state) final;

  hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) final;
  hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) final;

  hardware_interface::return_type prepare_command_mode_switch(const std::vector<std::string>& start_interfaces,
                                                              const std::vector<std::string>& stop_interfaces) final;

  hardware_interface::return_type perform_command_mode_switch(const std::vector<std::string>& start_interfaces,
                                                              const std::vector<std::string>& stop_interfaces) final;

  /*!
   * \brief Callback to handle a change in the current state of the URCaps program running on the
   * robot. Executed only on the state change.
   *
   * \param program_running True when the URCap program is running on the robot.
   */
  void handleRobotProgramState(bool program_running);

  static constexpr double NO_NEW_CMD_ = std::numeric_limits<double>::quiet_NaN();

  void asyncThread();

protected:
  template <typename T>
  void readData(const std::unique_ptr<urcl::rtde_interface::DataPackage>& data_pkg, const std::string& var_name,
                T& data);
  template <typename T, size_t N>
  void readBitsetData(const std::unique_ptr<urcl::rtde_interface::DataPackage>& data_pkg, const std::string& var_name,
                      std::bitset<N>& data);

  void initAsyncIO();
  void checkAsyncIO();
  void updateNonDoubleValues();
  void extractToolPose();
  void transformForceTorque();

  urcl::vector6d_t urcl_position_commands_;
  urcl::vector6d_t urcl_position_commands_old_;
  urcl::vector6d_t urcl_velocity_commands_;
  urcl::vector6d_t urcl_joint_positions_;
  urcl::vector6d_t urcl_joint_velocities_;
  urcl::vector6d_t urcl_joint_efforts_;
  urcl::vector6d_t urcl_ft_sensor_measurements_;
  urcl::vector6d_t urcl_tcp_pose_;

  bool packet_read_;

  uint32_t runtime_state_;
  bool controllers_initialized_;

  std::bitset<18> actual_dig_out_bits_;
  std::bitset<18> actual_dig_in_bits_;
  std::array<double, 2> standard_analog_input_;
  std::array<double, 2> standard_analog_output_;
  std::bitset<4> analog_io_types_;
  uint32_t tool_mode_;
  std::bitset<2> tool_analog_input_types_;
  std::array<double, 2> tool_analog_input_;
  int32_t tool_output_voltage_;
  double tool_output_current_;
  double tool_temperature_;
  double speed_scaling_;
  double target_speed_fraction_;
  double speed_scaling_combined_;
  int32_t robot_mode_;
  int32_t safety_mode_;
  std::bitset<4> robot_status_bits_;
  std::bitset<11> safety_status_bits_;

  // transform stuff
  tf2::Vector3 tcp_force_;
  tf2::Vector3 tcp_torque_;
  geometry_msgs::msg::TransformStamped tcp_transform_;

  // asynchronous commands
  std::array<double, 18> standard_dig_out_bits_cmd_;
  std::array<double, 2> standard_analog_output_cmd_;
  double io_async_success_;
  double target_speed_fraction_cmd_;
  double scaling_async_success_;
  double resend_robot_program_cmd_;
  double resend_robot_program_async_success_;
  bool first_pass_;
  bool initialized_;
  double system_interface_initialized_;
  bool async_thread_shutdown_;

  // payload stuff
  urcl::vector3d_t payload_center_of_gravity_;
  double payload_mass_;
  double payload_async_success_;

  // copy of non double values
  std::array<double, 18> actual_dig_out_bits_copy_;
  std::array<double, 18> actual_dig_in_bits_copy_;
  std::array<double, 4> analog_io_types_copy_;
  double tool_mode_copy_;
  std::array<double, 2> tool_analog_input_types_copy_;
  double tool_output_voltage_copy_;
  double robot_mode_copy_;
  double safety_mode_copy_;
  std::array<double, 4> robot_status_bits_copy_;
  std::array<double, 11> safety_status_bits_copy_;

  bool robot_program_running_;
  bool non_blocking_read_;

  PausingState pausing_state_;
  double pausing_ramp_up_increment_;

  // resources switching aux vars
  std::vector<uint> stop_modes_;
  std::vector<std::string> start_modes_;
  bool position_controller_running_;
  bool velocity_controller_running_;

  std::unique_ptr<urcl::UrDriver> ur_driver_;
  std::shared_ptr<std::thread> async_thread_;
};
}  // namespace ur_robot_driver

#endif  // UR_ROBOT_DRIVER__HARDWARE_INTERFACE_HPP_
