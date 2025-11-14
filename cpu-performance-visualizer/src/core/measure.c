#include "measure.h"
#include <x86intrin.h> // enthält _rdtscp() für Zugriff auf CPU Zeit
#include <stdint.h> // definiert feste Ganzzahltypen
#include <math.h>
// Misst die Cpu Zeit in Taktzyklen, die ein bestimmter Speicherzugriff benötigt

uint64_t measure_access_time(int *data, size_t N, int mode) {
    unsigned int aux;
    uint64_t start = __rdtscp(&aux);

    if (mode == 0) {
        // Zeilenweise
        for (size_t i = 0; i < N; i++) {
            int *row = &data[i * N];
            for (size_t j = 0; j < N; j++) {
                row[j] += 1;
            }
        }
    } else {
        // Spaltenweise
        for (size_t j = 0; j < N; j++) {
            int *col = &data[j];
            for (size_t i = 0; i < N; i++) {
                col[i * N] += 1;
            }
        }
    }

    uint64_t end = __rdtscp(&aux);
    return end - start;
}

