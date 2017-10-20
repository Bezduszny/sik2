#ifndef SIK2_RANDGENERATOR_H
#define SIK2_RANDGENERATOR_H


class RandGenerator {
public:
    uint32_t random();
    RandGenerator(uint32_t seed);

    void setSeed(uint32_t val);

private:
    uint64_t seed;
};


#endif //SIK2_RANDGENERATOR_H
