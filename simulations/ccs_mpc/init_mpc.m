%% init_mpc.m - Parametry symulacji CCS-MPC dla PMSM
clear; clc;

% 1. Czas próbkowania i częstotliwości
f_pwm   = 10e3;            % Częstotliwość nośnej trójkątnej PWM: 20 kHz
T_pwm   = 1 / f_pwm;       % Okres fali nośnej PWM: 50 us (0.00005 s)

% 1. Krok pętli prądowej FOC / CCS-MPC (Single Update - raz na okres PWM)
Ts_cur  = T_pwm;           % 50 us (20 kHz)

% 2. Krok pętli prędkościowej PI (10x wolniej od pętli prądowej)
Ts_spd  = 10 * Ts_cur;     % 500 us (2 kHz)

% 3. Ograniczenie kroku dla solvera Variable-step (Max step size)
Ts_max  = T_pwm / 25;      % 2 us (2e-6 s)

% 2. Parametry silnika PMSM (przykładowy silnik SPMSM/IPMSM)
Rs = 0.25;                  % Rezystancja stojana [Ohm]
Ld = 0.8e-3;                % Indukcyjność w osi d [H]
Lq = 0.8e-3;                % Indukcyjność w osi q [H]
Psi_f = 0.065;              % Strumień magnesów trwałych [Wb]
p = 4;                      % Liczba par biegunów [-]
J = 0.0012;                 % Moment bezwładności [kg*m^2]
B = 0.0005;                 % Tarcie lepkie [N*m*s/rad]

% 3. Parametry falownika
U_dc = 48;                  % Napięcie szyny DC [V]
U_max = U_dc / sqrt(3);     % Promień koła wpisanego w sześciokąt SVPWM [V]

% 4. Parametry regulatora CCS-MPC (wagi funkcji kosztu)
q_d = 1.0;                  % Waga uchybu prądu w osi d
q_q = 1.0;                  % Waga uchybu prądu w osi q
r_d = 0.002;                % Waga tłumienia przyrostu napięcia delta_u_d
r_q = 0.002;                % Waga tłumienia przyrostu napięcia delta_u_q

% Wzmocnienie obserwatora zakłóceń (całkowanie błędu predykcji)
Ki_obs_d = 0.15;            
Ki_obs_q = 0.15;