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
#include <ros/ros.h>
#include <limits>

int main(int argc, char* argv[])
{
  // Init ros node with private nodehandler
  ros::init(argc, argv, "amaze_autonomy");
  ros::NodeHandle nh("~");

  // Provide a reliable low-latency communication. However, this affect the bandwidth
  // If you encounter bandwith saturation issues comment this line
  ros::TransportHints().tcpNoDelay();

  ROS_INFO("Starting the AMAZE Autonomy");

  // Start asynch (multi-threading) spinner
  ros::AsyncSpinner spinner(0);
  spinner.start();

  try {

    // Instanciate AMAZE autonomy
    AmazeAutonomy autonomy(nh);

    std::cout << std::endl << BOLD(GREEN(" >>> Press Enter to start the AMAZE Autonomy"));
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Start the autonomy
    autonomy.startAutonomy();

    // Wait for the node to be killed
    ros::waitForShutdown();

  } catch (const ManualException&) {

    // Stop the spinner
    spinner.stop();

    // Print info
    std::cout << std::endl << BOLD(RED(" >>> AMAZE autonomy disabled <<<")) << std::endl;

  }

  return EXIT_SUCCESS;
}
