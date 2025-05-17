<#function _remove_last_char text><#return text[0..text?length-2]></#function>
<#function _last_char text><#return text[text?length-1]></#function>
<#function _last_word text sep="_"><#return text?split(sep)?last></#function>
<#function _first_word text sep="_"><#return text?split(sep)?first></#function>

<#function Fx_Freq_Scaling pwm_freq>
<#list [ 1 , 2 , 4 , 8 , 16 ] as scaling>
       <#if (pwm_freq/scaling) < 65536 >
            <#return scaling >
        </#if>
    </#list>
    <#return 1 >
</#function>

<#-- PWM Timer used to extract driver in order to simplify logic for Hall/Encoder IC_Filter determination -->
<#assign DRIVER = configs[0].peripheralParams.get(_last_word(MC.M1_PWM_TIMER_SELECTION)).driver>

<#function GetSpeedSensor motor aux=false> 
      <#local SENSORS =
        [ {"name": "NULL", "val": "ENO_SENSOR"} 
        , {"name": "PLL", "val": "EPLL"} 
        , {"name": "CORDIC", "val": "ECORDIC"} 
        , {"name": "ENCODER", "val": "EENCODER"}
        , {"name": "HALL", "val": "EHALL"}
        , {"name": "HSO", "val": "EHSO"}        
        , {"name": "ZEST", "val": "EZEST"}
        ] >
      <#if motor == "M1" > 
        <#if !aux > <#assign entry = MC.M1_SPEED_SENSOR> <#else> 
        <#assign entry = MC.AUXILIARY_SPEED_MEASUREMENT_SELECTION > </#if>
      <#else>
        <#if !aux > <#assign entry = MC.M2_SPEED_SENSOR> <#else> 
        <#assign entry = MC.AUXILIARY_SPEED_MEASUREMENT_SELECTION2 > </#if>
      </#if>
      <#list SENSORS as sensor >
        <#if entry?contains(sensor.name) >
         <#return  sensor.val >
        </#if>
      </#list>
     <#return 0 >
</#function>

<#function GetTopology motor> 
      <#local TOPOLOGIES =
        [ {"name": "THREE_SHUNT", "val": 0} 
        , {"name": "SINGLE_SHUNT_PHASE_SHIFT", "val": 2}
        , {"name": "SINGLE_SHUNT_ACTIVE_WIN", "val": 1} 
        , {"name": "ICS_SENSORS", "val": 3}
        , {"name": "NONE", "val": 4}
        ] >
      <#if motor == "M1" > 
        <#assign entry = MC.M1_CURRENT_SENSING_TOPO > 
      <#else>
        <#assign entry = MC.M2_CURRENT_SENSING_TOPO >
      </#if>
      <#list TOPOLOGIES as topology >
        <#if entry?contains(topology.name) >
         <#return  topology.val >
        </#if>
      </#list>
     <#return 0 >
</#function>

<#function GetConfigurationFlag motor Flag>
  <#local result =""> 
  <#if Flag == 1 >
    <#if motor == "M1" >
      <#if MC.M1_FLUX_WEAKENING_ENABLING > <#local result+="|FLUX_WEAKENING_FLAG"> </#if>
      <#if MC.M1_FEED_FORWARD_CURRENT_REG_ENABLING > <#local result+="|FEED_FORWARD_FLAG"> </#if>
      <#if MC.M1_MTPA_ENABLING > <#local result+="|MTPA_FLAG"> </#if>
      <#if MC.PFC_ENABLED >   <#local result+="|PFC_FLAG">  </#if>
      <#if MC.M1_ICL_ENABLED >   <#local result+="|ICL_FLAG"> </#if>  
      <#if MC.M1_ON_OVER_VOLTAGE == "TURN_ON_R_BRAKE" >  <#local result+="|RESISTIVE_BREAK_FLAG"> </#if>  
      <#if MC.M1_HW_OV_CURRENT_PROT_BYPASS >  <#local result+="|OCP_DISABLE_FLAG"> </#if>  
      <#if MC.USE_STGAP1S >  <#local result+="|STGAP_FLAG"> </#if>  
      <#if MC.M1_POSITION_CTRL_ENABLING >  <#local result+="|POSITION_CTRL_FLAG"> </#if>  
      <#if MC.M1_BUS_VOLTAGE_READING >  <#local result+="|VBUS_SENSING_FLAG"> </#if>  
      <#if MC.M1_TEMPERATURE_READING >  <#local result+="|TEMP_SENSING_FLAG"> </#if>  
    /* Voltage sensing to be defined in data model*/
    /* Flash Config to be defined in data model*/
      <#if MC.DEBUG_DAC_CH1_EN >  <#local result+="|DAC_CH1_FLAG"> </#if>  
      <#if MC.DEBUG_DAC_CH2_EN >  <#local result+="|DAC_CH2_FLAG"> </#if>
      <#if MC.M1_OTF_STARTUP >  <#local result+="|OTF_STARTUP_FLAG"> </#if>    
    <#else> 
      <#if MC.M2_FLUX_WEAKENING_ENABLING > <#local result+="|FLUX_WEAKENING_FLAG"> </#if>
      <#if MC.M2_FEED_FORWARD_CURRENT_REG_ENABLING > <#local result+="|FEED_FORWARD_FLAG"> </#if>
      <#if MC.M2_MTPA_ENABLING > <#local result+="|MTPA_FLAG"> </#if>
      <#if MC.PFC_ENABLED >   <#local result+="|PFC_FLAG">  </#if>
      <#if MC.M2_ICL_ENABLED >   <#local result+="|ICL_FLAG"> </#if>  
      <#if MC.M2_ON_OVER_VOLTAGE == "TURN_ON_R_BRAKE" >  <#local result+="|RESISTIVE_BREAK_FLAG"> </#if>  
      <#if MC.M2_HW_OV_CURRENT_PROT_BYPASS >  <#local result+="|OCP_DISABLE_FLAG"> </#if>  
      <#if MC.USE_STGAP1S >  <#local result+="|STGAP_FLAG"> </#if>  
      <#if MC.M2_POSITION_CTRL_ENABLING >  <#local result+="|POSITION_CTRL_FLAG"> </#if>  
      <#if MC.M2_BUS_VOLTAGE_READING >  <#local result+="|VBUS_SENSING_FLAG"> </#if>  
      <#if MC.M2_TEMPERATURE_READING >  <#local result+="|TEMP_SENSING_FLAG"> </#if>  
      <#if MC.DEBUG_DAC_CH1_EN >  <#local result+="|DAC_CH1_FLAG"> </#if>  
      <#if MC.DEBUG_DAC_CH2_EN >  <#local result+="|DAC_CH2_FLAG"> </#if>
      <#if MC.M2_OTF_STARTUP >  <#local result+="|OTF_STARTUP_FLAG"> </#if> 
    </#if>
  <#elseif Flag == 2 >
    <#if motor == "M1" >  
      <#if MC.M1_OVERMODULATION >  <#local result+="|OVERMODULATION_FLAG"> </#if> 
      <#if MC.M1_DISCONTINUOUS_PWM > <#local result+="|DISCONTINUOUS_PWM_FLAG"> </#if>
      <#if MC.MOTOR_PROFILER==true> <#local result+="|PROFILER_FLAG"> </#if>
      <#if MC.DBG_MCU_LOAD_MEASURE > <#local result+="|DBG_MCU_LOAD_MEASURE_FLAG"> </#if>
      <#if MC.M1_DBG_OPEN_LOOP_ENABLE >   <#local result+="|DBG_OPEN_LOOP_FLAG">  </#if>    
    <#else> 
      <#if MC.M2_OVERMODULATION >  <#local result+="|OVERMODULATION_FLAG"> </#if> 
      <#if MC.M2_DISCONTINUOUS_PWM > <#local result+="|DISCONTINUOUS_PWM_FLAG"> </#if>
      <#if MC.DBG_MCU_LOAD_MEASURE > <#local result+="|DBG_MCU_LOAD_MEASURE_FLAG"> </#if>
      <#if MC.M2_DBG_OPEN_LOOP_ENABLE >   <#local result+="|DBG_OPEN_LOOP_FLAG">  </#if> 
    </#if>
  </#if> <#-- Flag number -->
  <#if result =="">
    <#return "0U" >
  <#else>
    <#return result[1..] >
   </#if>
</#function>

<#if CondFamily_STM32G0>
<#-- 
Useful for debug
      <#list NVICParams.keySet() as nvicParam>
          ${nvicParam}
      </#list>
-->
</#if><#-- CondFamily_STM32G0 -->

<#if CondFamily_STM32C0 || CondFamily_STM32F0 || CondFamily_STM32G0>
<#function EXT_IRQHandler line>
    <#local EXTI_IRQ =
        [ {"name": "EXTI0_1_IRQHandler", "line": 1} 
        , {"name": "EXTI2_3_IRQHandler", "line": 3} 
        , {"name": "EXTI4_15_IRQHandler", "line": 15}
        ] >
    <#list EXTI_IRQ as handler >
        <#if line <= (handler.line) >
           <#return  handler.name >
         </#if>
    </#list>
     <#return "EXTI4_15_IRQHandler" >
</#function>
  <#if CondFamily_STM32C0 || CondFamily_STM32G0>
<#assign NVICParams = configs[0].peripheralNVICParams >

<#function IRQHandler_name component irq="IRQ">
    <#if NVICParams.containsKey(component) >
    <#assign IRQHandler = NVICParams.get(component)>
    <#else>  
    <#return "#error component ${component} not found at NVIC level (no IT activated) " >
    </#if>
    <#if IRQHandler.isEmpty() == true >
      <#return "IRQ ${irq} probably not activated at NVIC level" >
    </#if>
  <#list IRQHandler.keySet() as entry>
       <#if entry?contains(irq)>     
         <#return "void "+entry[0..entry.length()-2]+"Handler(void)">
       </#if>
    </#list>
  <#return "#error IRQ ${irq} not activated at NVIC level for component ${component}">
</#function>
  </#if><#-- CondFamily_STM32C0 || CondFamily_STM32G0 -->
</#if><#-- CondFamily_STM32C0 || CondFamily_STM32F0 || CondFamily_STM32G0 -->

<#if CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32F3 || CondFamily_STM32G4 || CondFamily_STM32H7 || CondFamily_STM32L4>
  <#function EXT_IRQHandler line>
  <#if CondFamily_STM32F3>
  <#local EXTI_IRQ =
        [ {"name": "EXTI0_IRQHandler", "line": 0} 
        , {"name": "EXTI1_IRQHandler", "line": 1}
        , {"name": "EXTI2_TSC_IRQHandler", "line": 2}
        , {"name": "EXTI3_IRQHandler", "line": 3} 
        , {"name": "EXTI4_IRQHandler", "line": 4} 
        , {"name": "EXTI9_5_IRQHandler", "line": 9}
        , {"name": "EXTI15_10_IRQHandler", "line": 15}
        ] >
  <#else>
  <#local EXTI_IRQ =
        [ {"name": "EXTI0_IRQHandler", "line": 0} 
        , {"name": "EXTI1_IRQHandler", "line": 1}
        , {"name": "EXTI2_IRQHandler", "line": 2}
        , {"name": "EXTI3_IRQHandler", "line": 3} 
        , {"name": "EXTI4_IRQHandler", "line": 4} 
        , {"name": "EXTI9_5_IRQHandler", "line": 9}
        , {"name": "EXTI15_10_IRQHandler", "line": 15}
        ] >
  </#if><#-- CondFamily_STM32F3 -->
  <#list EXTI_IRQ as handler >
        <#if line <= (handler.line) >
           <#return  handler.name >
         </#if>
  </#list>
  <#return "EXTI15_10_IRQHandler" >
  </#function>
</#if><#-- CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32F3 || CondFamily_STM32G4 || CondFamily_STM32H7 || CondFamily_STM32L4 -->

<#if CondFamily_STM32H5>
<#function EXT_IRQHandler line>
  <#return "EXTI"+line+"_IRQHandler" >
</#function>
</#if><#-- CondFamily_STM32H5 -->

<#if CondFamily_STM32L4>
  <#if _last_word(MC.M1_PWM_TIMER_SELECTION) == "TIM1"  >
<#assign DMA_nb = "1">
<#assign DMA_channel = "4">
  <#elseif _last_word(MC.M1_PWM_TIMER_SELECTION) == "TIM8"  >
<#assign DMA_nb = "2">
<#assign DMA_channel = "2">
  </#if>
</#if><#-- CondFamily_STM32L4 -->

<#function TimerHandler timer><#return "${timer}_IRQHandler"></#function>
<#function Fx_TIM_Div TimFreq PWMFreq>
    <#list [1,2,3,4,5] as divider>
    <#-- (PWM_PERIOD_CYCLES*SQRT3FACTOR)/16384u must feet in 16 bits with SQRT3FACTOR=0xDDB4 -->
       <#if (TimFreq/(PWMFreq*divider)*56756)/16384 < 65535 >
            <#return divider >
        </#if>
    </#list>
    <#return 1 >
</#function>

<#function Fx_TIM_Div_hso TimFreq PWMFreq>
    <#list [1,2,3,4,5] as divider>
    <#-- (PWM_PERIOD_CYCLES*SQRT3FACTOR)/16384u must feet in 16 bits with SQRT3FACTOR=0xDDB4 -->
       <#if (TimFreq/(PWMFreq*divider))/2 < 65535 >
            <#return divider >
        </#if>
    </#list>
    <#return 1 >
</#function>
<#assign SYSCLKFreq = RCC.get("SYSCLKFreq_VALUE")?number>

<#if CondFamily_STM32F4 || CondFamily_STM32H5>
<#assign TimerDivider = Fx_TIM_Div(SYSCLKFreq,(MC.M1_PWM_FREQUENCY)?number) >
<#assign TimerDivider2 = Fx_TIM_Div(SYSCLKFreq,(MC.M2_PWM_FREQUENCY)?number) >
</#if><#-- CondFamily_STM32F4 || CondFamily_STM32H5 -->

<#if CondFamily_STM32F3>
<#assign TimerFreq = _last_word(MC.M1_PWM_TIMER_SELECTION)+"Freq_Value" >
<#assign ADV_TIM_CLKFreq = RCC.get(TimerFreq)?number>
<#assign TimerDivider = Fx_TIM_Div(ADV_TIM_CLKFreq,(MC.M1_PWM_FREQUENCY)?number) >
  <#if MC.DRIVE_NUMBER != "1">
<#assign TimerFreq2 = _last_word(MC.M2_PWM_TIMER_SELECTION)+"Freq_Value" >
<#assign ADV_TIM_CLKFreq2 = RCC.get(TimerFreq2)?number>
<#assign TimerDivider2 = Fx_TIM_Div(ADV_TIM_CLKFreq2,(MC.M2_PWM_FREQUENCY)?number) >
  </#if>
</#if><#-- CondFamily_STM32F3 -->

<#if CondFamily_STM32G4>
<#assign ADV_TIM_CLKFreq = RCC.get("APB2TimFreq_Value")?number> <#-- Advanced timers are all on APB2 -->
<#assign ADV_TIM_CLKFreq2 = RCC.get("APB2TimFreq_Value")?number>
  <#if MC.M1_SPEED_SENSOR == "HSO" || MC.M1_SPEED_SENSOR == "ZEST">
    <#assign TimerDivider = Fx_TIM_Div_hso(ADV_TIM_CLKFreq,(MC.M1_PWM_FREQUENCY)?number) >
    <#assign TimerDivider2 = Fx_TIM_Div_hso(ADV_TIM_CLKFreq2,(MC.M2_PWM_FREQUENCY)?number) >
  <#else>
<#assign TimerDivider = Fx_TIM_Div(ADV_TIM_CLKFreq,(MC.M1_PWM_FREQUENCY)?number) >
<#assign TimerDivider2 = Fx_TIM_Div(ADV_TIM_CLKFreq2,(MC.M2_PWM_FREQUENCY)?number) >
  </#if><#-- MC.M1_SPEED_SENSOR == "HSO" || MC.M1_SPEED_SENSOR == "ZEST" -->
</#if><#-- CondFamily_STM32G4 -->

<#if CondFamily_STM32G0>
<#assign ADV_TIM_CLKFreq = RCC.get("TIM1Freq_Value")?number> <#-- TIM1 only candidate, can be hardcoded -->
<#assign TimerDivider = Fx_TIM_Div(ADV_TIM_CLKFreq,(MC.M1_PWM_FREQUENCY)?number) >
</#if><#-- CondFamily_STM32G0 -->

<#if CondFamily_STM32H7 >
<#assign ADV_TIM_CLKFreq = RCC.get("APB2Freq_Value")?number>
<#assign ADC_CLKFreq = RCC.get("ADCFreq_Value")?number>
<#assign APB1Tim_CLKFreq = RCC.get("APB1Freq_Value")?number>
</#if><#-- CondFamily_STM32H7 -->

<#if CondFamily_STM32L4 || CondFamily_STM32H7 || CondFamily_STM32G4 || CondFamily_STM32F7 || CondFamily_STM32F3>
<#-- For futur use 
<#assign ADC12outputFreq ${RCC.get("ADC12outputFreq_Value")}>
<#assign ADC34outputFreq ${RCC.get("ADC34outputFreq_Value")}>
-->
</#if><#-- CondFamily_STM32L4 || CondFamily_STM32H7 || CondFamily_STM32G4 || CondFamily_STM32F7 || CondFamily_STM32F3-->

<#-- This table is an implementation of the Bits 7:4 IC1F register definition -->
<#function Fx_ic_filter icx mode>
    <#local coefficients =
        [ {"divider":  1, "N": 1} <#--   1 -->
        , {"divider":  1, "N": 2} <#--   2 -->
        , {"divider":  1, "N": 4} <#--   4 -->
        , {"divider":  1, "N": 8} <#--   8 -->

        , {"divider":  2, "N": 6} <#--  12 -->
        , {"divider":  2, "N": 8} <#--  16 -->

        , {"divider":  4, "N": 6} <#--  24 -->
        , {"divider":  4, "N": 8} <#--  32 -->

        , {"divider":  8, "N": 6} <#--  48 -->
        , {"divider":  8, "N": 8} <#--  64 -->

        , {"divider": 16, "N": 5} <#--  80 -->
        , {"divider": 16, "N": 6} <#--  96 -->
        , {"divider": 16, "N": 8} <#-- 128 -->

        , {"divider": 32, "N": 5} <#-- 160 -->
        , {"divider": 32, "N": 6} <#-- 192 -->
        , {"divider": 32, "N": 8} <#-- 256 -->
        ] >

    <#list coefficients as coeff >
        <#if icx <= (coeff.divider * coeff.N) >
          <#if mode == "coeffs">
            <#if coeff.divider != coeff.N>
              <#return coeff.divider+"_N"+coeff.N>
            <#else>
              <#return 1>
            </#if>
          <#elseif mode == "index">
            <#return coeff_index> 
          </#if>
        </#if>
    </#list>
    <#if mode == "coeffs">
      <#return "32_N8">
    <#elseif mode == "index">
      <#return 15 >
    </#if>
</#function>

<#macro define_IC_FILTER  motor sensor driver icx_filter>
#define M${ motor }_${ sensor }_IC_FILTER_LL LL_TIM_IC_FILTER_FDIV${Fx_ic_filter(icx_filter, "coeffs")}
    <#if driver == "HAL">
#define M${ motor }_${ sensor }_IC_FILTER   ${Fx_ic_filter(icx_filter, "index")}
    <#else>
#define M${ motor }_${ sensor }_IC_FILTER   LL_TIM_IC_FILTER_FDIV${Fx_ic_filter(icx_filter, "coeffs")}
    </#if>
</#macro>

<#if CondFamily_STM32C0 || CondFamily_STM32G0>
  <#if M1_ENCODER == true >
<#assign SensorTimer=_last_word(MC.M1_ENC_TIMER_SELECTION)>
  <#elseif M1_HALL_SENSOR == true>
<#assign SensorTimer=_last_word(MC.M1_HALL_TIMER_SELECTION)>
  </#if>
</#if>

<#if MC.DRIVE_NUMBER != "1">
  <#if MC.M1_PWM_FREQUENCY == MC.M2_PWM_FREQUENCY>
    <#assign FREQ_RATIO = "1">
    <#assign M1_FREQ_RELATION = "HIGHEST_FREQ">
    <#assign M2_FREQ_RELATION = "LOWEST_FREQ">
  <#elseif MC.M1_PWM_FREQUENCY?number gt MC.M2_PWM_FREQUENCY?number>
    <#assign FREQ_RATIO = MC.M1_PWM_FREQUENCY?number/MC.M2_PWM_FREQUENCY?number>
    <#assign M1_FREQ_RELATION = "HIGHEST_FREQ">
    <#assign M2_FREQ_RELATION = "LOWEST_FREQ">
  <#else>
    <#assign FREQ_RATIO = MC.M2_PWM_FREQUENCY?number/MC.M1_PWM_FREQUENCY?number>
    <#assign M2_FREQ_RELATION = "HIGHEST_FREQ">
    <#assign M1_FREQ_RELATION = "LOWEST_FREQ">
  </#if>
</#if><#-- MC.DRIVE_NUMBER != "1" -->