// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from flir_camera_msgs:msg/CameraControl.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "flir_camera_msgs/msg/camera_control.hpp"


#ifndef FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__TRAITS_HPP_
#define FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "flir_camera_msgs/msg/detail/camera_control__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"

namespace flir_camera_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const CameraControl & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: exposure_time
  {
    out << "exposure_time: ";
    rosidl_generator_traits::value_to_yaml(msg.exposure_time, out);
    out << ", ";
  }

  // member: gain
  {
    out << "gain: ";
    rosidl_generator_traits::value_to_yaml(msg.gain, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const CameraControl & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: exposure_time
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "exposure_time: ";
    rosidl_generator_traits::value_to_yaml(msg.exposure_time, out);
    out << "\n";
  }

  // member: gain
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "gain: ";
    rosidl_generator_traits::value_to_yaml(msg.gain, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const CameraControl & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace flir_camera_msgs

namespace rosidl_generator_traits
{

[[deprecated("use flir_camera_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const flir_camera_msgs::msg::CameraControl & msg,
  std::ostream & out, size_t indentation = 0)
{
  flir_camera_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use flir_camera_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const flir_camera_msgs::msg::CameraControl & msg)
{
  return flir_camera_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<flir_camera_msgs::msg::CameraControl>()
{
  return "flir_camera_msgs::msg::CameraControl";
}

template<>
inline const char * name<flir_camera_msgs::msg::CameraControl>()
{
  return "flir_camera_msgs/msg/CameraControl";
}

template<>
struct has_fixed_size<flir_camera_msgs::msg::CameraControl>
  : std::integral_constant<bool, has_fixed_size<std_msgs::msg::Header>::value> {};

template<>
struct has_bounded_size<flir_camera_msgs::msg::CameraControl>
  : std::integral_constant<bool, has_bounded_size<std_msgs::msg::Header>::value> {};

template<>
struct is_message<flir_camera_msgs::msg::CameraControl>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__TRAITS_HPP_
