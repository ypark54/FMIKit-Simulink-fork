/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include <stdarg.h>

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#include "fmi2Functions.h"

#define FMI_VERSION 2
#define CO_SIMULATION

#include "sfun_fmu_common.c"


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

  time_T startTime = ssGetT(S);
  time_T stopTime = ssGetTFinal(S);

  static fmi2CallbackFunctions callbacks = { logFMUMessage, calloc, free, NULL, NULL };

	COMPONENT = fmi2Instantiate(ssGetPath(S), fmi2CoSimulation, MODEL_GUID, "", &callbacks, fmi2False, fmi2False);

	if (!COMPONENT) {
		ssSetErrorStatus(S, "Failed to instatiate FMU");
		return;
	}

	setStartValues(S);

	if (ssGetErrorStatus(S)) {
		return;  // may have been set by setStartValues()
	}

	ASSERT_OK(fmi2SetupExperiment(COMPONENT, fmi2False, 0, startTime, stopTime > startTime, stopTime), "Failed to set up experiment")

	ASSERT_OK(fmi2EnterInitializationMode(COMPONENT), "Failed to enter initialization mode")

	ASSERT_OK(fmi2ExitInitializationMode(COMPONENT), "Failed to exit initialization mode")

	FMU_TIME = ssGetT(S);
}
#endif /* MDL_START */


static void mdlOutputs(SimStruct *S, int_T tid) {

    if (ssGetT(S) > FMU_TIME) {

		ASSERT_OK(fmi2DoStep(COMPONENT, FMU_TIME, ssGetT(S) - FMU_TIME, fmi2True), "Failed to do step")

		FMU_TIME = ssGetT(S);
	}

	setOutputs(S);

	if (ssGetErrorStatus(S)) {
		return;  // may have been set by setOutputs()
	}
}


static void mdlTerminate(SimStruct *S) {
	ASSERT_OK(fmi2Terminate(COMPONENT), "Failed to terminate model");
	fmi2FreeInstance(COMPONENT);
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
