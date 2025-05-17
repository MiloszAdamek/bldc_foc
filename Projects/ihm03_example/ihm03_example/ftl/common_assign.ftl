<#assign FOC = MC.M1_DRIVE_TYPE == "FOC" || MC.M2_DRIVE_TYPE == "FOC">
<#assign SIX_STEP = MC.M1_DRIVE_TYPE == "SIX_STEP" || MC.M2_DRIVE_TYPE == "SIX_STEP">
<#assign ACIM = MC.M1_DRIVE_TYPE == "ACIM" || MC.M2_DRIVE_TYPE == "ACIM">
<#assign MOTOR_PROFILER = MC.M1_MOTOR_PROFILER || MC.M2_MOTOR_PROFILER>

<#assign M1_ENCODER = (MC.M1_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_SPEED_SENSOR == "QUAD_ENCODER_Z") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
<#assign M2_ENCODER = (MC.M2_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_SPEED_SENSOR == "QUAD_ENCODER_Z") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
<#assign M1_HALL_SENSOR = (MC.M1_SPEED_SENSOR == "HALL_SENSOR") || (MC.M1_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR")>
<#assign M2_HALL_SENSOR = (MC.M2_SPEED_SENSOR == "HALL_SENSOR") || (MC.M2_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR")>
<#assign AUX_SPEED_FDBK_M1 = ((MC.M1_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z") || (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_PLL") || (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_CORDIC"))>
<#assign AUX_SPEED_FDBK_M2 = ((MC.M2_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z") || (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_PLL") || (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_CORDIC"))>

<#assign M1_IS_STO_CORDIC = (MC.M1_SPEED_SENSOR == "STO_CORDIC") || (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_CORDIC")>
<#assign M2_IS_STO_CORDIC = (MC.M2_SPEED_SENSOR == "STO_CORDIC") || (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_CORDIC")>
<#assign M1_IS_STO_PLL = (MC.M1_SPEED_SENSOR == "STO_PLL") || (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_PLL")>
<#assign M2_IS_STO_PLL = (MC.M2_SPEED_SENSOR == "STO_PLL") || (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_PLL")>
<#assign M1_IS_SENSORLESS = (MC.M1_SPEED_SENSOR == "STO_CORDIC") || (MC.M1_SPEED_SENSOR == "STO_PLL")>
<#assign M2_IS_SENSORLESS = (MC.M2_SPEED_SENSOR == "STO_CORDIC") || (MC.M2_SPEED_SENSOR == "STO_PLL")>
<#assign M1_IS_ENCODER = (MC.M1_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_SPEED_SENSOR == "QUAD_ENCODER_Z") ||  (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
<#assign M2_IS_ENCODER = (MC.M2_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_SPEED_SENSOR == "QUAD_ENCODER_Z") ||  (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
<#assign M1_IS_HALL_SENSOR = (MC.M1_SPEED_SENSOR == "HALL_SENSOR") || (MC.M1_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR")>
<#assign M2_IS_HALL_SENSOR = (MC.M2_SPEED_SENSOR == "HALL_SENSOR") || (MC.M2_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR")>

<#-- Charge Boot Cap enable condition -->
<#assign CHARGE_BOOT_CAP_ENABLING = (! MC.M1_OTF_STARTUP) || ((MC.M1_OTF_STARTUP) && (MC.M1_PWM_DRIVER_PN == "STDRIVE101") && (MC.M1_DP_TOPOLOGY != "NONE"))>
<#assign CHARGE_BOOT_CAP_ENABLING2 = (! MC.M2_OTF_STARTUP) || ((MC.M2_OTF_STARTUP) && (MC.M2_PWM_DRIVER_PN == "STDRIVE101") && (MC.M2_DP_TOPOLOGY != "NONE"))>

<#-- Condition for STM32F302x8x MCU -->
<#assign CondMcu_STM32F302x8x = (McuName?? && McuName?matches("STM32F302.8.*"))>
<#-- Condition for STM32F072xxx MCU -->
<#assign CondMcu_STM32F072xxx = (McuName?? && McuName?matches("STM32F072.*"))>
<#-- Condition for STM32F050xxx MCU -->
<#assign CondMcu_STM32F050xxx = (McuName?? && McuName?matches("STM32F050.*"))>
<#-- Condition for STM32F051xxx MCU -->
<#assign CondMcu_STM32F051xxx = (McuName?? && McuName?matches("STM32F051.*"))>
<#-- Condition for STM32F030xxx MCU -->
<#assign CondMcu_STM32F030xxx = (McuName?? && McuName?matches("STM32F030.*"))>
<#-- Condition for STM32F031xxx MCU -->
<#assign CondMcu_STM32F031xxx = (McuName?? && McuName?matches("STM32F031.*"))>
<#-- Condition for STM32F446xCx or STM32F446xEx -->
<#assign CondMcu_STM32F446xCEx = (McuName?? && McuName?matches("STM32F446.(C|E).*"))>
<#-- Condition for STM32F0 Family -->
<#assign CondFamily_STM32F0 = (FamilyName?? && FamilyName=="STM32F0")>
<#-- Condition for STM32G0 Family -->
<#assign CondFamily_STM32G0 = (FamilyName?? && FamilyName=="STM32G0") >
<#-- Condition for STM32C0 Family -->
<#assign CondFamily_STM32C0 = (FamilyName?? && FamilyName=="STM32C0") >
<#-- Condition for STM32F3 Family -->
<#assign CondFamily_STM32F3 = (FamilyName?? && FamilyName == "STM32F3")>
<#-- Condition for STM32F4 Family -->
<#assign CondFamily_STM32F4 = (FamilyName?? && FamilyName == "STM32F4")>
<#-- Condition for STM32G4 Family -->
<#assign CondFamily_STM32G4 = (FamilyName?? && FamilyName == "STM32G4") >
<#-- Condition for STM32L4 Family -->
<#assign CondFamily_STM32L4 = (FamilyName?? && FamilyName == "STM32L4") >
<#-- Condition for STM32F7 Family -->
<#assign CondFamily_STM32F7 = (FamilyName?? && FamilyName == "STM32F7") >
<#-- Condition for STM32H5 Family -->
<#assign CondFamily_STM32H5 = (FamilyName?? && FamilyName == "STM32H5") >
<#-- Condition for STM32H7 Family -->
<#assign CondFamily_STM32H7 = (FamilyName?? && FamilyName == "STM32H7") >

<#assign CondIncludeMCConfig = (MC.M1_POTENTIOMETER_ENABLE == true) || (MC.M2_POTENTIOMETER_ENABLE == true) || (MC.ESC_ENABLE == true) >
<#assign CondBootHookUsed = (MC.M1_POTENTIOMETER_ENABLE == true) || (MC.M2_POTENTIOMETER_ENABLE == true) || (MC.ESC_ENABLE == true) >
<#assign CondPostMediumFrequencyHookM1Used = (MC.M1_POTENTIOMETER_ENABLE == true) || (MC.ESC_ENABLE == true) >
<#assign CondPostMediumFrequencyHookM2Used = (MC.M2_POTENTIOMETER_ENABLE == true) >

<#assign UNALIGNMENT_SUPPORTED = !(CondFamily_STM32F0 || CondFamily_STM32G0 || CondFamily_STM32C0)>
<#assign DWT_CYCCNT_SUPPORTED = !(CondFamily_STM32F0 || CondFamily_STM32G0 || CondFamily_STM32C0)>
<#assign MCP_DMA = !CondFamily_STM32C0 || MC.M1_CURRENT_SENSING_TOPO != "SINGLE_SHUNT_PHASE_SHIFT">
<#assign DMAStream = CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32H7 >
<#assign MEMORY_FOOTPRINT_REG = !(MC.LOW_MEMORY_FOOTPRINT_REG)>
<#assign MEMORY_FOOTPRINT_REG2 = !(MC.LOW_MEMORY_FOOTPRINT_REG2)>

<#if CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32H7>
<#assign DMA_TYPE = "STREAM">
<#assign DMA_MODE = "NORMAL">
<#else><#-- CondFamily_STM32F4 == false && CondFamily_STM32F7 == false && CondFamily_STM32H7 == false -->
<#assign DMA_TYPE = "CHANNEL">
<#assign DMA_MODE = "CIRCULAR">
</#if><#-- CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32H7 -->

<#if CondFamily_STM32F3 || CondFamily_STM32F4 || CondFamily_STM32G4>
<#assign IS_DUALDRIVE_CAPABLE = true>
<#else><#-- CondFamily_STM32F3 || CondFamily_STM32F4 || CondFamily_STM32G4 -->
<#assign IS_DUALDRIVE_CAPABLE = false>
</#if><#-- CondFamily_STM32F3 == false && CondFamily_STM32F4 == false && CondFamily_STM32G4 -->

<#-- Define some helper symbols -->
<#assign G4_Cut2_2_patch = CondFamily_STM32G4 >
<#assign NoInjectedChannel = (CondFamily_STM32F0 || CondFamily_STM32G0 || CondFamily_STM32C0 || G4_Cut2_2_patch ) >

<#if MC.M1_CURRENT_SENSING_TOPO=="THREE_SHUNT">
<#assign currentSuffix = "Curr">
<#else>
<#assign currentSuffix = "">
</#if>

<#if CondFamily_STM32F3>
  <#assign FIRST_TRIG_CH = "3" >
  <#assign SEC_TRIG_CH = "1" >
  <#assign DUTY_CH = "1" >
  <#assign TIM_TRIG = "TIMx_2" >
  <#assign TIM_AC_MAIN = "TIMx_2" >
  <#assign CH_AC_MAIN = "2" >  
</#if>

<#if CondFamily_STM32F4>
  <#if MC.M1_PWM_TIMER_SELECTION == "PWM_TIM1">
<#assign Stream   = "5"> 
  </#if>
  <#if MC.M1_PWM_TIMER_SELECTION == "PWM_TIM8">
<#assign Stream   = "1">   
  </#if>
  <#if MC.M2_PWM_TIMER_SELECTION == "PWM_TIM1">
<#assign Stream2   = "5"> 
  </#if>
  <#if MC.M2_PWM_TIMER_SELECTION == "PWM_TIM8">
<#assign Stream2   = "1">   
  </#if>
</#if><#-- CondFamily_STM32F4 -->

<#if CondFamily_STM32H5>
  <#if MC.M1_PWM_TIMER_SELECTION == "PWM_TIM1">
<#assign Stream   = "5"> 
  </#if>
  <#if MC.M2_PWM_TIMER_SELECTION == "PWM_TIM1">
<#assign Stream2   = "5"> 
  </#if>
</#if><#-- CondFamily_STM32H5 -->