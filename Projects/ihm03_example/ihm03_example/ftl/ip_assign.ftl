<#if  AUX_SPEED_FDBK_M1 == true>
  <#if   (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_PLL") == true>
    <#assign SPD_AUX_SUPER_M1 = "&STO_PLL_M1._Super">
  <#elseif  (MC.M1_AUXILIARY_SPEED_SENSOR == "STO_CORDIC") == true>
    <#assign SPD_AUX_SUPER_M1 = "&STO_CR_M1._Super">
  <#elseif  MC.M1_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR">
    <#assign SPD_AUX_SUPER_M1 = "&HALL_M1._Super">
  <#elseif  (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
    <#assign SPD_AUX_SUPER_M1 = "&ENCODER_M1._Super">
  </#if>
</#if>
<#if  AUX_SPEED_FDBK_M2 == true>
  <#if   (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_PLL") == true>
    <#assign SPD_AUX_SUPER_M2 = "&STO_PLL_M2._Super">
  <#elseif  (MC.M2_AUXILIARY_SPEED_SENSOR == "STO_CORDIC") == true>
    <#assign SPD_AUX_SUPER_M2 = "&STO_CR_M2._Super">
  <#elseif  MC.M2_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR">
    <#assign SPD_AUX_SUPER_M2 = "&HALL_M2._Super">
  <#elseif  (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
    <#assign SPD_AUX_SUPER_M2 = "&ENCODER_M2._Super">
  </#if>
</#if>

<#if MC.M1_OVERMODULATION == true><#assign OVM ="_OVM"><#else><#assign OVM =""></#if>
<#if MC.M2_OVERMODULATION == true><#assign OVM2 ="_OVM"><#else><#assign OVM2 =""></#if>

<#-- Motor 1 Tables -->
<#assign M1_OPAMPInputMapp_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W}
  ]>
 <#assign M1_ADCConfig_2OPAMPs_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#assign M1_OPAMPInputMapp_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M1_ADCConfig_2OPAMPs_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#assign M1_OPAMPInputMapp_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M1_ADCConfig_2OPAMPs_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U}
  ]>

<#assign M1_OPAMPInputMapp_3_OPAMPS =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_OPAMP_U+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_OPAMP_V+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M1_CS_OPAMP_W+"_NonInvertingInput_"+MC.M1_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M1_ADCConfig_3ADCS =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#assign M1_ADCConfig_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_SHARED,
                  "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_SHARED,
                  "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#assign M1_ADCConfig_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U,
                  "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_SHARED},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U,
                  "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_SHARED},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_W+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#assign M1_ADCConfig_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U,
                  "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_SHARED},
    {"Sector": 3, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U,
                  "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_SHARED},
    {"Sector": 4, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_U, "PHASE_2": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M1_CS_ADC_V+"_"+MC.M1_CS_CHANNEL_V, "PHASE_2": MC.M1_CS_ADC_U+"_"+MC.M1_CS_CHANNEL_W}
  ]>

<#-- Motor 2 Tables -->
<#assign M2_OPAMPInputMapp_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W}
  ]>
 <#assign M2_ADCConfig_2OPAMPs_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M2_OPAMPInputMapp_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M2_ADCConfig_2OPAMPs_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M2_OPAMPInputMapp_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M2_ADCConfig_2OPAMPs_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U}
  ]>

<#assign M2_OPAMPInputMapp_3_OPAMPS =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_OPAMP_U+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_U,
                  "PHASE_2": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_OPAMP_V+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_V,
                  "PHASE_2": MC.M2_CS_OPAMP_W+"_NonInvertingInput_"+MC.M2_CS_OPAMP_VPSEL_W}
  ]>

 <#assign M2_ADCConfig_3ADCS =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M2_ADCConfig_Shared_U =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_SHARED,
                  "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_SHARED,
                  "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M2_ADCConfig_Shared_V =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U,
                  "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_SHARED},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U,
                  "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_SHARED},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_W+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M2_ADCConfig_Shared_W =
  [ {"Sector": 1, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_W},
    {"Sector": 2, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U,
                  "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_SHARED},
    {"Sector": 3, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U,
                  "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_SHARED},
    {"Sector": 4, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 5, "PHASE_1": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_U, "PHASE_2": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V},
    {"Sector": 6, "PHASE_1": MC.M2_CS_ADC_V+"_"+MC.M2_CS_CHANNEL_V, "PHASE_2": MC.M2_CS_ADC_U+"_"+MC.M2_CS_CHANNEL_W}
  ]>

<#assign M1_OPAMPip = {"U":MC.M1_CS_OPAMP_U ,"V":MC.M1_CS_OPAMP_V ,"W":MC.M1_CS_OPAMP_W }>
<#assign M2_OPAMPip = {"U":MC.M2_CS_OPAMP_U ,"V":MC.M2_CS_OPAMP_V ,"W":MC.M2_CS_OPAMP_W }>

<#assign G4_ADC_Channel_Table =
  [ {"VPOPAMP": "VPOPAMP1", "ADC": "ADC1", "CHANNEL": 13},
    {"VPOPAMP": "VPOPAMP2", "ADC": "ADC2", "CHANNEL": 16},
    {"VPOPAMP": "VPOPAMP3", "ADC": "ADC2", "CHANNEL": 18},
    {"VPOPAMP": "VPOPAMP3", "ADC": "ADC3", "CHANNEL": 13},
    {"VPOPAMP": "VPOPAMP6", "ADC": "ADC4", "CHANNEL": 17},
    {"VPOPAMP": "VPOPAMP5", "ADC": "ADC5", "CHANNEL": 3},
    {"VPOPAMP": "VPOPAMP4", "ADC": "ADC5", "CHANNEL": 5}
  ]>

  <#if  MC.M1_SPEED_SENSOR == "STO_PLL">
    <#assign SPD_M1   = "&STO_PLL_M1">
    <#assign SPD_init_M1 = "STO_PLL_Init" >
    <#assign SPD_calcAvrgMecSpeedUnit_M1 = "STO_PLL_CalcAvrgMecSpeedUnit" >
    <#assign SPD_calcElAngle_M1 = "STO_PLL_CalcElAngle" >
    <#assign SPD_calcAvergElSpeedDpp_M1 = "STO_PLL_CalcAvrgElSpeedDpp">
    <#assign SPD_clear_M1 = "STO_PLL_Clear">
  <#elseif  MC.M1_SPEED_SENSOR == "STO_CORDIC">
    <#assign SPD_M1 = "&STO_CR_M1" >
    <#assign SPD_init_M1 = "STO_CR_Init" >
    <#assign SPD_calcElAngle_M1 = "STO_CR_CalcElAngle" >
    <#assign SPD_calcAvrgMecSpeedUnit_M1 = "STO_CR_CalcAvrgMecSpeedUnit" >
    <#assign SPD_calcAvergElSpeedDpp_M1 = "STO_CR_CalcAvrgElSpeedDpp">
    <#assign SPD_clear_M1 = "STO_CR_Clear">
  <#elseif  MC.M1_SPEED_SENSOR == "HALL_SENSOR">
    <#assign SPD_M1 = "&HALL_M1" >
    <#assign SPD_init_M1 = "HALL_Init" >
    <#assign SPD_calcElAngle_M1 = "HALL_CalcElAngle" >
    <#assign SPD_calcAvrgMecSpeedUnit_M1 = "HALL_CalcAvrgMecSpeedUnit" >
    <#assign SPD_clear_M1 = "HALL_Clear">
  <#elseif (MC.M1_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_SPEED_SENSOR == "QUAD_ENCODER_Z")>
    <#assign SPD_M1 = "&ENCODER_M1" >
    <#assign SPD_init_M1 = "ENC_Init" >
    <#assign SPD_calcElAngle_M1 = "ENC_CalcElAngle" >
    <#assign SPD_calcAvrgMecSpeedUnit_M1 = "ENC_CalcAvrgMecSpeedUnit" >
    <#assign SPD_clear_M1 = "ENC_Clear">
  <#elseif  MC.M1_SPEED_SENSOR == "SENSORLESS_ADC"  >
    <#assign SPD_M1 = "&Bemf_ADC_M1" >
    <#assign SPD_init_M1 = "BADC_Init" >
    <#assign SPD_calcElAngle_M1 = "BADC_CalcElAngle" >
    <#assign SPD_calcAvrgMecSpeedUnit_M1 = "BADC_CalcAvrgMecSpeedUnit" >
    <#assign SPD_clear_M1 = "BADC_Clear">
  </#if>
  <#if  AUX_SPEED_FDBK_M1 == true>
    <#if   MC.M1_AUXILIARY_SPEED_SENSOR == "STO_PLL">
      <#assign SPD_AUX_M1 = "&STO_PLL_M1">
      <#assign SPD_aux_init_M1 = "STO_PLL_Init">
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M1 ="STO_PLL_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_calcAvrgElSpeedDpp_M1 = "STO_PLL_CalcAvrgElSpeedDpp">
      <#assign SPD_aux_calcElAngle_M1 = "STO_PLL_CalcElAngle">
      <#assign SPD_aux_clear_M1 = "STO_PLL_Clear">
    <#elseif MC.M1_AUXILIARY_SPEED_SENSOR == "STO_CORDIC">
      <#assign SPD_AUX_M1 = "&STO_CR_M1">
      <#assign SPD_aux_init_M1 = "STO_CR_Init">
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M1 = "STO_CR_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_calcAvrgElSpeedDpp_M1 = "STO_CR_CalcAvrgElSpeedDpp">
      <#assign SPD_aux_calcElAngle_M1 = "STO_CR_CalcElAngle">
      <#assign SPD_aux_clear_M1 = "STO_CR_Clear">
    <#elseif  MC.M1_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR">
      <#assign SPD_AUX_M1 = "&HALL_M1">
      <#assign SPD_aux_init_M1 = "HALL_Init" >
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M1 = "HALL_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_clear_M1 = "HALL_Clear">
    <#elseif (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M1_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
      <#assign SPD_AUX_M1 = "&ENCODER_M1">
      <#assign SPD_aux_init_M1 = "ENC_Init" >
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M1 = "ENC_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_clear_M1 = "ENC_Clear">
    </#if>
  </#if>
    <#if  MC.M2_SPEED_SENSOR == "STO_PLL">
      <#assign SPD_M2   = "&STO_PLL_M2">
      <#assign SPD_init_M2 = "STO_PLL_Init" >
      <#assign SPD_calcAvrgMecSpeedUnit_M2 = "STO_PLL_CalcAvrgMecSpeedUnit" >
      <#assign SPD_calcElAngle_M2 = "STO_PLL_CalcElAngle" >   /* if not sensorless then 2nd parameter is MC_NULL*/
      <#assign SPD_calcAvergElSpeedDpp_M2 = "STO_PLL_CalcAvrgElSpeedDpp">
      <#assign SPD_clear_M2 = "STO_PLL_Clear">
    <#elseif  MC.M2_SPEED_SENSOR == "STO_CORDIC">
      <#assign SPD_M2 = "&STO_CR_M2" >
      <#assign SPD_init_M2 = "STO_CR_Init" >
      <#assign SPD_calcElAngle_M2 = "STO_CR_CalcElAngle" >
      <#assign SPD_calcAvrgMecSpeedUnit_M2 = "STO_CR_CalcAvrgMecSpeedUnit" >
      <#assign SPD_calcAvergElSpeedDpp_M2 = "STO_CR_CalcAvrgElSpeedDpp">
      <#assign SPD_clear_M2 = "STO_CR_Clear">
    <#elseif  MC.M2_SPEED_SENSOR == "HALL_SENSOR">
      <#assign SPD_M2 = "&HALL_M2" >
      <#assign SPD_init_M2 = "HALL_Init" >
      <#assign SPD_calcElAngle_M2 = "HALL_CalcElAngle" >
      <#assign SPD_calcAvrgMecSpeedUnit_M2 = "HALL_CalcAvrgMecSpeedUnit" >
      <#assign SPD_clear_M2 = "HALL_Clear">
    <#elseif  (MC.M2_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_SPEED_SENSOR == "QUAD_ENCODER_Z")>
      <#assign SPD_M2 = "&ENCODER_M2" >
      <#assign SPD_init_M2 = "ENC_Init" >
      <#assign SPD_calcElAngle_M2 = "ENC_CalcElAngle" >
      <#assign SPD_calcAvrgMecSpeedUnit_M2 = "ENC_CalcAvrgMecSpeedUnit" >
      <#assign SPD_clear_M2 = "ENC_Clear">
    </#if>
  <#if  AUX_SPEED_FDBK_M2 == true>
    <#if   MC.M2_AUXILIARY_SPEED_SENSOR == "STO_PLL">
      <#assign SPD_AUX_M2 = "&STO_PLL_M2">
      <#assign SPD_aux_init_M2 = "STO_PLL_Init" >
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M2 ="STO_PLL_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_calcAvrgElSpeedDpp_M2 = "STO_PLL_CalcAvrgElSpeedDpp">
      <#assign SPD_aux_calcElAngle_M2 = "STO_PLL_CalcElAngle">
      <#assign SPD_aux_clear_M2 = "STO_PLL_Clear">
    <#elseif  MC.M2_AUXILIARY_SPEED_SENSOR == "STO_CORDIC">
      <#assign SPD_AUX_M2 = "&STO_CR_M2">
      <#assign SPD_aux_init_M2 = "STO_CR_Init" >
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M2 = "STO_CR_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_calcAvrgElSpeedDpp_M2 = "STO_CR_CalcAvrgElSpeedDpp">
      <#assign SPD_aux_calcElAngle_M2 = "STO_CR_CalcElAngle">
      <#assign SPD_aux_clear_M2 = "STO_CR_Clear">
  <#elseif  MC.M2_AUXILIARY_SPEED_SENSOR == "HALL_SENSOR">
      <#assign SPD_AUX_M2 = "&HALL_M2">
      <#assign SPD_aux_init_M2 = "HALL_Init" >
      <#assign SPD_aux_calcElAngle_M2 = "HALL_CalcElAngle">
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M2 = "HALL_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_clear_M2 = "HALL_Clear">
    <#elseif  (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER") || (MC.M2_AUXILIARY_SPEED_SENSOR == "QUAD_ENCODER_Z")>
      <#assign SPD_AUX_M2 = "&ENCODER_M2">
      <#assign SPD_aux_init_M2 = "ENC_Init" >
      <#assign SPD_aux_calcElAngle_M2 = "ENC_CalcElAngle">
      <#assign SPD_aux_calcAvrgMecSpeedUnit_M2 = "ENC_CalcAvrgMecSpeedUnit">
      <#assign SPD_aux_clear_M2 = "ENC_Clear">
    </#if>
  </#if>
<#if MC.M1_CURRENT_SENSING_TOPO=='ICS_SENSORS'>
  <#assign M1_CS_TOPO = "ICS_">
<#elseif (MC.M1_CURRENT_SENSING_TOPO=='SINGLE_SHUNT_PHASE_SHIFT') || (MC.M1_CURRENT_SENSING_TOPO=='SINGLE_SHUNT_ACTIVE_WIN')>
  <#assign M1_CS_TOPO = "R1_">
<#elseif (MC.M1_CURRENT_SENSING_TOPO=='THREE_SHUNT')>
  <#assign M1_CS_TOPO = "R3_"+MC.M1_CS_ADC_NUM+"_">
</#if>
<#if MC.DRIVE_NUMBER != "1">
  <#if MC.M2_CURRENT_SENSING_TOPO=='ICS_SENSORS'>
    <#assign M2_CS_TOPO = "ICS_">
  <#elseif (MC.M2_CURRENT_SENSING_TOPO=='SINGLE_SHUNT_PHASE_SHIFT') || (MC.M2_CURRENT_SENSING_TOPO=='SINGLE_SHUNT_ACTIVE_WIN')>
    <#assign M2_CS_TOPO = "R1_">
  <#elseif (MC.M2_CURRENT_SENSING_TOPO=='THREE_SHUNT')>
    <#assign M2_CS_TOPO = "R3_"+MC.M2_CS_ADC_NUM+"_">
  </#if>
</#if>

<#if MC.M1_OVERMODULATION == true> <#assign OVM ="_OVM"> <#else> <#assign OVM =""> </#if>
<#if MC.M2_OVERMODULATION == true> <#assign OVM2 ="_OVM"> <#else>  <#assign OVM2 =""> </#if>

<#if CondFamily_STM32C0 || CondFamily_STM32G0 || CondFamily_STM32F3 || CondFamily_STM32L4 || CondFamily_STM32F0 || CondFamily_STM32G4 || CondFamily_STM32H5>
<#assign currentFactor = "">
<#elseif CondFamily_STM32F4 || CondFamily_STM32F7>
<#assign currentFactor = "*2">
</#if>
<#if CondFamily_STM32F3 || CondFamily_STM32L4 || CondFamily_STM32F4 || CondFamily_STM32F7 || CondFamily_STM32G4 || CondFamily_STM32H5>
<#assign HAS_ADC_INJ = true>
<#else>
<#assign HAS_ADC_INJ = false>
</#if>
<#if CondFamily_STM32F3 || CondFamily_STM32L4 || CondFamily_STM32G0 || CondFamily_STM32F7 || CondFamily_STM32G4 || CondFamily_STM32H5 || CondFamily_STM32C0 >
<#assign HAS_TIM_6_CH = true>
<#else><#-- CondFamily_STM32F3 == false && CondFamily_STM32L4 == false && CondFamily_STM32G0 == false
         && CondFamily_STM32F7 == false && CondFamily_STM32G4 && CondFamily_STM32H5 == false && CondFamily_STM32C0 == false -->
<#assign HAS_TIM_6_CH = false>
</#if><#-- CondFamily_STM32F3 || CondFamily_STM32L4 || CondFamily_STM32G0 || CondFamily_STM32F7 
        || CondFamily_STM32G4  || CondFamily_STM32H5 || CondFamily_STM32C0  -->
<#if CondFamily_STM32F4 || CondFamily_STM32F7>
<#assign DMA_TYPE = "Stream">
<#assign DMA_MODE = "NORMAL">
<#else>
<#assign DMA_TYPE = "Channel">
<#assign DMA_MODE = "CIRCULAR">
</#if>
<#if CondFamily_STM32F3 || CondFamily_STM32L4 || CondFamily_STM32G4 || CondFamily_STM32F7 || CondFamily_STM32H5 || CondFamily_STM32H7>
<#assign HAS_BRK2 = true>
<#else>
<#assign HAS_BRK2 = false>
</#if>

<#if (MC.M1_DP_DESTINATION != "NONE") && (MC.M1_DP_DESTINATION == "TIM_BKIN")>
<#assign brkNb = "BRK">
<#assign brkEnBit = "BKE">
<#elseif (MC.M1_DP_DESTINATION != "NONE") && (MC.M1_DP_DESTINATION == "TIM_BKIN2")>
<#assign brkNb = "BRK2">
<#assign brkEnBit = "BK2E">
</#if>

<#if MC.M1_CS_ADC_NUM == "2">
<#assign ADCX=MC.M1_CS_ADC_U>
  <#if MC.M1_CS_ADC_U == MC.M1_CS_ADC_V>
    <#assign ADCY=MC.M1_CS_ADC_W>
  <#else>
    <#assign ADCY=MC.M1_CS_ADC_V>
  </#if>
<#else>
<#assign ADCX=MC.M1_CS_ADC_U>
<#assign ADCY=MC.M1_CS_ADC_V>
<#assign ADCZ=MC.M1_CS_ADC_W>
</#if>

<#if CondFamily_STM32F4 || CondFamily_STM32H5>
<#assign AUXTIMFreq = RCC.get("APB1TimFreq_Value")?number>
<#assign PWMTIMFreq = RCC.get("APB2TimFreq_Value")?number>
<#assign ADCFreq = RCC.get("APB2Freq_Value")?number>
</#if><#-- CondFamily_STM32F4 || CondFamily_STM32H5-->
<#if CondFamily_STM32F3>
<#assign APB1TIM_Freq = RCC.get("APB1TimFreq_Value")?number>
</#if><#-- CondFamily_STM32F3 -->
<#if CondFamily_STM32C0 || CondFamily_STM32G0 || CondFamily_STM32H5>
<#assign ADCFreq = RCC.get("ADCFreq_Value")?number>
</#if><#-- CondFamily_STM32C0 || CondFamily_STM32G0 || CondFamily_STM32H5 -->
<#if CondFamily_STM32C0 >
<#assign TIM1Freq = RCC.get("APBTimFreq_Value")?number>
</#if><#-- CondFamily_STM32C0 -->

<#if CondFamily_STM32G4 >
<#assign APB1Tim_CLKFreq = RCC.get("APB1TimFreq_Value")?number>
<#assign ADC_TIM_CLKFreq = RCC.get("ADC12Freq_Value")?number>
<#assign ADC_TIM_CLKFreq2 = RCC.get("ADC12Freq_Value")?number>
</#if><#-- CondFamily_STM32G4 -->

<#assign ADC_tab = [ MC.M1_CS_ADC_U, MC.M1_CS_ADC_V, MC.M1_CS_ADC_W, MC.M1_VS_ADC_U, MC.M1_VS_ADC_V, MC.M1_VS_ADC_W, MC.M1_VBUS_ADC, MC.M1_TEMP_FDBK_ADC, MC.POTENTIOMETER_ADC] >
<#assign ADC1Length = 0 >
<#assign ADC2Length = 0 >
<#assign ADC3Length = 0 >
<#assign ADC4Length = 0 >
<#assign ADC5Length = 0 >
<#list ADC_tab as x>
      <#if "ADC1" == x>
      <#assign ADC1Length++> 
      <#elseif "ADC2" == x>
      <#assign ADC2Length++> 
      <#elseif "ADC3" == x>
      <#assign ADC3Length++> 
      <#elseif "ADC4" == x>
      <#assign ADC4Length++> 
      <#elseif "ADC5" == x>
      <#assign ADC5Length++> 
      </#if>
</#list>
<#assign ADC_length = [ ADC1Length, ADC2Length, ADC3Length, ADC4Length, ADC5Length] >
<#assign maxLength = 0 >
<#list ADC_length as x>
      <#if  maxLength <= x>
      <#assign maxLength = x >
      </#if>
</#list>