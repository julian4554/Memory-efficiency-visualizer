#include "measure_runner.h"
#include "measure.h"
#include "../utils/stats.h"
#include <windows.h>
#include <immintrin.h>
#include <vector>
#include <cstdint>
#include <string>
#include <functional>
#include <cstdio>
#include <iostream>
// -------------------------------------------------------------
// Cache vollständig leeren, um Messungen unabhängig vom Cachezustand zu machen
// -------------------------------------------------------------
static void flush_cache(int *data, size_t total) {
    for (size_t i = 0; i < total; i += 16)
        _mm_clflush(&data[i]);
    _mm_mfence();
}

// -------------------------------------------------------------
// Führt Messungen für eine bestimmte Matrixgröße N durch
// und gibt die Ergebnisse (durchschnitt, median, stddev) zurück
// -------------------------------------------------------------
std::vector<MeasureResult> run_measurements(
    size_t N,
    std::function<void(const std::string &)> onUpdate) {
    // Prozess priorisieren und an festen Kern binden
    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessAffinityMask(process, 1ULL << 2);

    std::vector<MeasureResult> results;

    // Matrix erzeugen und vorbereiten
    std::vector<int> matrix(N * N, 0);
    const size_t total = N * N;

    std::vector<uint64_t> zeile, spalte;
    zeile.reserve(30);
    spalte.reserve(30);

    // ---------------------------------------------------------
    // Messschleife: Zeilen- und Spaltenzugriff 30×
    // ---------------------------------------------------------
    for (int repeat = 0; repeat < 30; ++repeat) {
        flush_cache(matrix.data(), total);
        zeile.push_back(measure_access_time(matrix.data(), N, 0));

        flush_cache(matrix.data(), total);
        spalte.push_back(measure_access_time(matrix.data(), N, 1));
    }

    // Statistik berechnen
    double avg_row = mean(zeile);
    double avg_col = mean(spalte);
    double med_row = median(zeile);
    double med_col = median(spalte);
    double std_row = stddev(zeile);
    double std_col = stddev(spalte);
    double ratio = (avg_row > 0) ? (avg_col / avg_row) : 0.0;

    results.push_back({
        N, avg_row, avg_col, med_row, med_col,
        std_row, std_col, ratio
    });

    // Live-Callback (z. B. für GUI-Konsole)
    if (onUpdate) {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "%zux%zu  Zeilenweise  %.0f CPU-Zyklen | Spaltenweise  %.0f CPU-Zyklen | Verhältnis %.2fx\n",
                 N, N, avg_row, avg_col, ratio);
        onUpdate(std::string(buf));

        switch (N) {
            case 256:
                onUpdate("128x128: Vollständig im L1-Cache.\n");
                break;
            case 512:
                onUpdate(
                    "512x512: Passt in den L2-Cache.\n");
                break;
            case 2048:
                onUpdate(
                    "2048x2048: Entspricht etwa 16MB, nutzt L3-Cache.\n");
                break;
            case 4096:
                onUpdate(
                    "4096x4096: Überschreitet L3-Cache (~64 MB). RAM-Zugriffe beginnen, deutlich mehr Cache-Misses.\n");
                break;
            case 8192:
                onUpdate(
                    "8192x8192: Hauptspeicher dominiert. Spaltenzugriff stark verlangsamt durch Page Faults und ineffiziente Zugriffe.\n");
                break;
            default:
                break;
        }
    }
    return results;
}
