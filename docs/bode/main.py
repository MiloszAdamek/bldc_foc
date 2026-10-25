import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ================================================
# 1. Konfiguracja stylu (Publikacja Techniczna)
# ================================================
# Reset do domyślnych, żeby nie śmiecić
plt.style.use('default')

# Słownik parametrów dla wyglądu "książkowego"
params = {
    'font.family': 'serif',
    'font.serif': ['Times New Roman'], # Zgodność z tekstem pracy
    'font.size': 12,
    'axes.labelsize': 12,
    'axes.titlesize': 12,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'text.usetex': False,       # True jeśli masz zainstalowanego TeXa lokalnie, False jest bezpieczniejsze
    'mathtext.fontset': 'stix', # Ładne symbole matematyczne bez instalacji TeXa
    'figure.figsize': (8, 6),   # Rozmiar A4 friendly (szerokość szpalty)
    'lines.linewidth': 1.2,
    'axes.grid': True,
    'grid.alpha': 0.4,
    'grid.linestyle': '--',
    'savefig.dpi': 300,         # Standard druku
    'savefig.bbox': 'tight'
}
plt.rcParams.update(params)

# Wczytanie i przygotowanie danych
try:
    curr = pd.read_csv("data/1/phase_currents_2025-11-30_14-42-23.csv").dropna()
    speed = pd.read_csv("data/1/speed_2025-11-30_14-42-23.csv").dropna()
    idiq = pd.read_csv("data/1/id_iq_2025-11-30_14-42-23.csv").dropna()
except FileNotFoundError:
    exit()

# Normalizacja czasu do wspólnego zera
t0 = min(curr["timestamp"].min(), speed["timestamp"].min(), idiq["timestamp"].min())

for df in (curr, speed, idiq):
    df["t"] = (df["timestamp"] - t0) / 1000.0

# Filtrowanie zmiennych
speed_actual = speed[speed["variable"] == "monitor_data.speed"]
speed_ref = speed[speed["variable"] == "monitor_data.speed_ref"]

id_actual = idiq[idiq["variable"] == "monitor_data.id"]
id_ref = idiq[idiq["variable"] == "monitor_data.id_ref"]
iq_actual = idiq[idiq["variable"] == "monitor_data.iq"]
iq_ref = idiq[idiq["variable"] == "monitor_data.iq_ref"]

# ================================================
# Rysunek Złożony: Dynamika Układu Regulacji
# Tworzymy 2 wykresy jeden pod drugim, dzielące oś czasu
# ================================================
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, figsize=(8, 7))

# --- Wykres górny: Prędkość ---
# Używamy r'' dla zapisu LaTeX-podobnego w labelach
ax1.plot(speed_actual["t"], speed_actual["value"], label=r'$\omega$ (pomiar)', color='black')
ax1.plot(speed_ref["t"], speed_ref["value"], '--', label=r'$\omega_{ref}$ (zadana)', color='red', alpha=0.8)

ax1.set_ylabel(r'Prędkość $n$ [obr/min]') # Polska jednostka
ax1.legend(loc='lower right')
ax1.grid(True, which='both', linestyle=':', linewidth=0.5)

# --- Wykres dolny: Prądy Id / Iq ---
ax2.plot(id_actual["t"], id_actual["value"], label=r'$i_d$', color='#1f77b4') # Niebieski
ax2.plot(iq_actual["t"], iq_actual["value"], label=r'$i_q$', color='#ff7f0e') # Pomarańczowy

# Referencje rysujemy cieniej i linią przerywaną
ax2.plot(id_ref["t"], id_ref["value"], '--', linewidth=1, color='#1f77b4', alpha=0.6)
ax2.plot(iq_ref["t"], iq_ref["value"], '--', linewidth=1, color='#ff7f0e', alpha=0.6)

ax2.set_ylabel(r'Prąd $i_{dq}$ [A]')
ax2.set_xlabel(r'Czas $t$ [s]')


# Legenda poza wykresem lub w środku - zależy od danych. 
# Tutaj sztuczka: tworzymy "custom" legendę, żeby nie dublować ref/actual
from matplotlib.lines import Line2D
custom_lines = [Line2D([0], [0], color='#1f77b4', lw=2),
                Line2D([0], [0], color='#ff7f0e', lw=2),
                Line2D([0], [0], color='gray', lw=1, linestyle='--')]
ax2.legend(custom_lines, [r'$i_d$', r'$i_q$', 'Wartość zadana'], loc='upper right', frameon=True)

# Formatowanie osi (np. rzadsze tiki, jeśli dane są gęste)
ax2.xaxis.set_major_locator(ticker.MaxNLocator(integer=False, prune='upper'))

plt.tight_layout()
fig.align_ylabels() # Wyrównanie etykiet osi Y w pionie

# Zapis
plt.savefig("fig_regulacja_foc.pdf") # PDF do LaTeXa
plt.savefig("fig_regulacja_foc.png") # PNG do Worda
plt.close() # Zamknij, żeby nie zajmować pamięci

# ================================================
# Rysunek: Prądy fazowe (Zoom na przebieg)
# ================================================
fig2, ax = plt.subplots(figsize=(8, 4))

# Kolory faz standardowe lub wyraźne
colors = {'monitor_data.current_a': '#000000', # Czarny
          'monitor_data.current_b': '#E69F00', # Pomarańczowy (colorblind friendly)
          'monitor_data.current_c': '#56B4E9'} # Błękitny

labels = {'monitor_data.current_a': r'$i_a$',
          'monitor_data.current_b': r'$i_b$',
          'monitor_data.current_c': r'$i_c$'}

# Rysowanie pętli
for phase_name in ["monitor_data.current_a", "monitor_data.current_b", "monitor_data.current_c"]:
    df_ph = curr[curr["variable"] == phase_name]
    # Opcjonalnie: wytnij tylko fragment czasu, żeby pokazać sinusoidę
    # df_ph = df_ph[(df_ph["t"] > 1.0) & (df_ph["t"] < 1.05)] 
    ax.plot(df_ph["t"], df_ph["value"], 
            label=labels[phase_name], 
            color=colors[phase_name], 
            linewidth=1)

ax.set_xlabel(r'Czas $t$ [s]')
ax.set_ylabel(r'Prąd fazowy [A]')
ax.legend(loc='upper right', frameon=True, ncol=3) # Legenda horyzontalna

plt.tight_layout()
fig2.align_ylabels() # Wyrównanie etykiet osi Y w pionie
plt.savefig("fig_prady_fazowe.pdf")
plt.savefig("fig_prady_fazowe.png")
plt.show()