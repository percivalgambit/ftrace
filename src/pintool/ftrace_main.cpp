/**
 * This is a Pin tool that can replace every floating-point arithmetic
 * instruction with user-defined functions to test different floating-point
 * implementations. It can also print the arguments and result of every
 * floating-point arithmetic instruction.
 *
 * @note All floating-point values are printed as 8 digit hex numbers padded
 * with 0's.
 */

#include <pin.H>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "client_lib/interfaces/fp_selector.h"
#include "client_lib/registry/internal/fp_selector_registry.h"
#include "pintool/instrument_routine.h"
#include "pintool/instrumentation_callbacks.h"

using ftrace::CloseOutputStream;
using ftrace::ExitCallback;
using ftrace::FpSelector;
using ftrace::ReplaceFpOperations;
using ftrace::PrintFpOperations;
using ftrace::StartCallback;
using ftrace::internal::FpSelectorRegistry;

KNOB<string> KnobFpSelectorName(KNOB_MODE_OVERWRITE, "pintool",
                                "fp_selector_name", "",
                                "specify the name of the FpSelector to use "
                                "when instrumenting an application");

KNOB<BOOL> KnobPrintFpOps(
    KNOB_MODE_WRITEONCE, "pintool", "print_fp_ops", "0",
    "print the value of every floating point operation in the instrumented "
    "program");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "ftrace.out",
                            "specify file name for ftrace output");
/**
 *  Prints out a help message.
 *
 *  @return An error code for the application.
 */
static INT32 Usage() {
  cerr << "This is a Pin tool that can replace every floating-point arithmetic "
          "instruction with user-defined functions to test different "
          "floating-point implementations. It can also print the arguments and "
          "result of every floating-point arithmetic instruction."
       << endl;
  cerr << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}

/**
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet
 * started.
 *
 * @param[in] argc Total number of elements in the argv array
 * @param[in] argv Array of command line arguments,
 *     including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[]) {
  PIN_InitSymbols();
  // Initialize PIN library. Print help message if -h(elp) is specified
  // in the command line or the command line is invalid.
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  const FpSelectorRegistry *fp_selector_registry =
      FpSelectorRegistry::GetFpSelectorRegistry();
  const string &fp_selector_name = KnobFpSelectorName.Value();
  // If the KnobFpSelectorName flag is specified on the command line, attempt to
  // look up the FpSelector from the registry and use it to instrument the
  // application program with a user-defined FP implementation if it is found.
  if (!fp_selector_name.empty()) {
    FpSelector *fp_selector =
        fp_selector_registry->GetFpSelectorOrDie(fp_selector_name);

    PIN_AddApplicationStartFunction(
        reinterpret_cast<APPLICATION_START_CALLBACK>(StartCallback),
        fp_selector);
    PIN_AddFiniFunction(reinterpret_cast<FINI_CALLBACK>(ExitCallback),
                        fp_selector);
    RTN_AddInstrumentFunction(
        reinterpret_cast<RTN_INSTRUMENT_CALLBACK>(ReplaceFpOperations),
        fp_selector);
  }

  const string &output_file_name = KnobOutputFile.Value();
  const BOOL &print_fp_ops = KnobPrintFpOps.Value();
  // If the KnobPrintFpOps flag is specified on the command line, instrument the
  // application program to print the arguments and result of every FP operation
  // formatted as 8 digit hex numbers padded with 0's to a file.
  if (print_fp_ops) {
    ofstream *output = new ofstream(output_file_name.c_str());
    cerr << "===============================================" << endl
         << "See file " << output_file_name << " for analysis results" << endl
         << "===============================================" << endl;

    PIN_AddFiniFunction(reinterpret_cast<FINI_CALLBACK>(CloseOutputStream),
                        output);
    INS_AddInstrumentFunction(
        reinterpret_cast<INS_INSTRUMENT_CALLBACK>(PrintFpOperations), output);
  }

  // Start the program, never returns.
  PIN_StartProgram();
}
