<#ftl>
<#if !MC??>
  <#if SWIPdatas??>
    <#list SWIPdatas as SWIP>
      <#if SWIP.ipName == "MotorControl">
        <#if SWIP.parameters??>
          <#assign MC = SWIP.parameters>
          <#break>
        </#if>
      </#if>
    </#list>
  </#if>
  <#if MC??>
  <#else>
    <#stop "No MotorControl SW IP data found">
  </#if>
  <#if configs[0].peripheralParams.get("RCC")??>
    <#assign RCC = configs[0].peripheralParams.get("RCC")>
  </#if>
  <#if !RCC??>
    <#stop "No RCC found">
  </#if>
</#if>