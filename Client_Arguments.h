#ifndef SIK2_CLIENT_ARGUMENTS_H
#define SIK2_CLIENT_ARGUMENTS_H

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <sstream>
#include <algorithm>
#include "Address.h"

class Client_Arguments {
private:
    Address server;
    Address gui;
    std::string player_name;

    void split_address(std::string &name, uint16_t &port);


public:
    Address get_server();
    Address get_gui();
    std::string get_playername();
    Client_Arguments(int argc, char *argv[]);
    Client_Arguments();
};
#endif //SIK2_CLIENT_ARGUMENTS_H
