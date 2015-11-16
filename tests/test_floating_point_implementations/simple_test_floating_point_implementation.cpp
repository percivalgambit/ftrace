
/*! @file
 * This is a simple replacement function for arithmetic floating-point instructions
 * in the ftrace tool.
 */

#include <pin.H>

#include "shared/floating_point_implementation.h"
#include "shared/program_state.h"
#include "shared/register_floating_point_implementation.h"

namespace ftrace {

class SimpleTestFloatingPointImplementation : public FloatingPointImplementation {
 public:
  /*!
   * A simple implementation of floating-point arithmetic operations.
   */
  FLT32 FloatingPointOperation(const FLT32 &operand1, const FLT32 &operand2,
                               const OPCODE &operation,
                               const ProgramState &program_state) override {
    return 1.0;
  }
};

REGISTER_FLOATING_POINT_IMPL(SimpleTestFloatingPointImplementation);

}  // namespace ftrace
