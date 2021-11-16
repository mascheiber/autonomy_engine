// Copyright (C) 2021 Alessandro Fornasier,
// Control of Networked Systems, Universitaet Klagenfurt, Austria
//
// You can contact the author at <alessandro.fornasier@ieee.org>
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

#ifndef LAND_H
#define LAND_H

#include "state_machine/state.h"

namespace autonomy {

  /**
   * @brief Land state.
   */
  class Land : public State {

  public:

    /**
     * @brief Instance of State (Singleton)
     */
    static State& Instance();

    /**
     * @brief Action to be performed when exiting a state
     * @param Reference to Autonomy
     */
    void onExit(Autonomy& autonomy) override;

    /**
     * @brief Action to be performed when entering a state
     * @param  Reference to Autonomy
     */
    void onEntry(Autonomy& autonomy) override;

  private:

    /**
     * @brief Private constructor and copy-constructor
     */
    Land();
    Land(const Land& other);

    /**
     * @brief Assognment operator
     */
    Land& operator=(const Land& other);

  }; // calss Land

} // namespace autonomy

#endif  // LAND_H
