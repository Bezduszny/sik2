#include "Payload.h"

Payload::Payload(size_t len) {
    pos = 0;
    data.resize(len);
}

Payload::Payload(Payload const &p) {
    *this = p;
    pos = 0;
}

Payload &Payload::operator=(Payload const &other) {
    data = other.data;
    return *this;
}

char *Payload::getData() {
    return data.data();
}

size_t Payload::getLen() {
    return data.size();
}

void Payload::trim(size_t len) {
    if (data.size() > len)
        data.resize(len);
}

void Payload::add32(uint32_t val) {
    val = htobe32(val);
    add(&val, sizeof(val));
}

uint32_t Payload::read_uint32() {
    if (pos > data.size()-4)
        throw std::exception();
    uint32_t val = *(uint32_t*)(data.data()+pos);
    pos += 4;
    return be32toh(val);
}

std::string Payload::read_string(bool until0) {
    std::string str;

    while (hasData() && (*(data.data()+pos) != 0 || !until0)) {
        str += *(data.data()+pos);
        pos++;
    }

    if (until0)
        pos++;
    return str;
}

std::string Payload::read_str(size_t len) {
    std::string str;
    int i = 0;
    while (hasData() && i < len) {
        str += *(data.data()+pos);
        pos++;
        i++;
    }

    return str;
}

void Payload::add8(int8_t val) {
    add(&val, sizeof(val));
}

uint8_t Payload::read_uint8() {
    uint8_t val = *(uint8_t*)(data.data()+pos);
    pos++;
    return val;
}

void Payload::add_str(std::string const &val, bool with0) {
    add((void*)val.c_str(), val.length());
    if (with0) {
        char zero = 0;
        add(&zero, 1);
    }
}

void Payload::add64(uint64_t val) {
    val = htobe64(val);
    add(&val, sizeof(val));
}

uint64_t Payload::read_uint64() {
    uint64_t val = *(uint64_t*)(data.data()+pos);
    pos += sizeof(val);
    return be64toh(val);
}

uint32_t Payload::read_crc(size_t len) {
    if (pos+len >= data.size())
        throw std::exception();
    uint32_t val = *(uint32_t*)(data.data()+pos+len);
    return be32toh(val);
}

void Payload::add(void *data, size_t len) {
    for(int i = 0; i < len; i++) {
        this->data.push_back(((char*)data)[i]);
    }
}

bool Payload::hasData() const {
    return pos < data.size();
}

uint64_t Payload::getRemainingData() {
    return data.size() - pos;
}

void Payload::add_first(void* val, size_t len) {
    for (int64_t i = len - 1; i >= 0; --i)
        this->data.insert(data.begin(), ((char*)val)[i]);
}

void Payload::add_first(uint32_t val) {
    uint32_t v = htobe32(val);
    add_first(&v, sizeof(uint32_t));
}

void* Payload::getPos() {
    return (void*)(data.data()+pos);
}
