%% Kalman

% x(k) = [theta(k); w(k)] wektor stanu w chwili t

% Równania stanu:

% theta(k+1) = theta(k) + w(k)*dt
% w(k+1) = w(k)

% W formie macierzowej:
% Model stanu
% x(k+1) = A*x(k) + B*u(k) + w(k), w(k) szum procesowy (macierz kowariancji
% Q)

% Model pomiaru
% z(k) = H*x(k) + v(k), v(k) - szum pomiarowy (macierz kow. R)

root = "../data/encoder";
encoder_data  = readtable(fullfile(root,'encoder_step_response.csv'));
encoder_data  = rmmissing(encoder_data);

t0 = min(encoder_data.timestamp);

encoder_data.t  = (encoder_data.timestamp  - t0) / 1000.0;

t_min = 10;
t_max = 20;

theta_mech = encoder_data(strcmp(encoder_data.variable, "monitor_data.theta_mech"), :);
theta_mech = theta_mech(theta_mech.t >= t_min & theta_mech.t <= t_max, :);

theta_raw = theta_mech.value; % kąt modulo 2pi
theta_vals = unwrap(theta_raw); % kąt odwinięty

omega_ref = encoder_data(strcmp(encoder_data.variable, "monitor_data.speed_ref"), :);
omega_ref = omega_ref(omega_ref.t >= t_min & omega_ref.t <= t_max, :);
omega_ref_vals = omega_ref.value;           % [rpm]
omega_ref_rad  = omega_ref_vals * 2*pi/60;  % [rad/s]

t = theta_mech.t;

dt = t(2) - t(1);
x = [theta_vals(1); 0];  % Inicjalizacja wektora stanu z średnim kątem i zerową prędkością

P = eye(2);  % Macierz kowariancji stanu
A = [1, dt; 0, 1];
H = [1, 0];
R = 5e-5;

q_theta = 1e-8;
q_omega = 1e-4;   % lub nawet 1e-1 na początek

Q = diag([q_theta, q_omega]);

N = length(theta_vals);
x_hist = zeros(2, N);   % do zapisu wyników

for k = 1:N
    % --- PREDYKCJA ---
    x = A * x;             % x(k|k-1)
    P = A * P * A' + Q;    % P(k|k-1)

    % --- KOREKCJA ---
    z = theta_vals(k);         % pomiar kąta
    y = z - H * x;             % innowacja

    S = H * P * H' + R;        % skalar
    K = P * H' / S;            % 2x1

    x = x + K * y;             % x(k|k)
    P = (eye(2) - K * H) * P;  % P(k|k)

    x_hist(:,k) = x;           % zapisz estymaty
end

theta_hat = x_hist(1,:).';   % Nx1
omega_hat = x_hist(2,:).';   % Nx1

% Zwykłe różniczkowanie
omega_diff = diff(theta_vals) / dt;      % (N-1)x1
omega_diff = [omega_diff; omega_diff(end)];  % Nx1
t_omega    = t;                          % Nx1

figure;
% subplot(2,1,1);
% hold on; grid on;
% plot(t, theta_vals, '.', 'DisplayName','\theta pomiar');
% plot(t, theta_hat, 'r', 'DisplayName','\theta_{KF}');
% ylabel('\theta [rad]');
% legend;
% 
% subplot(2,1,2);
hold on; grid on;
plot(t_omega, omega_diff, '-r', 'DisplayName','d\theta/dt');
plot(t,       omega_hat,  'b','LineWidth',3,'DisplayName','\omega_{KF}');
plot(omega_ref.t, omega_ref_rad, 'g--', 'DisplayName','\omega_{ref}'); % jeśli chcesz
ylabel('\omega [rad/s]');
xlabel('t [s]');
legend;

err_kf   = omega_hat  - omega_ref_rad;   % Nx1
err_diff = omega_diff - omega_ref_rad;   % Nx1

rmse_kf   = sqrt(mean(err_kf.^2)) % [rad/s]
rmse_diff = sqrt(mean(err_diff.^2)) % [rad/s]
