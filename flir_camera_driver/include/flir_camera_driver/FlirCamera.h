/*
This code was developed by the National Robotics Engineering Center (NREC), part of the Robotics Institute at Carnegie Mellon University.
Its development was funded by DARPA under the LS3 program and submitted for public release on June 7th, 2012.
Release was granted on August, 21st 2012 with Distribution Statement "A" (Approved for Public Release, Distribution Unlimited).

This software is released under a BSD license:

Copyright (c) 2012, Carnegie Mellon University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of the Carnegie Mellon University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



/*-*-C++-*-*/
/**
   @file FlirCamera.h
   @author Chad Rockey
   @date July 11, 2011
   @brief Interface to Point Grey cameras

   @attention Copyright (C) 2011
   @attention National Robotics Engineering Center
   @attention Carnegie Mellon University
*/

#ifndef _FLIRCAMERA_H_
#define _FLIRCAMERA_H_

#include <sensor_msgs/Image.h> // ROS message header for Image
#include <sensor_msgs/image_encodings.h> // ROS header for the different supported image encoding types
#include <sensor_msgs/fill_image.h>
#include <flir_camera_driver/camera_exceptions.h>

#include <sstream>
#include <mutex>

// Header generated by dynamic_reconfigure
#include <flir_camera_driver/FlirConfig.h>
#include "flir_camera_driver/camera.h"
#include "flir_camera_driver/cm3.h"
#include "flir_camera_driver/set_property.h"


// Spinnaker SDK
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

namespace flir_camera_driver {
class FlirCamera
{

public:
  FlirCamera();
  ~FlirCamera();

  /*!
  * \brief Function that allows reconfiguration of the camera.
  *
  * This function handles a reference of a camera_library::CameraConfig object and
  * configures the camera as close to the given values as possible.  As a function for
  * dynamic_reconfigure, values that are not valid are changed by the driver and can
  * be inspected after this function ends.
  * This function will stop and restart the camera when called on a SensorLevels::RECONFIGURE_STOP level.
  * \param config  camera_library::CameraConfig object passed by reference.  Values will be changed to those the driver is currently using.
  * \param level  Reconfiguration level. See constants below for details.
  *
  * \return Returns true when the configuration could be applied without modification.
  */
  bool setNewConfiguration(flir_camera_driver::FlirConfig &config, const uint32_t &level);


  /** Parameters that need a sensor to be stopped completely when changed. */
  static const uint8_t LEVEL_RECONFIGURE_CLOSE = 3;

  /** Parameters that need a sensor to stop streaming when changed. */
  static const uint8_t LEVEL_RECONFIGURE_STOP = 1;

  /** Parameters that can be changed while a sensor is streaming. */
  static const uint8_t LEVEL_RECONFIGURE_RUNNING = 0;

  /*!
  * \brief Function that connects to a specified camera.
  *
  * Will connect to the camera specified in the setDesiredCamera(std::string id) call.  If setDesiredCamera is not called first
  * this will connect to the first camera.  Connecting to the first camera is not recommended for multi-camera or production systems.
  * This function must be called before setNewConfiguration() or start()!
  *
  * \return 0 if no error, -1 if error encountered.
  */
  int connect();

  /*!
  * \brief Disconnects from the camera.
  *
  * Disconnects the camera and frees it.
  *
  * \return 0 if no error, -1 if error encountered.
  */
  int disconnect();

  /*!
  * \brief Starts the camera loading data into its buffer.
  *
  * This function will start the camera capturing images and loading them into the buffer.  To retrieve images, grabImage must be called.
  *
  * \return 0 if no error, -1 if error encountered.
  */
  int start();

  /*!
  * \brief Stops the camera loading data into its buffer.
  *
  * This function will stop the camera capturing images and loading them into the buffer.
  *
  * \return Returns true if the camera was started when called.  Useful for knowing if the camera needs restarted in certain instances.
  */
  bool stop();


  /*!
  * \brief Loads the raw data from the cameras buffer.
  *
  * This function will load the raw data from the buffer and place it into a sensor_msgs::Image.
  * \param image sensor_msgs::Image that will be filled with the image currently in the buffer.
  * \param frame_id The name of the optical frame of the camera.
  *
  * \return 0 if no error, -1 if error encountered.
  */
  int grabImage(sensor_msgs::Image &image, const std::string &frame_id);

  void grabStereoImage(sensor_msgs::Image &image, const std::string &frame_id,
                      sensor_msgs::Image &second_image, const std::string &second_frame_id);

  /*!
  * \brief Will set grabImage timeout for the camera.
  *
  * This function will set the time required for grabCamera to throw a timeout exception.  Must be called after connect().
  * \param timeout The desired timeout value (in seconds)
  *
  */
  // TODO(tthomas): Implement later
  // void setTimeout(const double &timeout);

  /*!
  * \brief Used to set the serial number for the camera you wish to connect to.
  *
  * Sets the desired serial number.  If this value is not set, the driver will try to connect to the first camera on the bus.
  * This function should be called before connect().
  * \param id serial number for the camera.  Should be something like 10491081.
  */
  void setDesiredCamera(const uint32_t &id);
  std::vector<uint32_t> getAttachedCameras();

  void setGain(const float& gain);
  uint getHeightMax();
  uint getWidthMax();

  uint32_t getSerial()
  {
    return serial_;
  }

private:

  uint32_t serial_; ///< A variable to hold the serial number of the desired camera.

  Spinnaker::SystemPtr system_;
  Spinnaker::CameraList camList_;
  Spinnaker::CameraPtr pCam_;

  //TODO(tthomas) use std::shared_ptr
  Spinnaker::GenApi::INodeMap *node_map_;
  std::shared_ptr<Camera> camera_;

  Spinnaker::ChunkData image_metadata_;

  std::mutex mutex_; ///< A mutex to make sure that we don't try to grabImages while reconfiguring or vice versa.  Implemented with boost::mutex::scoped_lock.
  volatile bool captureRunning_; ///< A status boolean that checks if the camera has been started and is loading images into its buffer.

  /// If true, camera is currently running in color mode, otherwise camera is running in mono mode
  bool isColor_;

  // For GigE cameras:
  /// If true, GigE packet size is automatically determined, otherwise packet_size_ is used:
  bool auto_packet_size_;
  /// GigE packet size:
  unsigned int packet_size_;
  /// GigE packet delay:
  unsigned int packet_delay_;


  // This function configures the camera to add chunk data to each image. It does
  // this by enabling each type of chunk data before enabling chunk data mode.
  // When chunk data is turned on, the data is made available in both the nodemap
  // and each image.
  int ConfigureChunkData(Spinnaker::GenApi::INodeMap & nodeMap);
};
}
#endif
