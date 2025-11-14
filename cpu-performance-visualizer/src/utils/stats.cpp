// stats.cpp
#include "stats.h"
#include <algorithm>
#include <cmath>

double mean(const std::vector<uint64_t> &v) {
    double sum = 0;
    for (auto val: v) sum += val;
    return v.empty() ? 0 : sum / v.size();
}

double median(std::vector<uint64_t> v) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    size_t mid = v.size() / 2;
    return (v.size() % 2) ? v[mid] : (v[mid - 1] + v[mid]) / 2.0;
}

double stddev(const std::vector<uint64_t> &v) {
    if (v.size() < 2) return 0;
    double m = mean(v);
    double acc = 0;
    for (auto val: v) acc += (val - m) * (val - m);
    return std::sqrt(acc / (v.size() - 1));
}
