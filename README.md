# CPU Cache Performance Visualizer

Ich hab dieses Programm geschrieben um ein besseres Verständnis für Code Effizienz zu erlangen.
Wissen entsteht nicht durch Beobachten, sondern durch Nachbauen.
Die Software ist ein interaktives Lern- und Analyseprogramm zur Visualisierung von Cache-Effizienz, Speicherlokalität und CPU-Speicherlatenzen.  
Das Projekt zeigt praxisnah, warum zeilenweiser Speicherzugriff deutlich schneller ist als spaltenweiser Zugriff — insbesondere bei großen Matrizen, die mehrere Cache-Ebenen und den RAM passieren.
![GUI](/cpu-performance-visualizer/assets/cpuvisualizer.png)
---

##  Features

### Messungen
- Messung der Speicherzugriffszeit in CPU-Taktzyklen (RDTSCP)
- Vergleich von:
  - **Row-major (zeilenweise)**
  - **Column-major (spaltenweise)**
- 30 Wiederholungen pro Durchlauf
- Cache-Flush vor jeder Messung (CLFLUSH + MFENCE)
- Prozess-Affinität auf festen Kern für reproduzierbare Ergebnisse
- Hohe Prozesspriorität für stabile Messwerte

### Berechnete Kennzahlen
- Durchschnitt (avg)
- Median
- Standardabweichung
- Verhältnis **Spalte/Zeile**
- Automatische Klassifikation nach Cache-Größe (L1/L2/L3/RAM)

---

##  GUI

Die Anwendung besitzt ein leichtes UI basierend auf:
- SDL2  
- OpenGL  
- Dear ImGui  

Mit Funktionen wie:
- Live-Konsole
- Einstellbarer Matrixgröße
- Popup-Fenstern mit Erklärungen des Speicherzugriffs
- Anzeige des verwendeten Zugriffscodes (row/column)

---

##  Vermittelte technische Konzepte
![Erklärungen](/cpu-performance-visualizer/assets/technisch.png)

Das Projekt macht folgende Hardware- und Betriebssystemkonzepte sichtbar:

- Cache-Lines (64 Byte)
- Spatial vs. Temporal Locality
- L1 / L2 / L3 Cache Grenzen
- RAM-Zugriffe und hohe Latenzen
- Page Faults bei großen Matrizengrößen
- Windows Kernel-Interaktionen wie:
  - `MiPageFault`
  - `MiZeroPhysicalPage`
  - `KeZeroPages`
  - `RtlpLookupFunctionEntryForStackWalks`
- Einfluss der Speicherzugriffsmuster auf die reale CPU-Leistung

##  Unit Tests

Im Ordner `/tests` befinden sich Doctest-basierte Unittests für:

- `measure_access_time`
- `run_measurements`

Die Tests werden beim Build als separates Executable `unit_tests` erzeugt.

---
