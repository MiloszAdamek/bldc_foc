/**
  ******************************************************************************
  * @file    g4xx_bemf_ADC_fdbk.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          Sensorless Bemf acquisition with ADC component of the Motor Control SDK.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  * @ingroup SpeednPosFdbk_Bemf
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BEMFADCFDBK_H
#define BEMFADCFDBK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "speed_pos_fdbk.h"
#include "speed_ctrl.h"
#include "pwmc_sixstep.h"
#include "bus_voltage_sensor.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup SpeednPosFdbk
  * @{
  */

/** @addtogroup SpeednPosFdbk_Bemf
  * @{
  */

#define SPEED_BUFFER_LENGTH     ((uint8_t) 18) /*!< Length of buffer used to store the timer period measurements */

  typedef enum
{
  DEMAGNETIZATION, COMMUTATION
} SpeedTimerState_t ;
/**
  * @brief  Bemf_ADC parameters definition
  */
typedef struct
{
  TIM_TypeDef * LfTim;                 /*!< Contains the pointer to the LF timer
                                            used for speed measurement. */
  uint32_t LfTimerChannel;             /*!< Channel of the LF timer used for speed measurement */
  ADC_TypeDef * pAdc[6];               /*!< Pointer to the ADC */
  uint32_t AdcChannel[6];              /*!< Array of ADC channels used for BEMF sensing */
  bool gpio_divider_available;         /*!< Availability of the GPIO port enabling the bemf resistor divider */
  GPIO_TypeDef * bemf_divider_port;    /*!< GPIO port enabling the bemf resistor divider */
  uint16_t bemf_divider_pin;           /*!< GPIO pin enabling the bemf resistor divider */
} Bemf_ADC_Params_t;

/**
  * @brief This structure is used to handle the thresholds for bemf zero crossing detection
  *
  */
typedef struct
{
  uint16_t AdcThresholdPwmPerc;        /*!< BEMF voltage threshold for zero crossing detection when BEMF is decreasing */
  uint16_t AdcThresholdHighPerc;        /*!< BEMF voltage threshold for zero crossing detection when BEMF is decreasing */
  uint16_t AdcThresholdLowPerc;          /*!< BEMF voltage threshold for zero crossing detection when BEMF is increasing */
  uint16_t Bus2ThresholdConvFactor;          /*!< BEMF voltage threshold for zero crossing detection when BEMF is increasing */
  uint16_t ThresholdCorrectFactor;
  uint16_t SamplingPointOff;           /*!< Pulse value of the timer channel used to trig the ADC */
  uint16_t SamplingPointOn;           /*!< Pulse value of the timer channel used to trig the ADC */
  uint8_t AWDfiltering;
} Bemf_Sensing_Params;

/**
  * @brief This structure is used to handle the demagnetization time before starting bemf acquisition
  *
  */
typedef struct
{
  uint16_t DemagMinimumSpeedUnit;    /*!< Speed threshold for minimum demagnetization time */
  uint32_t RevUpDemagSpeedConv;      /*!< Convertion factor between speed and demagnetization time */
  uint32_t RunDemagSpeedConv;        /*!< Open loop convertion factor between speed and demagnetization time during */
  uint32_t DemagMinimumThreshold;    /*!< Minimum demagnetization time */
} Bemf_Demag_Params;

/**
  * @brief This structure is used to handle the data of an instance of the B-emf Feedback component
  *
  */
typedef struct
{
  SpeednPosFdbk_Handle_t _Super;
  uint8_t ZcEvents;               /**< Number of bemf zero crossing events */
  Bemf_Sensing_Params Pwm_H_L;     /**< Parameters for zero crossing detection during ON time */
  uint16_t pSensing_Threshold_HSMod;
  uint16_t pSensing_Threshold_LSMod;
  uint16_t *pSensing_Point;
  bool IsOnSensingEnabled;     /*!< Value where 0 means BEMF is sensed during PWM OFF time and 1 or greater means BEMF is sensed during PWM ON time */
  uint16_t OnSensingEnThres;   /*!< Pulse value of HF timer above which the PWM ON sensing is enabled */
  uint16_t OnSensingDisThres;  /*!< Pulse value of HF timer below which the PWM ON sensing is disabled */
  uint16_t ZcRising2CommDelay; /*!< Rising Zero Crossing detection to commutation delay in 15/128 degrees */
  uint16_t ZcFalling2CommDelay; /*!< Falling Zero Crossing detection to commutation delay in 15/128 degrees */
  Bemf_ADC_Params_t const *pParams_str;
  uint16_t SpeedSamplingFreqHz; /*!< Frequency (Hz) at which motor speed is to
                             be computed. It must be equal to the frequency
                             at which function SPD_CalcAvrgMecSpeedUnit
                             is called.*/
  uint8_t  SpeedBufferSize; /*!< Size of the buffer used to calculate the average
                             speed. It must be less than 18.*/
  uint32_t TIMClockFreq;
  bool ADCRegularLocked;      /*!< This flag is set when ADC is locked for bemf acquisition */
  int32_t  ElPeriodSum;       /*!< Period accumulator used to speed up the average speed computation*/
  int16_t PrevRotorFreq;      /*!< Used to store the last valid rotor electrical
                                   speed in dpp used when MAX_PSEUDO_SPEED
                                   is detected */
  int8_t Direction;           /*!< Instantaneous direction of rotor between two
                                   captures*/
  int16_t AvrElSpeedDpp; /*!< Averaged rotor electrical speed express
                               in s16degree per current control period.*/
  int16_t VirtualElSpeedDpp; /*!< Averaged rotor electrical speed express
                               in s16degree per current control period.*/
  uint8_t StartUpConsistThreshold;   /*!< Number of consecutive tests on speed
                                           consistency to be passed before
                                           validating the start-up */
  bool IsSpeedReliable;        /*!< Latest private speed reliability information,
                                     updated by SPD_CalcAvrgMecSpeedUnit, it is
                                     true if the speed measurement variance is
                                     lower then threshold corresponding to
                                     hVariancePercentage */
  bool IsAlgorithmConverged;   /*!< Boolean variable containing observer
                                     convergence information */
  bool IsLoopClosed;           /*!< Boolean variable containing speed
                                     loop status*/
  DrivingMode_t DriveMode;
  volatile uint8_t BufferFilled;   /*!< Indicates the number of speed measuremt
                                         present in the buffer from the start.
                                         It will be max bSpeedBufferSize and it
                                         is used to validate the start of speed
                                         averaging. If bBufferFilled is below
                                         bSpeedBufferSize the instantaneous
                                         measured speed is returned as average
                                         speed.*/
  int32_t SpeedBufferDpp[SPEED_BUFFER_LENGTH];/*!< Holding the last period captures */
  uint32_t LowFreqTimerPsc; /*!< Prescaler value of the low frequency timer */
  uint16_t SpeedFIFOIdx;/*!< Pointer of next element to be stored in
                                         the speed sensor buffer*/
  int16_t DeltaAngle;     /*!< Delta angle at the Hall sensor signal edge between
                               current electrical rotor angle of synchronism.
                               It is in s16degrees.*/
  int16_t MeasuredElAngle;/*!< Electrical angle  measured at each
                               bemf zero crossing. It is considered the
                               best measurement of electrical rotor angle.*/
  int16_t CompSpeed;      /*!< Speed compensation factor used to syncronize
                               the current electrical angle with the target
                               electrical angle. */
  uint16_t SatSpeed;      /*!< Returned value if the measured speed is above the
                               maximum realistic.*/
  uint32_t PseudoPeriodConv;/*!< Conversion factor between time interval Delta T
                             between bemf zero crossing points, express in timer
                             counts, and electrical rotor speed express in dpp.
                             Ex. Rotor speed (dpp) = wPseudoFreqConv / Delta T
                             It will be ((CKTIM / 6) / (SAMPLING_FREQ)) * 65536.*/
  uint32_t MaxPeriod;   /*!< Time delay between two bemf zero crossing points when the speed
                             of the rotor is the minimum realistic in the
                             application: this allows to discriminate too low
                             freq for instance.
                             This period shoud be expressed in timer counts and
                             it will be:
                             wMaxPeriod = ((10 * CKTIM) / 6) / MinElFreq(0.1Hz).*/
  uint32_t MinPeriod;  /*!< Time delay between two bemf zero crossing points when the speed
                            of the rotor is the maximum realistic in the
                            application: this allows discriminating glitches
                            for instance.
                            This period shoud be expressed in timer counts and
                            it will be: wSpeedOverflow = ((10 * CKTIM) / 6) / MaxElFreq(0.1Hz).*/
  uint16_t PWMNbrPSamplingFreq; /*!< Number of current control periods inside
                             each speed control periods it will be:
                             (hMeasurementFrequency / hSpeedSamplingFreqHz) - 1.*/
  uint8_t ComputationDelay;       /*!< Selection of the period for computation of the delay between the zero crossing and the step change */
  uint8_t PWMFreqScaling;       /*!< Scaling factor to allow to store a PWMFrequency greater than 16 bits */
  uint32_t Counter_Period;      /*!< Low frequency timer period that allows speed calculation */
  uint32_t StepTime_Up;       /*!< Low frequency timer counter value at zero crossing with increasing bemf */
  uint32_t StepTime_Down;     /*!< Low frequency timer counter value at zero crossing with increasing bemf */
  uint32_t StepTime_Last;     /*!< Low frequency timer counter value at zero crossing with increasing bemf */
  uint32_t Last_TimerSpeed_Counter;  /*!< Last delay between zero crossing and step change */
  uint32_t DemagCounterThreshold; /*!< PWM cycles dedicated to windings demagnetization */
  Bemf_Demag_Params DemagParams;  /*!< Demagnetization parameters */
  SpeedTimerState_t SpeedTimerState;
} Bemf_ADC_Handle_t;

/* Exported functions --------------------------------------------------------*/

/* Initializes all the object variables. */
void BADC_Init(Bemf_ADC_Handle_t *pHandle);

/* Resets the ADC status and empties arrays. */
void BADC_Clear(Bemf_ADC_Handle_t *pHandle);

/* Gets ADC value and check for zero crossing detection.*/
void BADC_IsZcDetected(Bemf_ADC_Handle_t *pHandle, uint8_t step);

/* Sets the trigger point of the ADC */
void BADC_SetSamplingPoint(Bemf_ADC_Handle_t *pHandle, PWMC_Handle_t *pHandlePWMC, BusVoltageSensor_Handle_t *BusVHandle);

/* Computes the rotor average mechanical speed in the unit defined by #SPEED_UNIT and returns it in pMecSpeedUnit. */
bool BADC_CalcAvrgMecSpeedUnit(Bemf_ADC_Handle_t *pHandle, int16_t * pMecSpeedUnit);

/* Forces the rotation direction. */
void BADC_SetDirection(Bemf_ADC_Handle_t *pHandle, uint8_t direction);

/* Checks whether the state observer algorithm converged.*/
bool BADC_IsObserverConverged(Bemf_ADC_Handle_t *pHandle);

/* Starts the bemf acquisition.*/
void BADC_Start(Bemf_ADC_Handle_t *pHandle, uint8_t step, uint8_t LSModArray[]);

/* Stops the bemf acquisition.*/
void BADC_Stop(Bemf_ADC_Handle_t *pHandle);

/* Selects the phase for the bemf acquisition.*/
void BADC_SelectAdcChannel(Bemf_ADC_Handle_t *pHandle, uint8_t step);

/* Updates the estimated electrical angle.*/
int16_t BADC_CalcElAngle(Bemf_ADC_Handle_t *pHandle);

/* Configures the sensorless parameters for the following step. */
void BADC_StepChangeEvent(Bemf_ADC_Handle_t *pHandle, int16_t hElSpeedDpp);

/* Computes the demagnetization time during revup procedure. */
void BADC_CalcRevUpDemagTime(Bemf_ADC_Handle_t *pHandle);

/* Computes the demagnetization time in closed loop operation.*/
void BADC_CalcRunDemagTime(Bemf_ADC_Handle_t *pHandle);

/* Sets the flag when switch over phase ends.*/
void BADC_SetLoopClosed(Bemf_ADC_Handle_t *pHandle);

/* Disables low frequecy timer interrupt */
void BADC_SpeedMeasureOff(Bemf_ADC_Handle_t *pHandle);

/* Sets the parameters for bemf sensing during pwm off-time */
void BADC_SetBemfSensorlessParam(Bemf_ADC_Handle_t *pHandle, Bemf_Sensing_Params *BemfAdcConfig, uint16_t *ZcRising2CommDelay, uint16_t *ZcFalling2CommDelay,
                                   Bemf_Demag_Params *bemfAdcDemagConfig, uint16_t *OnSensingEnThres, uint16_t *OnSensingDisThres, uint16_t *ComputationDelay);

/* Gets the parameters for bemf sensing during pwm off-time */
void BADC_GetBemfSensorlessParam(Bemf_ADC_Handle_t *pHandle, Bemf_Sensing_Params *BemfAdcConfig, uint16_t *ZcRising2CommDelay, uint16_t *ZcFalling2CommDelay,
                                   Bemf_Demag_Params *BemfAdcDemagConfig, uint16_t *OnSensingEnThres, uint16_t *OnSensingDisThres, uint16_t *ComputationDelay);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif /* __cpluplus */

#endif /* BEMFADCFDBK_H */

/******************* (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/
