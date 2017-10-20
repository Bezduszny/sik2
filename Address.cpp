#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include "Address.h"

Address::Address(std::string name, uint16_t port) {
    addrinfo request, *returned;
    memset(&request, 0, sizeof(addrinfo));
    request.ai_family = AF_UNSPEC;

    if(getaddrinfo(name.c_str(), std::to_string(port).c_str(), &request, &returned) != 0) {
        std::cout << "Nie można znaleźć hosta\n";
        exit(1);
    }

    memcpy(&address, returned->ai_addr, returned->ai_addrlen);
    length = returned->ai_addrlen;
    freeaddrinfo(returned);
}

Address::Address() {
    length = sizeof(address);
}

sockaddr_storage *Address::addr() {
    return &address;
}

socklen_t &Address::len() {
    return length;
}

int Address::get_family() const {
    return address.ss_family;
}

socklen_t *Address::len_ptr() {
    return &length;
}

bool Address::operator<(const Address &rhs) const {
    return memcmp(rhs.addr(), addr(), std::max(len(), rhs.len())) < 0;
}

bool Address::operator>(const Address &rhs) const {
    return rhs < *this;
}

bool Address::operator<=(const Address &rhs) const {
    return !(rhs < *this);
}

bool Address::operator>=(const Address &rhs) const {
    return !(*this < rhs);
}

socklen_t Address::len() const {
    return length;
}

sockaddr_storage const *Address::addr() const {
    return &address;
}

