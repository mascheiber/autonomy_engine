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

#ifndef CONTINUE_H
#define CONTINUE_H

#include "state_machine/abstract_state.h"

/**
 * @brief Nominal state. The predefine behaviour continues
 */
class Nominal : public AbstractState {

public:

  /**
   * @brief Instance of State (Singleton)
   */
  static AbstractState& Instance();

  /**
   * @brief Trigger a state transition
   * @param Pointer to State
   * @param reference to entity event
   */
  void stateTransition(State* state, EntityEvent& event) override;

  /**
   * @brief Action to be performed when exiting a state
   * @param Pointer to State
   * @param Event that triggered the exit from the state
   */
  void onExit(State* state, EntityEvent& event) override;

  /**
   * @brief Action to be performed when entering a state
   * @param Pointer to State
   * @param Event that triggered the entry to the state
   */
  void onEntry(State* state, EntityEvent& event) override;

  /**
   * @brief Trigger a state transition from undefined state to nominal state
   * @param Pointer to State
   */
  void nominal(State* state) override;


private:

  /**
   * @brief Private constructor and copy-constructor
   */
  Nominal();
  Nominal(const Nominal& other);

  /**
   * @brief Assognment operator
   */
  Nominal& operator=(const Nominal& other);

};

#endif  // CONTINUE_H
