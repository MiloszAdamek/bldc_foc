% Parametry AOTL66912

R_g = 0; %Ohm,  Brak rezystora bramki
f_pwm = 20 * 1e3; %khz
T_pwm = 1/f_pwm;

t_sw = 0.015 * T_pwm; % Czas przełączania na poziomie max 3% okresu PWM

% Dla V_gs = 10V
Rds_on = 1.7 * 1e-3; %Ohm
Qg_total = 220 * 1e-9; %nC - typowo 155 nC
Qgd      = 31 * 1e-9;  %nC (ładunek Millera / Gate-Drain Charge, typowy)

% Obliczenie wymaganego średniego prądu bramki
I_drive = Qg_total / t_sw;

I_drive_N = 2 * I_drive; % Prąd rozładowania bramki, 2-4x większy ze wzlgędu na tłumienie efektu Millera
I_drive_P = I_drive; % Prąd ładowania bramki

T_drive = 2.5 * t_sw;

% --- WYPISANIE WYNIKÓW W TERMINALU ---
fprintf(' Okres PWM (T_pwm):           %.2f us\n', T_pwm * 1e6);
fprintf(' Docelowy czas t_sw (1.5%%):     %.2f us (%.0f ns)\n', t_sw * 1e6, t_sw * 1e9);
fprintf('--------------------------------------------------\n');
fprintf(' Wymagany prąd IDRIVEP (SRC): %.2f mA\n', I_drive_P * 1e3);
fprintf(' Wymagany prąd IDRIVEN (SNK): %.2f mA\n', I_drive_N * 1e3);
fprintf(' Wyliczony T_drive:           %.2f us (%.0f ns)\n', T_drive * 1e6, T_drive * 1e9);

% Czasy przełaczenia
t_sw_rise = Qg_total / I_drive_P;
t_sw_fall = Qg_total / I_drive_N;

% Czas rozładowania bramki
t_discharge = Qg_total / I_drive_N;

% Czas trwania obszaru plateau Millera podczas wyłączania
t_miller_off = Qgd / I_drive_N;

% Narzut na niedopasowanie propagacji kanałów w driverze (Delay Matching)
t_pd_mismatch = 50 * 1e-9; % s (standardowy margines bezpieczeństwa 50 ns)

% Wyznaczenie minimalnego czasu martwego
t_dead_min = t_discharge + t_pd_mismatch;

fprintf(' ------------------------------------------------------\n');
fprintf(' Rzeczywisty czas narastania:  %.2f us (przy %.1f A)\n', t_sw_rise * 1e6, I_drive_P);
fprintf(' Rzeczywisty czas opadania:    %.2f us (przy %.1f A)\n', t_sw_fall * 1e6, I_drive_N);
fprintf(' ------------------------------------------------------\n');
fprintf(' Czas rozładowania bramki:     %.2f ns\n', t_discharge * 1e9);
fprintf(' Czas plateau Millera (Qgd):   %.2f ns\n', t_miller_off * 1e9);
fprintf(' Wyliczony min. czas martwy:   %.2f ns\n', t_dead_min * 1e9);
fprintf('========================================================\n');

% --- OBLICZENIA OCP ---
I_max_peak = 60;          % A (szczytowy prąd fazowy / blokada wirnika)
T_j_max = 100;            % °C (szacowana maksymalna temperatura złącza)

Rds_on_max_25 = 1.7 * 1e-3; % Ohm (max Rds_on z karty katalogowej przy 10V)
K_temp = 1.4;             % Współczynnik wzrostu Rds_on dla ~100°C (z Fig. 4 w datasheet)
Rds_on_hot = Rds_on_max_25 * K_temp;

% Maksymalny spadek napięcia Vds przy pełnym prądzie i gorącym tranzystorze
Vds_max = I_max_peak * Rds_on_hot;

% Próg OCP z 40% zapasem na transjenty prądowe
Vds_ocp_target = Vds_max * 1.4;

fprintf('------------------------------------------------------\n');
fprintf(' OBLICZENIA ZABEZPIECZENIA OCP:\n');
fprintf(' Rds_on w temp. %.0f°C:        %.2f mOhm\n', T_j_max, Rds_on_hot * 1e3);
fprintf(' Max spadek Vds (przy %dA):    %.1f mV\n', I_max_peak, Vds_max * 1e3);
fprintf(' Sugerowany próg Vds_OCP:     %.1f mV\n', Vds_ocp_target * 1e3);
fprintf('========================================================\n');