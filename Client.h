#ifndef SIK2_CLIENT_H
#define SIK2_CLIENT_H


#include <sys/time.h>
#include <netinet/tcp.h>
#include <cstring>
#include <poll.h>
#include "Client_Arguments.h"
#include "Payload.h"

class Client {
public:
    Client(int argc, char *argv[]);

    void Run();

private:
    Client_Arguments ca;
    uint64_t session_id;
    int8_t turn_direction = 0;
    uint32_t next_expected_event_no = 0;
    int server_fd;
    int gui_fd;
    pollfd polls[2];
    std::queue<Payload> payloads;
    std::queue<std::string> guicommands;
    std::vector<std::string> names;
    uint64_t last_client_info_send;
    uint32_t current_game_id;
    uint32_t game_maxx;
    uint32_t game_maxy;
    std::string message;



    uint64_t GetTimeStamp();;

    int from_server();

    int to_server();

    int from_gui();

    int to_gui();

    void InitPolls();

    void ConnectToGui();

    void OptimizeGuiSocket();



    bool check_crc(void *pVoid, ssize_t len, uint32_t crc);

    void ConnectToServer();
};


#endif //SIK2_CLIENT_H
