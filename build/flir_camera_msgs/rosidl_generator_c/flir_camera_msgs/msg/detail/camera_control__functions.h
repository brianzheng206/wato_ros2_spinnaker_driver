// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from flir_camera_msgs:msg/CameraControl.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "flir_camera_msgs/msg/camera_control.h"


#ifndef FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__FUNCTIONS_H_
#define FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "rosidl_runtime_c/type_description/type_description__struct.h"
#include "rosidl_runtime_c/type_description/type_source__struct.h"
#include "rosidl_runtime_c/type_hash.h"
#include "rosidl_runtime_c/visibility_control.h"
#include "flir_camera_msgs/msg/rosidl_generator_c__visibility_control.h"

#include "flir_camera_msgs/msg/detail/camera_control__struct.h"

/// Initialize msg/CameraControl message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * flir_camera_msgs__msg__CameraControl
 * )) before or use
 * flir_camera_msgs__msg__CameraControl__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__init(flir_camera_msgs__msg__CameraControl * msg);

/// Finalize msg/CameraControl message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
void
flir_camera_msgs__msg__CameraControl__fini(flir_camera_msgs__msg__CameraControl * msg);

/// Create msg/CameraControl message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * flir_camera_msgs__msg__CameraControl__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
flir_camera_msgs__msg__CameraControl *
flir_camera_msgs__msg__CameraControl__create(void);

/// Destroy msg/CameraControl message.
/**
 * It calls
 * flir_camera_msgs__msg__CameraControl__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
void
flir_camera_msgs__msg__CameraControl__destroy(flir_camera_msgs__msg__CameraControl * msg);

/// Check for msg/CameraControl message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__are_equal(const flir_camera_msgs__msg__CameraControl * lhs, const flir_camera_msgs__msg__CameraControl * rhs);

/// Copy a msg/CameraControl message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__copy(
  const flir_camera_msgs__msg__CameraControl * input,
  flir_camera_msgs__msg__CameraControl * output);

/// Retrieve pointer to the hash of the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
const rosidl_type_hash_t *
flir_camera_msgs__msg__CameraControl__get_type_hash(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
const rosidl_runtime_c__type_description__TypeDescription *
flir_camera_msgs__msg__CameraControl__get_type_description(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the single raw source text that defined this type.
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
const rosidl_runtime_c__type_description__TypeSource *
flir_camera_msgs__msg__CameraControl__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support);

/// Retrieve pointer to the recursive raw sources that defined the description of this type.
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
const rosidl_runtime_c__type_description__TypeSource__Sequence *
flir_camera_msgs__msg__CameraControl__get_type_description_sources(
  const rosidl_message_type_support_t * type_support);

/// Initialize array of msg/CameraControl messages.
/**
 * It allocates the memory for the number of elements and calls
 * flir_camera_msgs__msg__CameraControl__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__Sequence__init(flir_camera_msgs__msg__CameraControl__Sequence * array, size_t size);

/// Finalize array of msg/CameraControl messages.
/**
 * It calls
 * flir_camera_msgs__msg__CameraControl__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
void
flir_camera_msgs__msg__CameraControl__Sequence__fini(flir_camera_msgs__msg__CameraControl__Sequence * array);

/// Create array of msg/CameraControl messages.
/**
 * It allocates the memory for the array and calls
 * flir_camera_msgs__msg__CameraControl__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
flir_camera_msgs__msg__CameraControl__Sequence *
flir_camera_msgs__msg__CameraControl__Sequence__create(size_t size);

/// Destroy array of msg/CameraControl messages.
/**
 * It calls
 * flir_camera_msgs__msg__CameraControl__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
void
flir_camera_msgs__msg__CameraControl__Sequence__destroy(flir_camera_msgs__msg__CameraControl__Sequence * array);

/// Check for msg/CameraControl message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__Sequence__are_equal(const flir_camera_msgs__msg__CameraControl__Sequence * lhs, const flir_camera_msgs__msg__CameraControl__Sequence * rhs);

/// Copy an array of msg/CameraControl messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_flir_camera_msgs
bool
flir_camera_msgs__msg__CameraControl__Sequence__copy(
  const flir_camera_msgs__msg__CameraControl__Sequence * input,
  flir_camera_msgs__msg__CameraControl__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // FLIR_CAMERA_MSGS__MSG__DETAIL__CAMERA_CONTROL__FUNCTIONS_H_
