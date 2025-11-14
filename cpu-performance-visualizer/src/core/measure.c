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

    // Startzeitpunk in CPU Zyklen
    uint64_t start = __rdtscp(&aux);

    if (mode == 0) {
        // Zeilenweise (ROW MAJOR FRIENDLY)
        // Durchläuft die Matrix in der Reihenfolge, wie sie im Speicher liegt, Linear
        for (size_t i = 0; i < N; i++) {
            int *row = &data[i * N];
            for (size_t j = 0; j < N; j++) {
                row[j] += 1;
            }
        }
    } else {
        // Spaltenweise ( Row Major feindlich)
        // Springt durch den Speicher in großen Abständen: data[0*N + j], data[1*N + j], data[2*N + j], etc.
        for (size_t j = 0; j < N; j++) {
            int *col = &data[j];
            for (size_t i = 0; i < N; i++) {
                col[i * N] += 1;
            }
        }
    }
    // Endzeitpunkt wird gespeichert und mit start Variable subtrahiert um gelaufene Zyklen zu erfassen
    uint64_t end = __rdtscp(&aux);
    return end - start;
}

