#ifndef SIK2_PAYLOAD_H
#define SIK2_PAYLOAD_H


#include <vector>
#include <cstdint>
#include <cstdio>
#include <zconf.h>
#include <iostream>

class Payload {
public:
    Payload(size_t len = 0);

    Payload(Payload const& p);

    Payload& operator=(Payload const& other);

    char* getData();

    void* getPos();

    size_t getLen();

    void trim(size_t len);

    void add32(uint32_t val);

    uint32_t read_uint32();

    std::string read_string(bool until0);

    void add8(int8_t val);

    uint8_t read_uint8();

    void add_str(std::string const &val, bool with0);

    void add64(uint64_t val);

    uint64_t read_uint64();

    void add(void* data, size_t len);

    bool hasData() const;

    uint64_t getRemainingData();

    uint32_t read_crc(size_t len);

    std::string read_str(size_t len);

    void add_first(uint32_t val);

    void add_first(void *val, size_t len);
private:
    std::vector<char> data;
    size_t pos;
};

#endif //SIK2_PAYLOAD_H
