#include <cstdint>
#include "RandGenerator.h"

uint32_t RandGenerator::random() {
    seed *= uint64_t(279470273);
    seed %= 4294967291;
    return (uint32_t)seed;
}

RandGenerator::RandGenerator(uint32_t seed) : seed(seed) {}

void RandGenerator::setSeed(uint32_t val) {
    seed = val;
};
