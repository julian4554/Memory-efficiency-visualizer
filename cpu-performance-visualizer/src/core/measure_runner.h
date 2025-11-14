#pragma once

#include <vector>
#include <string>
#include <functional>

// -----------------------------------------
// Ergebnisstruktur für eine einzelne Messung
// -----------------------------------------
struct MeasureResult {
    size_t N;

    double avg_row;
    double avg_col;

    double med_row;
    double med_col;

    double std_row;
    double std_col;

    double ratio;
};

// -----------------------------------------
// Führt Messungen für ein NxN-Matrixproblem aus.
// - N: Seitenlänge der Matrix
// - onUpdate: Callback für Zwischenausgaben (GUI/Console)
// -----------------------------------------
std::vector<MeasureResult> run_measurements(
    size_t N,
    std::function<void(const std::string &)> onUpdate = nullptr
);
