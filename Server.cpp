#include <sys/socket.h>
#include <algorithm>
#include "Server.h"
#include "Helper.h"


int Server::from_client() {
    return polls[0].revents & (POLLIN | POLLERR);
}

int Server::to_client() {
    return polls[0].revents & POLLOUT;
}

void Server::RestartGame() {
    game_id = randGenerator.random();
    pixel_map.clear();

    events.clear();

    lobby.remove_if([](std::shared_ptr<Player> const& p) {
       return p->isDisconnected();
    });

    lobby.sort(
              [] (const std::shared_ptr<Player>& lhs, const std::shared_ptr<Player>& rhs) {
                  return lhs->getName() < rhs->getName();
              });
    std::shared_ptr<Payload> new_game = GenerateEvent(0);
    uint32_t len = 8;
    new_game->add32(maxx);
    new_game->add32(maxy);

    uint8_t id= 0;
    for(std::shared_ptr<Player> &player : lobby) {
        player->setID(id++);
        new_game->add_str(player->getName(), true);
        len += player->getName().length()+1;
    }

    PackEvent(new_game, len);

    SendToAll(new_game, true);

    for(std::shared_ptr<Player>& player : lobby) {
        player->setX((randGenerator.random() % maxx) + 0.5);
        player->setY((randGenerator.random() % maxy) + 0.5);
        player->setRotation(randGenerator.random() % 360);
        players.push_front(player);
    }

    lobby.clear();

    in_game = true;
}

void Server::GameOver() {
    auto game_over = GenerateEvent(3);
    PackEvent(game_over, 0);
    SendToAll(game_over);
    in_game = false;
}

void Server::Pixel(Player& player) {
    auto pixel = GenerateEvent(1);
    pixel->add8(player.getId());
    pixel->add32(player.getFakex());
    pixel->add32(player.getFakey());
    PackEvent(pixel, 9);
    pixel_map.insert(std::make_pair(player.getFakex(), player.getFakey()));
    SendToAll(pixel);
}

void Server::PlayerEliminated(Player &player) {
    auto eliminated = GenerateEvent(2);
    eliminated->add8(player.getId());
    PackEvent(eliminated, 1);
    SendToAll(eliminated);
}

bool Server::out_of_map(Player& player) const {
    return player.getFakey() >= maxy || player.getFakex() >= maxx;
}

void Server::GameLoop()
{
    for (auto itr = players.begin(); itr != players.end();) {
        Player& player = **itr;
        bool fake_pos_changed = player.Move(TURNING_SPEED);
        if(fake_pos_changed) {
            if(out_of_map(player) || collision(player)) {
                lobby.push_back(*itr);
                PlayerEliminated(player);
                itr = players.erase(itr);

                if(players.size() == 1) {
                    lobby.push_back(*players.begin());

                    for(auto player2 : lobby)
                        (*player2).setReady(false);

                    itr = players.erase(players.begin());

                    GameOver();
                }
            }
            else {
                Pixel(player);
                itr++;
            }
        }
        else
            ++itr;
    }
}

void Server::Run() {
    do {
        poll(polls, 1, 1);

        if(from_client()) {
            Address address;
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(sockaddr_storage);
            Payload p(128);
            ssize_t dglen = recvfrom(polls[0].fd, p.getData(), 128, 0,
                                     (sockaddr*)address.addr(), address.len_ptr());

            if (dglen == -1) {
                std::cout << "Read error\n";
            }
            else {
                p.trim(dglen);

                if (dglen >= 8+1+4) {
                    uint64_t session_id = p.read_uint64();
                    uint8_t turn_direction = p.read_uint8();
                    uint32_t next_expected_event_no = p.read_uint32();
                    std::string player_name = p.read_string(false);

                    auto itr = clients.find(address);
                    if (itr == clients.end()) {

                        bool incorrect_nick = false;

                        for (auto& player : clients)
                            if (player.second->getName().length() > 0 && player.second->getName() == player_name)
                                incorrect_nick = true;

                        if (!incorrect_nick) {
                            auto player = std::make_shared<Player>(address, player_name, session_id, next_expected_event_no, turn_direction);

                            clients.insert(std::make_pair(address, player));

                            if (player_name.length() > 0)
                                lobby.push_back(player);

                            SendMessagesToPlayer(player);
                            if (turn_direction != 0)
                                player->setReady(true);
                        }
                    }
                    else {
                        if (itr->second->getSession_id() == session_id) {
                            itr->second->setTurn_direction(turn_direction);
                            itr->second->setNext_expected_event_no(next_expected_event_no);
                            itr->second->setLast_message(Helper::GetTimeStamp());
                            if (turn_direction != 0 && !itr->second->isReady())
                            {
                                itr->second->setReady(true);
                            }
                        } else if (itr->second->getSession_id() < session_id) {
                            lobby.remove(itr->second);
                            players.remove(itr->second);
                            clients.erase(itr);

                            auto player = std::make_shared<Player>(address, player_name, session_id,
                                                                   next_expected_event_no, turn_direction);
                            clients.insert(std::make_pair(address, player));

                            if (player_name.length() > 0)
                                lobby.push_back(player);

                            SendMessagesToPlayer(player);
                            if (turn_direction != 0)
                                player->setReady(true);
                        }
                    }
                }
            }

            if (!in_game && lobby.size() >= 2) {
                if (PlayersAreReady()) {
                    RestartGame();
                }
            }

            polls[0].revents &= ~(POLLIN|POLLERR);
        }

        if (to_client()) {
            sendto(polls[0].fd, payloadsToSend.front().second->getPos(), payloadsToSend.front().second->getLen(), 0,
                   (sockaddr *) payloadsToSend.front().first.addr(), payloadsToSend.front().first.len());
            payloadsToSend.pop();
            polls[0].revents &= ~POLLOUT;
            if (payloadsToSend.empty())
                polls[0].events &= ~POLLOUT;
        }


        if (in_game && Helper::GetTimeStamp()-last_send >= 1000*1000/rounds_per_second) {
            GameLoop();
            last_send = Helper::GetTimeStamp();
        }

        if (Helper::GetTimeStamp()-last_check_timeout >= 500 * 1000) {
            uint64_t now = Helper::GetTimeStamp();
            for (auto itr = clients.begin(); itr != clients.end();) {
                if (now - (*itr).second->getLast_message() > 2 * 1000 * 1000) {
                    lobby.remove(itr->second);
                    itr->second->setTurn_direction(0);
                    itr->second->setDisconnected(true);
                    itr = clients.erase(itr);
                }
                else
                    ++itr;
            }
            last_check_timeout = now;
        }

    } while (1);

}
bool Server::collision(Player &player) {
    return pixel_map.find(std::make_pair(player.getFakex(), player.getFakey())) != pixel_map.end();
}

std::shared_ptr<Payload> Server::GenerateEvent(uint8_t event_type) {
    auto ptr = std::make_shared<Payload>(0);
    uint32_t event_no = events.size();

    ptr->add32(event_no);
    ptr->add8(event_type);

    return ptr;
}

void Server::PackEvent(std::shared_ptr<Payload> ptr, uint32_t len) {
    ptr->add_first(len+4+1);
    uint32_t crc32 = Helper::GetCrc32(ptr->getPos(), len+4+4+1);
    ptr->add32(crc32);
}

void Server::SendMessagesToPlayer(std::shared_ptr<Player> ptr) {
    uint32_t i = ptr->getNext_expected_event_no();

    while (i < events.size()) {
        std::shared_ptr<Payload> payload = std::make_shared<Payload>(0);
        payload->add32(game_id);

        while (i < events.size() && payload->getLen() + events[i]->getLen() <= 512) {
            payload->add(events[i]->getPos(), events[i]->getLen());
            ++i;
        }

        Send(ptr->getAddress(), payload);
    }
}


void Server::SendToAll(std::shared_ptr<Payload> p, bool all) {
    events.push_back(p);

    for (auto client : clients) {
        if (all)
            client.second->setNext_expected_event_no(0);
        SendMessagesToPlayer(client.second);
    }
}

void Server::Send(Address const &address, std::shared_ptr<Payload> data) {
    payloadsToSend.push(std::make_pair(address, data));
    polls[0].events |= POLLOUT;
}

bool Server::PlayersAreReady() {
    for(auto player : lobby) {
        if (!player->isReady())
            return false;
    }
    return true;
}

Server::Server(int argc, char **argv) : in_game(false), last_send(0), last_check_timeout(0), randGenerator(0)
{
    maxx = 800;
    maxy = 600;
    uint32_t port = 12345;
    uint32_t seed = time(NULL);
    int opt;
    while ((opt = getopt(argc, argv, "W:H:p:s:t:r:")) != -1) {
        switch (opt) {
            case 'W':
                maxx = (uint32_t) atol(optarg);
                break;
            case 'H':
                maxy = (uint32_t) atol(optarg);
                break;
            case 'p':
                port = (uint32_t) atol(optarg);
                break;
            case 's':
                rounds_per_second = (uint16_t) atoi(optarg);
                break;
            case 't':
                TURNING_SPEED = (uint16_t) atoi(optarg);
                break;
            case 'r':
                seed = (uint32_t) atol(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-W n] [-H n] [-p n] [-s n] [-t n] [-r n]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }



    randGenerator.setSeed(seed);

    fd = socket(AF_INET6, SOCK_DGRAM, 0);

    sockaddr_in6 ip;
    memset(&ip, 0, sizeof(sockaddr_in6));
    ip.sin6_family = AF_INET6;
    ip.sin6_port = htobe16(port);
    ip.sin6_addr = in6addr_any;

    if (bind(fd, (sockaddr*)&ip, sizeof(ip))<0) {
        std::cout << "cannot bind on port \n";
        exit(1);
    }

    polls[0].fd = fd;
    polls[0].events = POLLIN | POLLERR;
    polls[0].revents = 0;
}

