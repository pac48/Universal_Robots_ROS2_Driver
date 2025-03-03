// Copyright 2019, FZI Forschungszentrum Informatik
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
 * \date    2020-11-9
 *
 */
//----------------------------------------------------------------------
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ur_client_library/exceptions.h"
#include "ur_client_library/ur/tool_communication.h"

#include "rclcpp/rclcpp.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "ur_robot_driver/hardware_interface.hpp"
#include "ur_robot_driver/urcl_log_handler.hpp"

namespace rtde = urcl::rtde_interface;

namespace ur_robot_driver
{

CallbackReturn URPositionHardwareInterface::on_init(const hardware_interface::HardwareInfo& system_info)
{
  if (hardware_interface::SystemInterface::on_init(system_info) != CallbackReturn::SUCCESS) {
    return CallbackReturn::ERROR;
  }
    info_ = system_info;

    // initialize
//  urcl_joint_positions_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,    0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_joint_velocities_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,    0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_joint_efforts_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_ft_sensor_measurements_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_ft_sensor_commands_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_tcp_pose_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_position_commands_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_position_commands_old_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  urcl_velocity_commands_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };


  for (const hardware_interface::ComponentInfo& joint : info_.joints) {
    if (joint.name == "gpio" || joint.name == "speed_scaling" || joint.name == "resend_robot_program" ||
        joint.name == "system_interface") {
      continue;
    }
    if (joint.command_interfaces.size() != 2) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' has %zu command interfaces found. 2 expected.", joint.name.c_str(),
                   joint.command_interfaces.size());
      return CallbackReturn::ERROR;
    }

    if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' have %s command interfaces found as first command interface. '%s' expected.",
                   joint.name.c_str(), joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
      return CallbackReturn::ERROR;
    }

    if (joint.command_interfaces[1].name != hardware_interface::HW_IF_VELOCITY) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' have %s command interfaces found as second command interface. '%s' expected.",
                   joint.name.c_str(), joint.command_interfaces[1].name.c_str(), hardware_interface::HW_IF_VELOCITY);
      return CallbackReturn::ERROR;
    }

    if (joint.state_interfaces.size() != 3) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"), "Joint '%s' has %zu state interface. 3 expected.",
                   joint.name.c_str(), joint.state_interfaces.size());
      return CallbackReturn::ERROR;
    }

    if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' have %s state interface as first state interface. '%s' expected.", joint.name.c_str(),
                   joint.state_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
      return CallbackReturn::ERROR;
    }

    if (joint.state_interfaces[1].name != hardware_interface::HW_IF_VELOCITY) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' have %s state interface as second state interface. '%s' expected.", joint.name.c_str(),
                   joint.state_interfaces[1].name.c_str(), hardware_interface::HW_IF_POSITION);
      return CallbackReturn::ERROR;
    }

    if (joint.state_interfaces[2].name != hardware_interface::HW_IF_EFFORT) {
      RCLCPP_FATAL(rclcpp::get_logger("URPositionHardwareInterface"),
                   "Joint '%s' have %s state interface as third state interface. '%s' expected.", joint.name.c_str(),
                   joint.state_interfaces[2].name.c_str(), hardware_interface::HW_IF_POSITION);
      return CallbackReturn::ERROR;
    }
  }

  return CallbackReturn::SUCCESS;
  ;
}

std::vector<hardware_interface::StateInterface> URPositionHardwareInterface::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
//    raise(SIGTRAP);
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    if (info_.joints[i].name == "gpio" || info_.joints[i].name == "speed_scaling" ||
        info_.joints[i].name == "resend_robot_program" || info_.joints[i].name == "system_interface") {
      continue;
    }
    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &urcl_joint_positions_[i]));

    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &urcl_joint_velocities_[i]));

    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_EFFORT, &urcl_joint_efforts_[i]));
  }
//
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "force.x", &urcl_ft_sensor_measurements_[0]));
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "force.y",  &urcl_ft_sensor_measurements_[1]));
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "force.z", &urcl_ft_sensor_measurements_[2]));
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "torque.x", &urcl_ft_sensor_measurements_[3]));
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "torque.y", &urcl_ft_sensor_measurements_[4]));
//    state_interfaces.emplace_back(hardware_interface::StateInterface("tcp_fts_sensor",
//            "torque.z", &urcl_ft_sensor_measurements_[5]));

  state_interfaces.emplace_back(
      hardware_interface::StateInterface("speed_scaling", "speed_scaling_factor", &speed_scaling_combined_));

  for (auto& sensor : info_.sensors) {
    for (uint j = 0; j < sensor.state_interfaces.size(); ++j) {
      state_interfaces.emplace_back(hardware_interface::StateInterface(sensor.name, sensor.state_interfaces[j].name,
                                                                       &urcl_ft_sensor_measurements_[j]));
    }
  }

  for (size_t i = 0; i < 18; ++i) {
    state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "digital_output_" + std::to_string(i),
                                                                     &actual_dig_out_bits_copy_[i]));
    state_interfaces.emplace_back(
        hardware_interface::StateInterface("gpio", "digital_input_" + std::to_string(i), &actual_dig_in_bits_copy_[i]));
  }

  for (size_t i = 0; i < 11; ++i) {
    state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "safety_status_bit_" + std::to_string(i),
                                                                     &safety_status_bits_copy_[i]));
  }

  for (size_t i = 0; i < 4; ++i) {
    state_interfaces.emplace_back(
        hardware_interface::StateInterface("gpio", "analog_io_type_" + std::to_string(i), &analog_io_types_copy_[i]));
    state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "robot_status_bit_" + std::to_string(i),
                                                                     &robot_status_bits_copy_[i]));
  }

  for (size_t i = 0; i < 2; ++i) {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
        "gpio", "tool_analog_input_type_" + std::to_string(i), &tool_analog_input_types_copy_[i]));

    state_interfaces.emplace_back(
        hardware_interface::StateInterface("gpio", "tool_analog_input_" + std::to_string(i), &tool_analog_input_[i]));

    state_interfaces.emplace_back(hardware_interface::StateInterface(
        "gpio", "standard_analog_input_" + std::to_string(i), &standard_analog_input_[i]));

    state_interfaces.emplace_back(hardware_interface::StateInterface(
        "gpio", "standard_analog_output_" + std::to_string(i), &standard_analog_output_[i]));
  }

  state_interfaces.emplace_back(
      hardware_interface::StateInterface("gpio", "tool_output_voltage", &tool_output_voltage_copy_));

  state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "robot_mode", &robot_mode_copy_));

  state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "safety_mode", &safety_mode_copy_));

  state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "tool_mode", &tool_mode_copy_));

  state_interfaces.emplace_back(
      hardware_interface::StateInterface("gpio", "tool_output_current", &tool_output_current_));

  state_interfaces.emplace_back(hardware_interface::StateInterface("gpio", "tool_temperature", &tool_temperature_));

  state_interfaces.emplace_back(
      hardware_interface::StateInterface("system_interface", "initialized", &system_interface_initialized_));

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> URPositionHardwareInterface::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (size_t i = 0; i < info_.joints.size(); ++i) {
    if (info_.joints[i].name == "gpio" || info_.joints[i].name == "speed_scaling" ||
        info_.joints[i].name == "resend_robot_program" || info_.joints[i].name == "system_interface") {
      continue;
    }
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &urcl_position_commands_[i]));

    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &urcl_velocity_commands_[i]));
  }

    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "force.x", &urcl_ft_sensor_commands_[0]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "force.y",  &urcl_ft_sensor_commands_[1]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "force.z", &urcl_ft_sensor_commands_[2]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "torque.x", &urcl_ft_sensor_commands_[3]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "torque.y", &urcl_ft_sensor_commands_[4]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface("tcp_fts_sensor",
                                                                     "torque.z", &urcl_ft_sensor_commands_[5]));

  command_interfaces.emplace_back(hardware_interface::CommandInterface("gpio", "io_async_success", &io_async_success_));

  command_interfaces.emplace_back(
      hardware_interface::CommandInterface("speed_scaling", "target_speed_fraction_cmd", &target_speed_fraction_cmd_));

  command_interfaces.emplace_back(hardware_interface::CommandInterface(
      "speed_scaling", "target_speed_fraction_async_success", &scaling_async_success_));

  command_interfaces.emplace_back(hardware_interface::CommandInterface(
      "resend_robot_program", "resend_robot_program_cmd", &resend_robot_program_cmd_));

  command_interfaces.emplace_back(hardware_interface::CommandInterface(
      "resend_robot_program", "resend_robot_program_async_success", &resend_robot_program_async_success_));

  command_interfaces.emplace_back(hardware_interface::CommandInterface("payload", "mass", &payload_mass_));
  command_interfaces.emplace_back(
      hardware_interface::CommandInterface("payload", "cog.x", &payload_center_of_gravity_[0]));
  command_interfaces.emplace_back(
      hardware_interface::CommandInterface("payload", "cog.y", &payload_center_of_gravity_[1]));
  command_interfaces.emplace_back(
      hardware_interface::CommandInterface("payload", "cog.z", &payload_center_of_gravity_[2]));
  command_interfaces.emplace_back(
      hardware_interface::CommandInterface("payload", "payload_async_success", &payload_async_success_));

  for (size_t i = 0; i < 18; ++i) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        "gpio", "standard_digital_output_cmd_" + std::to_string(i), &standard_dig_out_bits_cmd_[i]));
  }

  for (size_t i = 0; i < 2; ++i) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        "gpio", "standard_analog_output_cmd_" + std::to_string(i), &standard_analog_output_cmd_[i]));
  }

  return command_interfaces;
}

CallbackReturn URPositionHardwareInterface::on_activate(const rclcpp_lifecycle::State& previous_state)
{
  RCLCPP_INFO(rclcpp::get_logger("URPositionHardwareInterface"), "Starting ...please wait...");

    position_controller_running_ = false;
    velocity_controller_running_ = false;

    if (info_.hardware_parameters["control_mode"] ==  "velocity"){
        velocity_controller_running_ = true;
    } else{
        position_controller_running_ = true;
    }

  // The robot's IP address.
  std::string robot_ip = info_.hardware_parameters["robot_ip"];
  // Path to the urscript code that will be sent to the robot
  std::string script_filename = info_.hardware_parameters["script_filename"];
  // Path to the file containing the recipe used for requesting RTDE outputs.
  std::string output_recipe_filename = info_.hardware_parameters["output_recipe_filename"];
  // Path to the file containing the recipe used for requesting RTDE inputs.
  std::string input_recipe_filename = info_.hardware_parameters["input_recipe_filename"];
  // Start robot in headless mode. This does not require the 'External Control' URCap to be running
  // on the robot, but this will send the URScript to the robot directly. On e-Series robots this
  // requires the robot to run in 'remote-control' mode.
  bool headless_mode =
      (info_.hardware_parameters["headless_mode"] == "true") || (info_.hardware_parameters["headless_mode"] == "True");
  // Port that will be opened to communicate between the driver and the robot controller.
  int reverse_port = stoi(info_.hardware_parameters["reverse_port"]);
  // The driver will offer an interface to receive the program's URScript on this port.
  int script_sender_port = stoi(info_.hardware_parameters["script_sender_port"]);
  //  std::string tf_prefix = info_.hardware_parameters["tf_prefix"];
  //  std::string tf_prefix;

  // Enables non_blocking_read mode. Should only be used with combined_robot_hw. Disables error generated when read
  // returns without any data, sets the read timeout to zero, and synchronises read/write operations. Enabling this when
  // not used with combined_robot_hw can suppress important errors and affect real-time performance.
//  non_blocking_read_ = static_cast<bool>(stoi(info_.hardware_parameters["non_blocking_read"]));

  // Specify gain for servoing to position in joint space.
  // A higher gain can sharpen the trajectory.
  int servoj_gain = stoi(info_.hardware_parameters["servoj_gain"]);
  // Specify lookahead time for servoing to position in joint space.
  // A longer lookahead time can smooth the trajectory.
  double servoj_lookahead_time = stod(info_.hardware_parameters["servoj_lookahead_time"]);

  bool use_tool_communication = (info_.hardware_parameters["use_tool_communication"] == "true") ||
                                (info_.hardware_parameters["use_tool_communication"] == "True");

  // Hash of the calibration reported by the robot. This is used for validating the robot
  // description is using the correct calibration. If the robot's calibration doesn't match this
  // hash, an error will be printed. You can use the robot as usual, however Cartesian poses of the
  // endeffector might be inaccurate. See the "ur_calibration" package on help how to generate your
  // own hash matching your actual robot.
  std::string calibration_checksum = info_.hardware_parameters["kinematics/hash"];

  std::unique_ptr<urcl::ToolCommSetup> tool_comm_setup;
  if (use_tool_communication) {
    tool_comm_setup = std::make_unique<urcl::ToolCommSetup>();

    using ToolVoltageT = std::underlying_type<urcl::ToolVoltage>::type;
    ToolVoltageT tool_voltage;
    // Tool voltage that will be set as soon as the UR-Program on the robot is started. Note: This
    // parameter is only evaluated, when the parameter "use_tool_communication" is set to TRUE.
    // Then, this parameter is required.}
    tool_voltage = std::stoi(info_.hardware_parameters["tool_voltage"]);

    tool_comm_setup->setToolVoltage(static_cast<urcl::ToolVoltage>(tool_voltage));

    using ParityT = std::underlying_type<urcl::Parity>::type;
    ParityT parity;
    // Parity used for tool communication. Will be set as soon as the UR-Program on the robot is
    // started. Can be 0 (None), 1 (odd) and 2 (even).
    //
    // Note: This parameter is only evaluated, when the parameter "use_tool_communication"
    // is set to TRUE.  Then, this parameter is required.
    parity = std::stoi(info_.hardware_parameters["tool_parity"]);
    tool_comm_setup->setParity(static_cast<urcl::Parity>(parity));

    int baud_rate;
    // Baud rate used for tool communication. Will be set as soon as the UR-Program on the robot is
    // started. See UR documentation for valid baud rates.
    //
    // Note: This parameter is only evaluated, when the parameter "use_tool_communication"
    // is set to TRUE.  Then, this parameter is required.
    baud_rate = std::stoi(info_.hardware_parameters["tool_baud_rate"]);
    tool_comm_setup->setBaudRate(static_cast<uint32_t>(baud_rate));

    int stop_bits;
    // Number of stop bits used for tool communication. Will be set as soon as the UR-Program on the robot is
    // started. Can be 1 or 2.
    //
    // Note: This parameter is only evaluated, when the parameter "use_tool_communication"
    // is set to TRUE.  Then, this parameter is required.
    stop_bits = std::stoi(info_.hardware_parameters["tool_stop_bits"]);
    tool_comm_setup->setStopBits(static_cast<uint32_t>(stop_bits));

    int rx_idle_chars;
    // Number of idle chars for the RX unit used for tool communication. Will be set as soon as the UR-Program on the
    // robot is started. Valid values: min=1.0, max=40.0
    //
    // Note: This parameter is only evaluated, when the parameter "use_tool_communication"
    // is set to TRUE.  Then, this parameter is required.
    rx_idle_chars = std::stoi(info_.hardware_parameters["tool_rx_idle_chars"]);
    tool_comm_setup->setRxIdleChars(rx_idle_chars);

    int tx_idle_chars;
    // Number of idle chars for the TX unit used for tool communication. Will be set as soon as the UR-Program on the
    // robot is started. Valid values: min=0.0, max=40.0
    //
    // Note: This parameter is only evaluated, when the parameter "use_tool_communication"
    // is set to TRUE.  Then, this parameter is required.
    tx_idle_chars = std::stoi(info_.hardware_parameters["tool_tx_idle_chars"]);
    tool_comm_setup->setTxIdleChars(tx_idle_chars);
  }

  RCLCPP_INFO(rclcpp::get_logger("URPositionHardwareInterface"), "Initializing driver...");
//  registerUrclLogHandler();
//  try {
//    ur_driver_ = std::make_unique<urcl::UrDriver>(
//        robot_ip, script_filename, output_recipe_filename, input_recipe_filename,
//        std::bind(&URPositionHardwareInterface::handleRobotProgramState, this, std::placeholders::_1), headless_mode,
//        std::move(tool_comm_setup), calibration_checksum, (uint32_t)reverse_port, (uint32_t)script_sender_port,
//        servoj_gain, servoj_lookahead_time, non_blocking_read_);
//  } catch (urcl::ToolCommNotAvailable& e) {
//    RCLCPP_FATAL_STREAM(rclcpp::get_logger("URPositionHardwareInterface"), "See parameter use_tool_communication");
//
//    return CallbackReturn::ERROR;
//  } catch (urcl::UrException& e) {
//    RCLCPP_FATAL_STREAM(rclcpp::get_logger("URPositionHardwareInterface"), e.what());
//    return CallbackReturn::ERROR;
//  }

//  ur_driver_->startRTDECommunication();
//
//  async_thread_ = std::make_shared<std::thread>(&URPositionHardwareInterface::asyncThread, this);

    std::vector<std::string> ign_joint_topics_list;//={"/model/ur10/joint/%s/0/cmd_pos"};
    std::vector<std::string> joint_names;//={"/model/ur10/joint/%s/0/cmd_pos"};

    for (auto joint : info_.joints){
//        ign_joint_topics_list.push_back("/model/ur10/joint/"+joint.name+"/0/cmd_pos");
        ign_joint_topics_list.push_back("/model/ur10/joint/"+joint.name+"/cmd_vel");
        joint_names.push_back(joint.name);
    }

    // create joint subscriber
    ign_joint_subscriber = std::make_shared<universal_robot_ign::IGNJointSubscriber>(joint_names, "/world/default/model/ur10/joint_state");
    joint_subscriber_thread = std::thread([this]() {
        ign_joint_subscriber->start();
    });

    // create wrench subscriber
    ign_wrench_subscriber = std::make_shared<universal_robot_ign::IGNWrenchSubscriber>("/world/default/model/ur10/force_torque");
    wrench_subscriber_thread = std::thread([this]() {
        ign_wrench_subscriber->start();
    });

    // create publisher
    ign_joint_publisher = std::make_shared<universal_robot_ign::IGNJointPublisher>(joint_names, ign_joint_topics_list);

//    ign_wrench_publisher = std::make_shared<universal_robot_ign::IGNWrenchPublisher>("/model/ur10/joint/ft_joint/0/cmd_force");
//    ign_wrench_publisher = std::make_shared<universal_robot_ign::IGNWrenchPublisher>("/model/ur10/link/ft_link/cmd_wrench");
    ign_wrench_publisher = std::make_shared<universal_robot_ign::IGNWrenchPublisher>("/model/ur10/link/robotiq_arg2f_base_link/cmd_wrench");


  RCLCPP_INFO(rclcpp::get_logger("URPositionHardwareInterface"), "System successfully started!");

  return CallbackReturn::SUCCESS;
}

CallbackReturn URPositionHardwareInterface::on_deactivate(const rclcpp_lifecycle::State& previous_state)
{
  RCLCPP_INFO(rclcpp::get_logger("URPositionHardwareInterface"), "Stopping ...please wait...");

  joint_subscriber_thread.join();

  RCLCPP_INFO(rclcpp::get_logger("URPositionHardwareInterface"), "System successfully stopped!");

  return CallbackReturn::SUCCESS;
}

hardware_interface::return_type URPositionHardwareInterface::read(const rclcpp::Time & time, const rclcpp::Duration & period)
{
    auto joint_msg = ign_joint_subscriber->getJointStateMsg();

    for (int i=0; i < urcl_joint_positions_.size(); i++){
        urcl_joint_positions_[i] = joint_msg.position[i];
    }

    auto wrench_msg = ign_wrench_subscriber->getWrenchMsg();
    urcl_ft_sensor_measurements_[0] = wrench_msg.force.x;
    urcl_ft_sensor_measurements_[1] = wrench_msg.force.y;
    urcl_ft_sensor_measurements_[2] = wrench_msg.force.z;
    urcl_ft_sensor_measurements_[3] = wrench_msg.torque.x;
    urcl_ft_sensor_measurements_[4] = wrench_msg.torque.y;
    urcl_ft_sensor_measurements_[5] = wrench_msg.torque.z;

    return hardware_interface::return_type::OK;
}

hardware_interface::return_type URPositionHardwareInterface::write(const rclcpp::Time & time, const rclcpp::Duration & period)
{

    sensor_msgs::msg::JointState msg;// = ign_joint_subscriber->getJointStateMsg();
    sensor_msgs::msg::JointState msg_cur;// = ign_joint_subscriber->getJointStateMsg();

    geometry_msgs::msg::Wrench msg2;// = ign_wrench_subscriber->getWrenchMsg();

    //TODO add positon ocntorl mode

  if (position_controller_running_){
      msg_cur = ign_joint_subscriber->getJointStateMsg();
      msg.position.resize(urcl_position_commands_.size());
      for (int i=0; i < urcl_position_commands_.size(); i++){
          msg.position[i] = urcl_position_commands_[i];
          // if any are nan, send the current robot state
          if (std::isnan(msg.position[i]) ){
              msg = msg_cur;
              break;
          }
      }
      ign_joint_publisher->setJointPositionCb(std::make_shared<sensor_msgs::msg::JointState>(msg), std::make_shared<sensor_msgs::msg::JointState>(msg_cur));

  } else if(velocity_controller_running_){
      msg.velocity.resize(urcl_velocity_commands_.size());
      for (int i=0; i < urcl_velocity_commands_.size(); i++){
          msg.velocity[i] = urcl_velocity_commands_[i];
          if (std::isnan(msg.velocity[i]) ){
              msg.velocity[i] = 0;
          }
      }
      ign_joint_publisher->setJointVelocityCb(std::make_shared<sensor_msgs::msg::JointState>(msg));

  }


    // need to write wrench values if present
    msg2.force.x = urcl_ft_sensor_commands_[0];
    msg2.force.y = urcl_ft_sensor_commands_[1];
    msg2.force.z = urcl_ft_sensor_commands_[2];
    msg2.torque.x = urcl_ft_sensor_commands_[3];
    msg2.torque.y = urcl_ft_sensor_commands_[4];
    msg2.torque.z = urcl_ft_sensor_commands_[5];

    ign_wrench_publisher->setWrenchCb(std::make_shared<geometry_msgs::msg::Wrench>(msg2));

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type URPositionHardwareInterface::prepare_command_mode_switch(
    const std::vector<std::string>& start_interfaces, const std::vector<std::string>& stop_interfaces)
{
//  hardware_interface::return_type ret_val = hardware_interface::return_type::OK;
//
//  start_modes_.clear();
//  stop_modes_.clear();
//
//  // Starting interfaces
//  // add start interface per joint in tmp var for later check
//  for (const auto& key : start_interfaces) {
//    for (auto i = 0u; i < info_.joints.size(); i++) {
//      if (key == info_.joints[i].name + "/" + hardware_interface::HW_IF_POSITION) {
//        start_modes_.push_back(hardware_interface::HW_IF_POSITION);
//      }
//      if (key == info_.joints[i].name + "/" + hardware_interface::HW_IF_VELOCITY) {
//        start_modes_.push_back(hardware_interface::HW_IF_VELOCITY);
//      }
//    }
//  }
//  // set new mode to all interfaces at the same time
//  if (start_modes_.size() != 0 && start_modes_.size() != 6) {
//    ret_val = hardware_interface::return_type::ERROR;
//  }
//
//  // all start interfaces must be the same - can't mix position and velocity control
//  if (start_modes_.size() != 0 && !std::equal(start_modes_.begin() + 1, start_modes_.end(), start_modes_.begin())) {
//    ret_val = hardware_interface::return_type::ERROR;
//  }
//
//  // Stopping interfaces
//  // add stop interface per joint in tmp var for later check
//  for (const auto& key : stop_interfaces) {
//    for (auto i = 0u; i < info_.joints.size(); i++) {
//      if (key == info_.joints[i].name + "/" + hardware_interface::HW_IF_POSITION) {
//        stop_modes_.push_back(StoppingInterface::STOP_POSITION);
//      }
//      if (key == info_.joints[i].name + "/" + hardware_interface::HW_IF_VELOCITY) {
//        stop_modes_.push_back(StoppingInterface::STOP_VELOCITY);
//      }
//    }
//  }
//  // stop all interfaces at the same time
//  if (stop_modes_.size() != 0 &&
//      (stop_modes_.size() != 6 || !std::equal(stop_modes_.begin() + 1, stop_modes_.end(), stop_modes_.begin()))) {
//    ret_val = hardware_interface::return_type::ERROR;
//  }
//
//  controllers_initialized_ = true;
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type URPositionHardwareInterface::perform_command_mode_switch(
    const std::vector<std::string>& start_interfaces, const std::vector<std::string>& stop_interfaces)
{
//  hardware_interface::return_type ret_val = hardware_interface::return_type::OK;
//
//  if (stop_modes_.size() != 0 &&
//      std::find(stop_modes_.begin(), stop_modes_.end(), StoppingInterface::STOP_POSITION) != stop_modes_.end()) {
//    position_controller_running_ = false;
//    urcl_position_commands_ = urcl_position_commands_old_ = urcl_joint_positions_;
//  } else if (stop_modes_.size() != 0 &&
//             std::find(stop_modes_.begin(), stop_modes_.end(), StoppingInterface::STOP_VELOCITY) != stop_modes_.end()) {
//    velocity_controller_running_ = false;
//    urcl_velocity_commands_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//  }
//
//  if (start_modes_.size() != 0 &&
//      std::find(start_modes_.begin(), start_modes_.end(), hardware_interface::HW_IF_POSITION) != start_modes_.end()) {
//    velocity_controller_running_ = false;
//    urcl_position_commands_ = urcl_position_commands_old_ = urcl_joint_positions_;
//    position_controller_running_ = true;
//
//  } else if (start_modes_.size() != 0 && std::find(start_modes_.begin(), start_modes_.end(),
//                                                   hardware_interface::HW_IF_VELOCITY) != start_modes_.end()) {
//    position_controller_running_ = false;
//    urcl_velocity_commands_ = { { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } };
//    velocity_controller_running_ = true;
//  }
//
//  start_modes_.clear();
//  stop_modes_.clear();

  return hardware_interface::return_type::OK;
}

}  // namespace ur_robot_driver

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(ur_robot_driver::URPositionHardwareInterface, hardware_interface::SystemInterface)
