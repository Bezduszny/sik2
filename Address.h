#ifndef SIK2_ADDRESS_H
#define SIK2_ADDRESS_H
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <netdb.h>

class Address {
public:
    Address();
    Address(std::string name, uint16_t port);
    socklen_t & len();
    sockaddr_storage* addr();
    socklen_t len() const;
    sockaddr_storage const* addr() const;
    int get_family() const;

    friend std::ostream &operator<<(std::ostream &os, Address const& address) {
        char buffer[INET6_ADDRSTRLEN];
        char port[8];
        getnameinfo((sockaddr const*)&address.address, address.length, buffer, sizeof(buffer), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
        os << buffer << ":" << port;
        return os;
    }

    socklen_t *len_ptr();

private:
    sockaddr_storage address;
    socklen_t length;

public:
    bool operator<(const Address &rhs) const;

    bool operator>(const Address &rhs) const;

    bool operator<=(const Address &rhs) const;

    bool operator>=(const Address &rhs) const;
};


#endif //SIK2_ADDRESS_H
