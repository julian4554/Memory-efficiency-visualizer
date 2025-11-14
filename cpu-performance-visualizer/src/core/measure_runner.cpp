#include "measure_runner.h"
#include "measure.h"
#include <windows.h>
#include <immintrin.h>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>
#include <cstdio>
#include <iostream>


// Leert den CPU-Cache über CLFLUSH, um sicherzustellen, dass jede Messung unter
// identischen Bedingungen startet

static void flush_cache(int *data, size_t total) {
    for (size_t i = 0; i < total; i += 16)
        _mm_clflush(&data[i]);
    _mm_mfence();
}


// Führt Messungen für eine bestimmte Matrixgröße N durch und berechnet:
// - durchschnittliche Zeilenzeit
// - durchschnittliche Spaltenzeit
// - Verhältnis zwischen beiden

std::vector<MeasureResult> run_measurements(
    size_t N,
    std::function<void(const std::string &)> onUpdate) {
    // Hohe Prozesspriorität + CPU-Affinität für reproduzierbare Messungen (kern 2 ausgewählt)
    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessAffinityMask(process, 1ULL << 2);

    std::vector<MeasureResult> results;

    // Matrix erzeugen
    std::vector<int> matrix(N * N, 0);
    const size_t total = N * N;

    std::vector<uint64_t> zeile, spalte;
    zeile.reserve(30);
    spalte.reserve(30);


    // Messschleife jeweils 30 Wiederholungen für genuaigkeit und messrauschen reduzieren

    for (int repeat = 0; repeat < 30; ++repeat) {
        flush_cache(matrix.data(), total);
        zeile.push_back(measure_access_time(matrix.data(), N, 0));

        flush_cache(matrix.data(), total);
        spalte.push_back(measure_access_time(matrix.data(), N, 1));
    }


    // Durchschnitt berechnen

    double avg_row = 0;
    for (auto v: zeile) avg_row += v;
    avg_row /= zeile.size();

    double avg_col = 0;
    for (auto v: spalte) avg_col += v;
    avg_col /= spalte.size();

    // Verhältnis Spalte/Zeile
    double ratio = (avg_row > 0) ? (avg_col / avg_row) : 0.0;


    // Ergebnis speichern

    results.push_back({
        N,
        avg_row,
        avg_col,
        ratio
    });


    // Ausgabe an gui

    if (onUpdate) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "%zux%zu  Zeilenweise %.0f CPU-Zyklen | Spaltenweise %.0f CPU-Zyklen | Verhältnis %.2fx\n",
                 N, N, avg_row, avg_col, ratio);
        onUpdate(std::string(buf));

        // Cache Level Hinweise für die gui
        switch (N) {
            case 256:
                onUpdate("256x256: Vollständig im L1-Cache.\n");
                break;
            case 512:
                onUpdate("512x512: Passt in den L2-Cache.\n");
                break;
            case 2048:
                onUpdate("2048x2048: Entspricht ~16MB (L3-Cache).\n");
                break;
            case 4096:
                onUpdate("4096x4096: Überschreitet L3, RAM-Zugriffe.\n");
                break;
            case 8192:
                onUpdate("8192x8192: Hauptspeicher dominiert.\n");
                break;
            default:
                break;
        }
    }

    return results;
}
