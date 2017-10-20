#include <boost/crc.hpp>
#include "Helper.h"

uint64_t Helper::GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void Helper::validate_playername(std::string name) {
    for(int i = 0; i < name.length(); i++) {
        if(name[i] < 33 || name[i] > 126) {
            std::cout << "Wrong player name\n";
            exit(1);
        }
    }
}

uint32_t Helper::GetCrc32(const void* buffer, size_t len) {
    boost::crc_32_type result;
    result.process_bytes(buffer, len);
    return result.checksum();
}