// Simple SDL2 GUI interface for the CPU Performance Visualizer
// Created by julia on 07.11.2025.

#ifndef CPU_PERFORMANCE_VISUALIZER_GUI_H
#define CPU_PERFORMANCE_VISUALIZER_GUI_H

namespace gui {
    // Runs a minimal SDL2 window showing an empty canvas until the user closes it.
    // Returns 0 on success, non-zero on initialization or runtime error.
    int run();
}

#endif //CPU_PERFORMANCE_VISUALIZER_GUI_H
