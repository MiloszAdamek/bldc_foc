<#assign PWM_Init = "PWMC_Init">
<#assign PWM_TurnOnLowSides = "PWMC_TurnOnLowSides">
<#assign PWM_SwitchOn = "PWMC_SwitchOnPWM">
<#assign PWM_SwitchOff = "PWMC_SwitchOffPWM">
<#assign PWM_GetPhaseCurrents ="">

<#if CondFamily_STM32F4 || CondFamily_STM32F7 >
<#assign LL_ADC_CYCLE_SUFFIX = 'CYCLES'>
<#else><#-- CondFamily_STM32F4 || CondFamily_STM32F7 -->
<#assign LL_ADC_CYCLE_SUFFIX = 'CYCLES_5'>
</#if><#-- CondFamily_STM32F4 || CondFamily_STM32F7 -->

<#if CondFamily_STM32F3 || CondFamily_STM32G4>
<#assign ADC1 = (MC.PHASE_U_BEMF_ADC == "ADC1" || MC.PHASE_V_BEMF_ADC == "ADC1" || MC.PHASE_W_BEMF_ADC == "ADC1")>
<#assign ADC2 = (MC.PHASE_U_BEMF_ADC == "ADC2" || MC.PHASE_V_BEMF_ADC == "ADC2" || MC.PHASE_W_BEMF_ADC == "ADC2")>
<#assign ADC3 = (MC.PHASE_U_BEMF_ADC == "ADC3" || MC.PHASE_V_BEMF_ADC == "ADC3" || MC.PHASE_W_BEMF_ADC == "ADC3")>
<#assign ADC4 = (MC.PHASE_U_BEMF_ADC == "ADC4" || MC.PHASE_V_BEMF_ADC == "ADC4" || MC.PHASE_W_BEMF_ADC == "ADC4")>
</#if><#-- CondFamily_STM32F3 -->

<#if MC.M1_PWM_DRIVER_PN != "STSPIN32G4">
  <#if configs[0].peripheralParams.get(_last_word(MC.M1_PWM_TIMER_SELECTION))??>
<#assign PWMTIM = configs[0].peripheralParams.get(_last_word(MC.M1_PWM_TIMER_SELECTION))>
  </#if><#-- configs[0].peripheralParams.get(_last_word(MC.M1_PWM_TIMER_SELECTION))?? -->
  <#if !PWMTIM??>
#error SORRY, it didn't work
  </#if><#-- !PWMTIM?? -->
</#if><#-- MC.M1_PWM_DRIVER_PN != "STSPIN32G4" -->