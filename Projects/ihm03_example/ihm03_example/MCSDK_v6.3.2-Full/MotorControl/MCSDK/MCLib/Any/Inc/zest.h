/* zest.h */

#ifndef _ZEST_H_
#define _ZEST_H_

#include "fixpmath.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h> /* memset */

typedef struct _ZEST_Params_
{
    float_t isrFreq_Hz;						/* Frequency of ZEST_run() call */
    float_t backgroundFreq_Hz;				/* Frequency of ZEST_runBackground() call */
    float_t fullScaleFreq_Hz;
    float_t fullScaleCurrent_A;
    float_t fullScaleVoltage_V;

    float_t thresholdInjectActiveCurrent_A;	/* Above this current injection is deemed active, and position tracking is possible */
    float_t thresholdResistanceCurrent_A;	/* Above this current resistance measurement is possible */
    float_t thresholdFreq_Hz;				/* Above this electrical frequency ZEST injection goes inactive */
    float_t speedPole_rps;					/*  */
} ZEST_Params;

typedef enum _ZEST_Mode_e_
{
    ZEST_INJECTMODE_None,
    ZEST_INJECTMODE_Auto,
    ZEST_INJECTMODE_Always,
    ZEST_INJECTMODE_AutoSquare,
} ZEST_Mode_e;


typedef struct _ZEST_Obj_
{
    uint16_t reserved[400];
} ZEST_Obj;


typedef ZEST_Obj* ZEST_Handle;

static inline ZEST_Handle ZEST_init(void *pMemory, const size_t size)
{
	ZEST_Handle handle = (ZEST_Handle) NULL;

    if (size >= sizeof(ZEST_Obj))
    {
        handle = (ZEST_Handle) pMemory;
        memset(pMemory, 0, size);
    }

    return handle;
}

extern void ZEST_setParams(ZEST_Handle handle, const ZEST_Params* pZestParams);

extern void ZEST_run(ZEST_Handle handle,
	     const Currents_Idq_t *pIdq_filt_pu,			/*  */
	     const Voltages_Udq_t *pVdq_filt_pu,			/*  */
     const FIXP_scaled_t* pOneOverFlux_pu_fps,   /* 1/flux in fixpointscaled */
     fixp30_t* pCorrection                       /* angle correction output */
);

extern void ZEST_runBackground(ZEST_Handle handle, const fixp30_t Fe_pu);

extern void ZEST_resetCorrection(ZEST_Handle handle);

/* Accessors */

/* Getters */

extern fixp30_t ZEST_getAngleIncrement(const ZEST_Handle handle);

extern fixp30_t ZEST_getCheckRs(const ZEST_Handle handle);

extern fixp30_t ZEST_getCorrectionLimit(const ZEST_Handle handle);

extern fixp_t ZEST_getFeedback_wi_rad_s(const ZEST_Handle handle);

extern fixp30_t ZEST_getFeedbackOffsetAngle(const ZEST_Handle handle);

extern bool ZEST_getFlagInjectActive(const ZEST_Handle handle);

extern fixp_t ZEST_getGainD(const ZEST_Handle handle);

extern fixp_t ZEST_getGainQ(const ZEST_Handle handle);

extern float_t ZEST_getIdInjectAmpl_A(const ZEST_Handle handle);

extern float_t ZEST_getIdInjectRef_A(const ZEST_Handle handle);

extern void ZEST_getIdq_ref_inject_pu(const ZEST_Handle handle, Currents_Idq_t *pIdq_ref_inject_pu);

extern fixp_t ZEST_getInjectFactor(const ZEST_Handle handle);

extern fixp_t ZEST_getInjectFactorDecrement(const ZEST_Handle handle);

extern fixp_t ZEST_getInjectFactorIncrement(const ZEST_Handle handle);

extern fixp30_t ZEST_getInjectFreqkHz(const ZEST_Handle handle);

extern float_t ZEST_getInjectMeasuredCurrent_A(const ZEST_Handle handle);

extern float_t ZEST_getL(const ZEST_Handle handle);

extern fixp30_t ZEST_getPostFiltersFactor_pu(const ZEST_Handle handle);

extern float_t ZEST_getR(const ZEST_Handle handle);

extern uint32_t ZEST_getRevision(const ZEST_Handle handle);

extern fixp30_t ZEST_getSignalD(const ZEST_Handle handle);

extern fixp30_t ZEST_getSignalQ(const ZEST_Handle handle);

extern fixp30_t ZEST_getThresholdFreq_pu(const ZEST_Handle handle);

extern float_t ZEST_getThresholdInjectActiveCurrent_A(const ZEST_Handle handle);

extern float_t ZEST_getThresholdResistanceCurrent_A(const ZEST_Handle handle);

/* Setters */

extern void ZEST_setCorrectionLimit(ZEST_Handle handle, const fixp30_t limit);

extern void ZEST_setFeedback_wi_rad_s(ZEST_Handle handle, const fixp_t wi_rad_s);

extern void ZEST_setFreqInjectkHz(ZEST_Handle handle, const fixp30_t freqkHz);

extern void ZEST_setGainD(ZEST_Handle handle, const fixp_t gainD);

extern void ZEST_setGainQ(ZEST_Handle handle, const fixp_t gainQ);

extern void ZEST_setIdInject_pu(ZEST_Handle handle, const fixp30_t IDinject_pu);

extern void ZEST_setInjectMode(ZEST_Handle handle, const ZEST_Mode_e inject_mode);

extern void ZEST_setInjectFactorDecrement(ZEST_Handle handle, const fixp_t decrement);

extern void ZEST_setInjectFactorIncrement(ZEST_Handle handle, const fixp_t increment);

extern void ZEST_setIqInject_pu(ZEST_Handle handle, const fixp30_t IQinject_pu);

extern void ZEST_setPostFiltersFactor_pu(ZEST_Handle handle, const fixp30_t lppf);

extern void ZEST_setThresholdFreq_pu(ZEST_Handle handle, const fixp30_t freq_pu);

extern void ZEST_setThresholdInjectActiveCurrent_A(ZEST_Handle handle, const float_t value);

extern void ZEST_setThresholdResistanceCurrent_A(ZEST_Handle handle, const float_t value);

#endif /* _ZEST_H_ */

/* end of zest.h */
