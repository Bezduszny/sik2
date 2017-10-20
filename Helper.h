#ifndef SIK2_HELPER_H
#define SIK2_HELPER_H


#include <cstdint>
#include <sys/time.h>
#include <iostream>

class Helper {
public:
    static uint64_t GetTimeStamp();

    static void validate_playername(std::string name);

    static uint32_t GetCrc32(const void *buffer, size_t len);

};


#endif //SIK2_HELPER_H
