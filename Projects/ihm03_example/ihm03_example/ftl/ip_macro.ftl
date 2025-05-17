<#macro setScandir Ph1 Ph2>
<#if Ph1?number < Ph2?number>
LL_ADC_REG_SEQ_SCAN_DIR_FORWARD>>ADC_CFGR1_SCANDIR_Pos,
<#else>
LL_ADC_REG_SEQ_SCAN_DIR_BACKWARD>>ADC_CFGR1_SCANDIR_Pos,
</#if>
<#return>
</#macro>

<#macro define_IC_FILTER  motor sensor driver icx_filter>
#define M${ motor }_${ sensor }_IC_FILTER_LL LL_TIM_IC_FILTER_FDIV${Fx_ic_filter(icx_filter, "coeffs")}
<#if driver == "HAL">
#define M${ motor }_${ sensor }_IC_FILTER   ${Fx_ic_filter(icx_filter, "index")}
<#else>
#define M${ motor }_${ sensor }_IC_FILTER   LL_TIM_IC_FILTER_FDIV${Fx_ic_filter(icx_filter, "coeffs")}
</#if>
</#macro>