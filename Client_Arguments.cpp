#include "Client_Arguments.h"
#include "Helper.h"

Client_Arguments::Client_Arguments(int argc, char **argv)  {
    if(argc <= 2) {
        std::cout << "Wrong number of arguments";
        exit(1);
    }

    player_name = argv[1];
    Helper::validate_playername(player_name);

    std::string server_name = argv[2];
    uint16_t server_port = 12345;
    split_address(server_name, server_port);
    server = Address(server_name, server_port);

    std::string gui_server_name = "localhost";
    uint16_t gui_server_port = 12346;

    if(argc == 4) {
        gui_server_name = argv[3];
        split_address(gui_server_name, gui_server_port);
    }
    gui = Address(gui_server_name, gui_server_port);
}

std::string Client_Arguments::get_playername() {
    return player_name;
}

Address Client_Arguments::get_gui() {
    return gui;
}

Address Client_Arguments::get_server() {
    return server;
}

void Client_Arguments::split_address(std::string &name, uint16_t &port) {
    long n = std::count(name.begin(), name.end(), ':');

    if (n == 1) {
        port = (uint16_t) atoi(name.substr(name.find(":") + 1).c_str());
        name = name.substr(0, name.find(":"));
    }
}

Client_Arguments::Client_Arguments() {

}
