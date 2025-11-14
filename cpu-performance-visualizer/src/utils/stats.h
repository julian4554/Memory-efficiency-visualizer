// stats.h
#ifndef STATS_H
#define STATS_H

#include <vector>
#include <cstdint>

double mean(const std::vector<uint64_t>& values);
double median(std::vector<uint64_t> values);
double stddev(const std::vector<uint64_t>& values);

#endif
