#include "measure.h"
#include <x86intrin.h> // enthält _rdtscp() für Zugriff auf CPU Zeit
#include <stdint.h> // definiert feste Ganzzahltypen

// Misst die Cpu Zeit in Taktzyklen, die ein bestimmter Speicherzugriff benötigt.
// Durchlauf einer NxN-Matrix.
// Die Matrix wird in "data" abgelegt und wird zeilenweise (mode 0) oder spaltenweise (mode 1) durchlaufen.
// Parameter:
// - data: POinter auf den linearen Speicher der Matrix
// - N: Dimension der Matrix (NxN) Hinweis: Für uns Menschen meisten 2D, für Computer 1D; Linear im Speicher liegend
// - mode: zeile (0) vs Spalte (1)
// Die Messung verwendet __rdtscp(), für das Auslesen des Taktzählers. Die Differenz zwischen Start und Endwert entspricht den benötigten CPU-Zyklen


uint64_t measure_access_time(int *data, size_t N, int mode) {
    unsigned int aux;
    volatile int sink = 0; // verhindert Optimierungen des compilers im release mode

    uint64_t start = __rdtscp(&aux);

    if (mode == 0) {
        for (size_t i = 0; i < N; i++) {
            int *row = &data[i * N];
            for (size_t j = 0; j < N; j++) {
                row[j] += 1;
                sink += row[j];
                // zwingt den Compiler, die Operation wirklich auszuführen (fix um das problem des debug/release mode zu lösen) compiler würde sonst optimieren und überspringen
            }
        }
    } else {
        for (size_t j = 0; j < N; j++) {
            int *col = &data[j];
            for (size_t i = 0; i < N; i++) {
                col[i * N] += 1;
                sink += col[i * N]; // auch hier
            }
        }
    }

    uint64_t end = __rdtscp(&aux);

    // kleine Nutzung, damit sink wirklich im final code bleibt
    (void) sink;

    return end - start;
}
