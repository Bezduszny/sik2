#ifndef SIK2_SERVER_H
#define SIK2_SERVER_H


#include <poll.h>
#include <map>
#include <memory>
#include "Payload.h"
#include "Address.h"
#include "Player.h"
#include "RandGenerator.h"
#include <list>
#include <unordered_set>
#include <queue>
#include <set>
#include <cstring>

class Server {
public:
    int from_client();

    int to_client();

    void Run();

    Server(int argc, char *argv[]);

private:
    uint32_t maxx;
    uint32_t maxy;
    int fd;
    uint16_t rounds_per_second = 50;
    uint16_t TURNING_SPEED = 6;
    pollfd polls[1];
    uint32_t game_id;
    std::list<std::shared_ptr<Player> > lobby;
    std::list<std::shared_ptr<Player> > players;
    std::map<Address, std::shared_ptr<Player> > clients;
    std::vector<std::shared_ptr<Payload> > events;
    std::set <std::pair<uint32_t, uint32_t> > pixel_map;
    RandGenerator randGenerator;
    uint64_t last_send;
    uint64_t last_check_timeout;
    std::queue<std::pair<Address, std::shared_ptr<Payload> > > payloadsToSend;

    void RestartGame();

    void SendToAll(std::shared_ptr<Payload> p, bool all = false);

    void GameLoop();

    bool collision(Player &player);

    void GameOver();

    void Pixel(Player &player);

    bool out_of_map(Player &player) const;

    std::shared_ptr<Payload> GenerateEvent(uint8_t event_type);

    void PackEvent(std::shared_ptr<Payload> ptr, uint32_t len);

    void PlayerEliminated(Player &player);

    bool in_game;

    void SendMessagesToPlayer(std::shared_ptr<Player> shared_ptr);

    void Send(Address const &address, std::shared_ptr<Payload> data);

    bool PlayersAreReady();
};


#endif //SIK2_SERVER_H
