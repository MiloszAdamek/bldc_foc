---
config:
  theme: redux-color
  themeVariables:
    fontFamily: "Calibri, Arial"
  sequence:
    actorFontSize: 18
    messageFontSize: 18
    noteFontSize: 16
---
sequenceDiagram
    autonumber

    %% ================================
    %% HARDWARE
    %% ================================
    box #e8eaf6 Hardware
        participant TIM1 as TIM1<br/>(PWM + Update 40 kHz)
        participant TIM4 as TIM4<br/>(Ext Clock Mode 1<br/>PSC=4 → 10 kHz)
        participant TIM2 as TIM2<br/>(Ext Clock Mode 1<br/>PSC=40 → 1 kHz)
        participant ADC_HW as ADC Injected<br/>(Hardware)
        participant SPI as SPI DMA<br/>(AS5048A)
    end

    %% ================================
    %% SOFTWARE
    %% ================================
    box #ffe0b2 Software
        participant ISR_TIM1 as ISR: TIM1 Update
        participant ISR_TIM4 as ISR: TIM4 Update
        participant ISR_TIM2 as ISR: TIM2 Update
        participant ISR_ADC as ISR: ADC Injected
        participant ISR_SPI as ISR: SPI DMA
        participant FOC as FOC_RunLoop()
    end

    Note over TIM1: Center-aligned PWM<br/>CNT: 0 → ARR → 0 (40 kHz Update)

    %% ----------------------------------------------------
    %% 1. HARDWARE: ADC TRIGGER (20 kHz, CC4)
    %% ----------------------------------------------------
    rect rgb(255,240,240)
        Note over TIM1, ADC_HW: CC4 compare match<br/>Sprzętowe wyzwolenie ADC
        TIM1->>ADC_HW: Start ADC injected conversion
        activate ADC_HW
    end

    Note over TIM1: CNT: 0 → ARR<br/>(zliczanie w górę)
    %% ----------------------------------------------------
    %% 2. TIM1 UPDATE: CNT == ARR (20 kHz)
    %% ----------------------------------------------------
    TIM1->>ISR_TIM1: Update Event<br/>(CNT == ARR)
    activate ISR_TIM1
        Note right of ISR_TIM1: TRGO → TIM4 & TIM2 (40 kHz input)
        ISR_TIM1->>TIM4: TRGO (40 kHz)
        ISR_TIM1->>TIM2: TRGO (40 kHz)
    deactivate ISR_TIM1

    %% ----------------------------------------------------
    %% 3. TIM4 dzieli 40 kHz → 10 kHz
    %% ----------------------------------------------------
    TIM4->>ISR_TIM4: TIM4 Update Event<br/>(PSC=4 → 10 kHz)
    activate ISR_TIM4
        Note right of ISR_TIM4: Start odczytu enkodera (SPI DMA)
        ISR_TIM4->>SPI: AS5048_ReadAngleDMA()
        activate SPI
    deactivate ISR_TIM4

    %% ----------------------------------------------------
    %% 4. TIM2 dzieli 40 kHz → 1 kHz
    %% ----------------------------------------------------
    TIM2->>ISR_TIM2: TIM2 Update Event<br/>(PSC=40 → 1 kHz)
    activate ISR_TIM2
        Note right of ISR_TIM2: PI Speed Control<br/>iq_ref = PI(n_ref − n_est)
        ISR_TIM2->>ISR_TIM2: Aktualizacja iq_ref
    deactivate ISR_TIM2

    %% ----------------------------------------------------
    %% 5. RÓWNOLEGŁE DZIAŁANIA SPRZĘTOWE
    %% ----------------------------------------------------
    par ADC sampling
        ADC_HW->>ADC_HW: Próbkowanie Ia/Ib (20 kHz)
    and SPI transfer
        SPI->>SPI: Transfer DMA<br/>2 bajty z AS5048A
    end

    %% ----------------------------------------------------
    %% 6. ADC Injected Conversion Complete (20 → 10 kHz)
    %% ----------------------------------------------------
    ADC_HW-->>ISR_ADC: JEOC interrupt
    deactivate ADC_HW
    activate ISR_ADC
        Note right of ISR_ADC: Obsługa tylko przy zliczaniu w dół (CNT → 0)<br/>(10 kHz)<br/>current_ready = true
        ISR_ADC->>ISR_ADC: CurrentSense_Process_ISR()
    deactivate ISR_ADC

    %% ----------------------------------------------------
    %% 7. SPI DMA Complete (10 kHz)
    %% ----------------------------------------------------
    SPI-->>ISR_SPI: DMA complete
    deactivate SPI
    activate ISR_SPI
        ISR_SPI->>ISR_SPI: raw_angle decoded<br/>encoder_ready = true
    deactivate ISR_SPI

    Note over TIM1: CNT: ARR → 0<br/>(zliczanie w dół)

    %% ----------------------------------------------------
    %% 8. TIM1 UPDATE: CNT == 0 (20 kHz) → toggle → FOC 10 kHz
    %% ----------------------------------------------------
    TIM1->>ISR_TIM1: Update Event<br/>(CNT == 0)
    activate ISR_TIM1
        Note right of ISR_TIM1: FOC 10 kHz<br/>toggle == true → wykonaj FOC<br/>toggle == false → pomiń
        ISR_TIM1->>FOC: FOC_RunLoop()
        activate FOC

            %% FOC core
            FOC->>FOC: if (current_ready && encoder_ready)
            alt Brak synchronizacji danych
                FOC->>FOC: foc_loop_err++
            else Dane kompletne
                FOC->>FOC: Read θ_mech & θ_el
                FOC->>FOC: SpeedEstimator_Update()
                FOC->>FOC: CurrentSense_CalculatePhases()
                FOC->>FOC: Clarke → Park → PI(id, iq_ref)
                FOC->>FOC: InvPark
                FOC->>TIM1: SVPWM → Update CCR1/2/3
                FOC->>FOC: foc_loop_ok++
            end

        deactivate FOC
    deactivate ISR_TIM1