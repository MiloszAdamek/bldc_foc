<#function getOPAMPMap Motor>
   <#if Motor == 1>
     <#switch MC.M1_CS_OPAMP_PHASE_SHARED>
       <#case "U">
         <#local OPAMPMapp = M1_OPAMPInputMapp_Shared_U>
         <#break>
       <#case "V">
         <#local OPAMPMapp = M1_OPAMPInputMapp_Shared_V>
         <#break>
       <#case "W">
         <#local OPAMPMapp = M1_OPAMPInputMapp_Shared_W>
         <#break>
       <#default><#-- No phase shared at Opmap Level -->
         <#local OPAMPMapp = M1_OPAMPInputMapp_3_OPAMPS>
         <#break>
     </#switch>
   <#elseif Motor==2>
   <#-- To be implemented -->
     <#switch MC.M2_CS_OPAMP_PHASE_SHARED>
       <#case "U">
         <#local OPAMPMapp = M2_OPAMPInputMapp_Shared_U>
         <#break>
       <#case "V">
         <#local OPAMPMapp = M2_OPAMPInputMapp_Shared_V>
         <#break>
       <#case "W">
         <#local OPAMPMapp = M2_OPAMPInputMapp_Shared_W>
         <#break>
       <#default><#-- No phase shared at Opmap Level -->
         <#local OPAMPMapp = M2_OPAMPInputMapp_3_OPAMPS>
         <#break>
     </#switch>
   </#if>
   <#return OPAMPMapp>
</#function>

<#function getOPAMP config Sector Phase Motor=1>
  <#local OPAMPMap=getOPAMPMap(Motor)>
  <#list OPAMPMap as OPAMPItem>
  <#if Motor == 1>
    <#if OPAMPItem.Sector == Sector>
      <#if MC.M1_CS_OPAMP_DYNAMIC_OUTPUT_SWITCH == false>
        <#if config=="IP">
          <#return _first_word(OPAMPItem[Phase])>
        <#else>
          <#if MC.M1_CS_OPAMP_NUM == '3'>
            <#return "OPAMP_UNCHANGED">
          <#else>  
            <#return OPAMPItem[Phase]>
          </#if>
        </#if>
      <#else><#-- Dynamic opamp to configure -->
        <#if _first_word(OPAMPItem[Phase]) == M1_OPAMPip[MC.M1_CS_ADC_PHASE_SHARED]>
          <#if config=="IP">
            <#return M1_OPAMPip[MC.M1_CS_ADC_PHASE_SHARED]>
          <#else>
            <#if MC.M1_CS_OPAMP_DIRECT_LINK_TO_ADC == getADC("IP",Sector,Phase,1)>
              <#return "DIRECT_CONNECT|"+OPAMPItem[Phase]>
            <#else>
              <#return "PIN_CONNECT|"+OPAMPItem[Phase]>
            </#if>
          </#if>
        <#else><#-- Not an OPAMP we configure return "NULL" -->
          <#if config=="IP">
            <#return _first_word(OPAMPItem[Phase])>
          <#else>
            <#return "OPAMP_UNCHANGED">
          </#if>
        </#if>
      </#if>
    </#if>
  <#elseif Motor == 2>  
    <#if OPAMPItem.Sector == Sector>
      <#if MC.M2_CS_OPAMP_DYNAMIC_OUTPUT_SWITCH == false>
        <#if config=="IP">
          <#return _first_word(OPAMPItem[Phase])>
        <#else>
          <#if MC.M2_CS_OPAMP_NUM == '3'>
            <#return "OPAMP_UNCHANGED">
          <#else>  
            <#return OPAMPItem[Phase]>
          </#if>
        </#if>
      <#else><#-- Dynamic opamp to configure -->
        <#if _first_word(OPAMPItem[Phase]) == M2_OPAMPip[MC.M2_CS_ADC_PHASE_SHARED]>
          <#if config=="IP">
            <#return M2_OPAMPip[MC.M2_CS_ADC_PHASE_SHARED]>
          <#else>
            <#if MC.M2_CS_OPAMP_DIRECT_LINK_TO_ADC == getADC("IP",Sector,Phase,2)>
              <#return "DIRECT_CONNECT|"+OPAMPItem[Phase]>
            <#else>
              <#return "PIN_CONNECT|"+OPAMPItem[Phase]>
             </#if>
          </#if>
        <#else><#-- Not an OPAMP we configure return "NULL" -->
          <#if config=="IP">
            <#return _first_word(OPAMPItem[Phase])>
          <#else>
            <#return "OPAMP_UNCHANGED">
          </#if>
        </#if>
      </#if>
    </#if>
  <#else>
    #error motor number could not exceed 2
  </#if>
  </#list>
</#function>

<#function getADCMap Motor>
   <#if Motor == 1>
     <#switch MC.M1_CS_ADC_PHASE_SHARED>
       <#case "U">
         <#local ADCMap = M1_ADCConfig_Shared_U>
         <#break>
       <#case "V">
         <#local ADCMap = M1_ADCConfig_Shared_V>
         <#break>
       <#case "W">
         <#local ADCMap = M1_ADCConfig_Shared_W>
         <#break>
       <#default><#-- No pahse shared at ADC Level -->
         <#switch MC.M1_CS_OPAMP_PHASE_SHARED>
           <#case "U">
             <#local ADCMap=M1_ADCConfig_2OPAMPs_Shared_U>
             <#break>
           <#case "V">
             <#local ADCMap=M1_ADCConfig_2OPAMPs_Shared_V>
             <#break>
           <#case "W">
             <#local ADCMap=M1_ADCConfig_2OPAMPs_Shared_W>
             <#break>
           <#default>
             <#local ADCMap=M1_ADCConfig_3ADCS><#-- No phase shared at ADC level not OPAMP level -->
            <#break>
         </#switch>
       <#break>
     </#switch>
   <#elseif Motor==2>
   <#-- To be implemented -->
     <#switch MC.M2_CS_ADC_PHASE_SHARED>
       <#case "U">
         <#local ADCMap = M2_ADCConfig_Shared_U>
         <#break>
       <#case "V">
         <#local ADCMap = M2_ADCConfig_Shared_V>
         <#break>
       <#case "W">
         <#local ADCMap = M2_ADCConfig_Shared_W>
         <#break>
       <#default><#-- No pahse shared at ADC Level -->
         <#switch MC.M2_CS_OPAMP_PHASE_SHARED>
           <#case "U">
             <#local ADCMap=M2_ADCConfig_2OPAMPs_Shared_U>
             <#break>
           <#case "V">
             <#local ADCMap=M2_ADCConfig_2OPAMPs_Shared_V>
             <#break>
           <#case "W">
             <#local ADCMap=M2_ADCConfig_2OPAMPs_Shared_W>
             <#break>
           <#default>
             <#local ADCMap=M2_ADCConfig_3ADCS><#-- No phase shared at ADC level not OPAMP level -->
            <#break>
         </#switch>
       <#break>
     </#switch>
   </#if>
   <#return ADCMap>
</#function>

<#function getADCChannel ADC channelIN>
  <#if CondFamily_STM32G4 >
    <#if channelIN?contains("VPOPAMP")>
      <#local ADCChannelMap = G4_ADC_Channel_Table>
      <#list ADCChannelMap as ADCChannelItem>
        <#if ADCChannelItem.VPOPAMP == channelIN && ADCChannelItem.ADC == ADC>
          <#return ADCChannelItem.CHANNEL>
        </#if>
      <#else> 
        <#return "OPAMP_ADC_CHANNEL_ERROR" >
	  </#list>  
    <#else> <#-- Classic ADC Channel inside G4 family -->
      <#return channelIN>
	</#if>
  <#else> <#-- Only G4 has the OPAMP ADC direct connection capability -->
    <#return channelIN>
  </#if>
</#function>

<#function getADC config sector phase Motor=1>
 <#local ADCMap = getADCMap(Motor)>
 <#if Motor == 1>
  <#list ADCMap as ADCItem>
     <#if ADCItem.Sector==sector>
       <#if config="IP">
         <#return _first_word(ADCItem[phase])>
       <#else>
         <#return getADCChannel (_first_word(ADCItem[phase]),_last_word(ADCItem[phase]))>
       </#if>
     </#if>
  </#list>
 <#elseif Motor==2>
  <#list ADCMap as ADCItem>
     <#if ADCItem.Sector==sector>
       <#if config="IP">
         <#return _first_word(ADCItem[phase])>
       <#else>
         <#return getADCChannel (_first_word(ADCItem[phase]),_last_word(ADCItem[phase]))>
       </#if>
     </#if>
  </#list>
  <#else>
    #error motor number could not exceed 2
  </#if>
</#function>

<#assign PWM_Timer_M1 = _last_word(MC.M1_PWM_TIMER_SELECTION)>
<#assign PWM_Timer_M2 = _last_word(MC.M2_PWM_TIMER_SELECTION)>

<#function _filter_opamp opamp>
  <#if opamp == "OPAMP1">
<#return "OPAMP">
  <#else>
<#return opamp>
  </#if>
</#function>
