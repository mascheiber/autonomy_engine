// Copyright (C) 2021 Christian Brommer and Alessandro Fornasier,
// Control of Networked Systems, Universitaet Klagenfurt, Austria
//
// You can contact the author at <christian.brommer@ieee.org>
// and <alessandro.fornasier@ieee.org>
//
// All rights reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "autonomy_core/autonomy.h"
#include "utils/colors.h"

AmazeAutonomy::AmazeAutonomy(ros::NodeHandle &nh) :
  nh_(nh), reconfigure_cb_(boost::bind(&AmazeAutonomy::configCallback, this, _1, _2)) {

  // Parse parameters and options
  if(!parseParams()) {
      throw std::exception();
  }

  // Print option
  opts_->printAutonomyOptions();

  // Set dynamic reconfigure callback
  reconfigure_srv_.setCallback(reconfigure_cb_);

  // Advertise watchdog service
  service_client_ = nh_.serviceClient<watchdog_msgs::Start>("/watchdog/service/start");

  // Subscriber to watchdog (system status) heartbeat
  sub_watchdog_heartbeat_ = nh_.subscribe("/watchdog/status", 1, &AmazeAutonomy::watchdogHeartBeatCallback, this);

  // Subscribe to watchdog status changes
  sub_watchdog_status_ = nh.subscribe("/watchdog/log", 1, &AmazeAutonomy::watchdogStatusCallback, this);

  // Instanciate timeout timer and connect signal
  timer_ = std::make_shared<Timer>(opts_->timeout);
  timer_->sh_.connect(boost::bind(&AmazeAutonomy::watchdogTimerOverflowHandler, this));
}

bool AmazeAutonomy::parseParams() {

  // Define auxilliary variables
  int watchdog_startup_time_s, n_missions, watchdog_timeout_ms;
  std::string description;
  std::map<int, std::string> missions;
  Entity entity;
  NextState action;
  XmlRpc::XmlRpcValue entities_actions;
  std::pair<int, std::pair<Entity, NextState>> mission_entity_action;
  std::vector<std::pair<int, std::pair<Entity, NextState>>> entity_action_vector;

  // Get watchdog rate
  if(!nh_.getParam("watchdog_rate", watchdog_timeout_ms)) {
     std::cout << std::endl << BOLD(RED("Watchdog heartbeat rate not defined")) << std::endl;
  }

  // Set watchdog timer timeout to be 125% of 1/watchdog_rate
  watchdog_timeout_ms = 1250/watchdog_timeout_ms;

  // Get watchdog startup time
  nh_.param<int>("watchdog_startup_time_s", watchdog_startup_time_s, 5);

  // Get missions information
  nh_.param<int>("missions/number", n_missions, 0);

  // Check whether missions are defined and parse them
  if (n_missions == 0) {
    std::cout << std::endl << BOLD(RED("No missions defined")) << std::endl;
    return false;
  } else {

    // Loop through missions
    for (int i = 1; i <= n_missions; ++i) {

      // get mission description
      if (!nh_.getParam("missions/mission_" + std::to_string(i) + "/description", description)) {
        std::cout << std::endl << BOLD(RED("Mission number " + std::to_string(i) + " description missing")) << std::endl;
        return false;
      }

      // Build Mission map <i, description>
      missions.insert({i, description});

      // get entities and actions
      if (!nh_.getParam("missions/mission_" + std::to_string(i) + "/entities_actions", entities_actions)) {
        std::cout << std::endl << BOLD(RED("Mission number " + std::to_string(i) + " entities_actions list missing")) << std::endl;
        return false;
      }

      // Check type to be array
      if (entities_actions.getType() ==  XmlRpc::XmlRpcValue::TypeArray) {

        // Loop through entities and actions
        for (int j = 0; j < entities_actions.size(); ++j) {

          // Check type to be array
          if (entities_actions[j].getType() ==  XmlRpc::XmlRpcValue::TypeArray) {

            // Get entity and action
            if (getEntityAction(entities_actions[j], entity, action)) {
              std::pair entity_action = std::make_pair(entity, action);

              // Build Entity Action vector
              mission_entity_action = std::make_pair(i,entity_action);
              entity_action_vector.emplace_back(mission_entity_action);

            } else {
              std::cout << std::endl << BOLD(RED("Mission number " + std::to_string(i) + " entities_actions list wrongly defined")) << std::endl;
              return false;
            }

          } else {
            std::cout << std::endl << BOLD(RED("Mission number " + std::to_string(i) + " entities_actions list wrongly defined")) << std::endl;
            return false;
          }
        }
      } else {
        std::cout << std::endl << BOLD(RED("Mission number " + std::to_string(i) + " entities_actions list wrongly defined")) << std::endl;
        return false;
      }
    }
  }

  // Make options
  opts_ = std::make_shared<autonomyOptions>(autonomyOptions({watchdog_timeout_ms, watchdog_startup_time_s, missions, entity_action_vector}));

  // Success
  return true;
}

bool AmazeAutonomy::getEntityAction(const XmlRpc::XmlRpcValue& entity_action, Entity& entity, NextState& action) {

  // Check type
  if (entity_action.getType() ==  XmlRpc::XmlRpcValue::TypeArray) {

    // Check type and get entity
    if (entity_action[0].getType() == XmlRpc::XmlRpcValue::TypeString) {
      if (std::string(entity_action[0]).compare("px4_gps") == 0) {
        entity = Entity::PX4_GPS;
      } else if (std::string(entity_action[0]).compare("px4_bar") == 0) {
        entity = Entity::PX4_BAR;
      } else if (std::string(entity_action[0]).compare("px4_mag") == 0) {
        entity = Entity::PX4_MAG;
      } else if (std::string(entity_action[0]).compare("mission_cam") == 0) {
        entity = Entity::MISSION_CAM;
      } else if (std::string(entity_action[0]).compare("realsense") == 0) {
        entity = Entity::REALSENSE;
      } else if (std::string(entity_action[0]).compare("lsm9ds1") == 0) {
        entity = Entity::LSM9DS1;
      } else if (std::string(entity_action[0]).compare("lrf") == 0) {
        entity = Entity::LRF;
      } else if (std::string(entity_action[0]).compare("rtk_gps_1") == 0) {
        entity = Entity::RTK_GPS_1;
      } else if (std::string(entity_action[0]).compare("rtk_gps_2") == 0) {
        entity = Entity::RTK_GPS_2;
      } else {
        return false;
      }
    } else {
      return false;
    }

    // Check type and get action
    if (entity_action[1].getType() == XmlRpc::XmlRpcValue::TypeString) {
      if (std::string(entity_action[1]).compare("continue") == 0) {
        action = NextState::NOMINAL;
      } else if (std::string(entity_action[1]).compare("hold") == 0) {
        action = NextState::HOLD;
      } else if (std::string(entity_action[1]).compare("manual") == 0) {
        action = NextState::MANUAL;
      } else {
        return false;
      }
    } else {
      return false;
    }

    return true;

  } else {
    return false;
  }
}

void AmazeAutonomy::startWatchdog() {

  // Define service request
  service_.request.header.stamp = ros::Time::now();
  service_.request.startup_time = opts_->watchdog_startup_time;

  // Call service request
  if (service_client_.call(service_)) {

    // Check responce
    if(service_.response.successful) {

      std::cout << std::endl << BOLD(GREEN("--------- WATCHDOG IS RUNNING ---------")) << std::endl << std::endl;
      std::cout << BOLD(GREEN(" System status is [NOMINAL] ")) << std::endl;
      std::cout << std::endl << BOLD(GREEN("---------------------------------------")) << std::endl;

    } else {
       std::cout << std::endl << BOLD(RED("------ FAILED TO START WATCHDOG -------")) << std::endl << std::endl;
       std::cout << BOLD(RED(" Please perform a system hard restart  ")) << std::endl;
       std::cout << BOLD(RED(" If you get the same problem after the ")) << std::endl;
       std::cout << BOLD(RED(" hard restart, shutdown the system and ")) << std::endl;
       std::cout << BOLD(RED(" abort the mission. ")) << std::endl << std::endl;
       std::cout << BOLD(RED("---------------------------------------")) << std::endl;

       // Eventually include debug information here

       throw std::exception();
    }
  } else {
    std::cout << std::endl << BOLD(RED("------- FAILED TO CALL SERVICE --------")) << std::endl << std::endl;
    std::cout << BOLD(RED(" Please perform a system hard restart  ")) << std::endl;
    std::cout << BOLD(RED(" If you get the same problem after the ")) << std::endl;
    std::cout << BOLD(RED(" hard restart, shutdown the system and ")) << std::endl;
    std::cout << BOLD(RED(" abort the mission. ")) << std::endl << std::endl;
    std::cout << BOLD(RED("---------------------------------------")) << std::endl;
    throw std::exception();
  }
}

void AmazeAutonomy::watchdogHeartBeatCallback(const watchdog_msgs::StatusStampedConstPtr& msg) {

  // Restart timeout timer
  timer_->resetTimer();
}

void AmazeAutonomy::watchdogStatusCallback(const watchdog_msgs::StatusChangesArrayStamped& msg) {

  // Parse the message
  // msg.data.changes.end();
  // changes is the changes wrt the last change

}

void AmazeAutonomy::watchdogTimerOverflowHandler() {

  // print message of watchdog timer overflow
  std::cout << std::endl << BOLD(RED("Timeout overflow -- no heartbeat from system watchdog")) << std::endl;
}

void AmazeAutonomy::configCallback(amaze_autonomy::autonomyConfig& config, uint32_t level) {

  if (config.option_a) {
    std::cout << "Option A was choosen in the Reconfigure GUI" << std::endl;
    config.option_a = false;
  }
}

void AmazeAutonomy::userInterface() {

  // Print missions
  std::cout << std::endl << BOLD(GREEN("Please select one of the following mission by inputting the mission ID")) << std::endl << std::endl;
  for (auto &it : opts_->missions) {
    std::cout << BOLD(GREEN(" - ID: ")) << it.first << BOLD(GREEN(" DESCRIPTION: ")) << it.second << std::endl;
  }

  // Get ID of mission being executed
  std::cout << std::endl << BOLD(GREEN(">>> "));
  std::cin >> mission_id_;

  // Check validity of mission id
  if (mission_id_ == 0 || mission_id_ > opts_->missions.size()) {
    std::cout << std::endl << BOLD(RED("Wrong mission ID chosen")) << std::endl;
    throw std::exception();
  } else {
    std::cout << std::endl << BOLD(GREEN(" - Selected mission with ID: ")) << mission_id_ << std::endl;
  }

  // check result of preflight checks
  std::cout << std::endl << BOLD(GREEN("Start Pre-Flight Checks ...")) << std::endl;
  if (!AmazeAutonomy::preFlightChecks()) {
    std::cout << std::endl << BOLD(RED("Pre-Flight checks failure")) << std::endl;
    throw std::exception();
  }
}

bool AmazeAutonomy::preFlightChecks() {

  // service call to check if we are ready to takeoff

  return true;
}
