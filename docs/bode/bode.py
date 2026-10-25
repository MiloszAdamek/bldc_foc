import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from control import tf, bode, forced_response

# =====================================================
# 1. Wczytanie logów Id/Iq
# =====================================================
idiq = pd.read_csv("data/1/id_iq_2025-11-30_14-42-23.csv").dropna()

t0 = idiq["timestamp"].min()
idiq["t"] = (idiq["timestamp"] - t0) / 1000.0

iq = idiq[idiq["variable"] == "monitor_data.iq"]["value"].to_numpy()
iq_ref = idiq[idiq["variable"] == "monitor_data.iq_ref"]["value"].to_numpy()
t = idiq[idiq["variable"] == "monitor_data.iq"]["t"].to_numpy()

# =====================================================
# 2. Znalezienie momentu skoku referencji
# =====================================================
diff = np.abs(np.diff(iq_ref))
step_index = np.argmax(diff)     # największa zmiana = skok
t_step = t[step_index]

print(f"Skok referencji wykryty w t = {t_step:.4f} s")

# Przesunięcie czasu tak, aby skok był w t=0
t_shifted = t - t_step

# Odcinamy próbki „przed skokiem”
valid = t_shifted >= 0
t_step_resp = t_shifted[valid]
iq_step_resp = iq[valid]
iq_ref_after = iq_ref[valid]

# =====================================================
# 3. Rysowanie odpowiedzi skokowej z logów
# =====================================================
plt.figure()
plt.plot(t_step_resp, iq_step_resp, label="Iq (actual)")
plt.plot(t_step_resp, iq_ref_after, "--", label="Iq_ref")
plt.xlabel("czas [s]")
plt.ylabel("Iq [A]")
plt.title("Odpowiedź skokowa prądu Iq (z logów FOC)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("step_response_iq.png", dpi=300)
plt.show()

# =====================================================
# 4. Dopasowanie prostego modelu 1-rzędowego
#    G(s) = K / (tau*s + 1)
# =====================================================

# Parametry wyznaczone heurystycznie
Iq_final = iq_step_resp[-1]
Iq_init = iq_step_resp[0]
K = Iq_final - Iq_init

# Szukanie tau ≈ czas do 63% wartości końcowej
target = Iq_init + 0.63*K
idx_tau = np.argmin(np.abs(iq_step_resp - target))
tau = t_step_resp[idx_tau]

print(f"Dopasowany model: G(s) = {K:.3f} / ({tau:.4f} s * s + 1)")

# Model transfer function
sys = tf([K], [tau, 1])

# =====================================================
# 5. Bode plot modelu regulatora
# =====================================================
plt.figure()
bode(sys, dB=True, Hz=True)   # <-- tylko wywołanie, bez assignowania do zmiennych
plt.tight_layout()
plt.savefig("bode_iq.png", dpi=300)
plt.show()

print("Step response + Bode wygenerowane!")
