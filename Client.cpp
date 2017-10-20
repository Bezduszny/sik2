#include "Client.h"
#include "Helper.h"
#include <boost/crc.hpp>

void Client::Run() {
    do
    {
        poll(polls, 2, 1);

        if (from_server()) {
            sockaddr_storage addr;
            socklen_t addr_len = sizeof(sockaddr_storage);
            Payload p(512);
            ssize_t dglen = recvfrom(polls[0].fd, p.getData(), 512, 0,
                                   (sockaddr*)&addr, &addr_len);

            if (dglen == -1)
            {
                if (errno == 111)
                {
                    std::cout << "connection with server lost\n";
                    exit(1);
                }
                else
                    std::cout << "Read error\n";

            }
            else
            {
                p.trim(dglen);
                uint32_t game_id = p.read_uint32();
                bool no_crc_error;

                while (p.hasData()) {
                    uint32_t len = p.read_uint32();
                    no_crc_error = check_crc((char*)p.getPos()-4, len+4, p.read_crc(len));
                    if(!no_crc_error)
                        break;
                    uint32_t event_no = p.read_uint32();

                    bool wrong_event_no=false;
                    if (event_no == next_expected_event_no || event_no == 0)
                        next_expected_event_no = event_no+1;
                    else {
                        if (event_no > next_expected_event_no)
                            break;
                        else
                        {
                            wrong_event_no = true;
                        }
                    }

                    char event_type = p.read_uint8();

                    if(!event_type || game_id == current_game_id) {
                        switch (event_type) {
                            case 0: {
                                uint32_t maxx = p.read_uint32();
                                uint32_t maxy = p.read_uint32();
                                game_maxx = maxx;
                                game_maxy = maxy;
                                std::stringstream ss;
                                ss << " NEW_GAME " << maxx << " " << maxy;
                                uint32_t parsed_data = 13;
                                while (parsed_data < len) {
                                    std::string name = p.read_string(true);
                                    Helper::validate_playername(name);
                                    ss << " " << name;
                                    names.push_back(name);
                                    parsed_data += name.length()+1;
                                }
                                ss << "\n";

                                guicommands.push(ss.str());
                                polls[1].events |= POLLOUT;
                                current_game_id = game_id;
                                break;
                            }
                            case 1: {
                                uint8_t player_number = p.read_uint8();
                                uint32_t x = p.read_uint32();
                                uint32_t y = p.read_uint32();
                                if(x > game_maxx || y > game_maxy) {
                                    std::cout << "Player out of map";
                                    exit(1);
                                }
                                if(player_number >= names.size()) {
                                    std::cout << "Wrong player number";
                                    exit(1);
                                }

                                std::stringstream ss;
                                ss << " PIXEL " << x << " " << y << " " << names[player_number] << "\n";
                                if(!wrong_event_no) {
                                    guicommands.push(ss.str());
                                    polls[1].events |= POLLOUT;
                                }
                                break;
                            }
                            case 2: {
                                uint8_t player_number = p.read_uint8();
                                std::stringstream ss;
                                ss << " PLAYER_ELIMINATED " << names[player_number] << "\n";
                                if(!wrong_event_no) {
                                    guicommands.push(ss.str());
                                    polls[1].events |= POLLOUT;
                                }
                                break;
                            }
                            case 3:
                                break;
                            default:
                                p.read_str(len-5);
                                break;

                        }
                    }
                    else
                        std::cout << "Incorrect game id\n";
                    p.read_uint32();
                }
            }

            polls[0].revents &= ~(POLLIN|POLLERR);
        }

        if (from_gui()) {
            char bufor[500];
            ssize_t len = recv(polls[1].fd, bufor, 500, 0);

            if (len == 0) {
                std::cout << "connection with gui lost\n";
                exit(1);
            }

            for (int i = 0; i < len; ++i)
            {
                if (bufor[i] == '\n')
                {
                    if(message == "RIGHT_KEY_DOWN")
                        turn_direction = 1;
                    else if(message == "LEFT_KEY_DOWN")
                        turn_direction = -1;
                    else if(message == "LEFT_KEY_UP" || message == "RIGHT_KEY_UP")
                        turn_direction = 0;

                    message = "";
                }
                else if (bufor[i] != '\0' && message.length() < 30)
                    message += bufor[i];
            }
        }

        if (to_server())
        {
            sendto(polls[0].fd, payloads.front().getData(), payloads.front().getLen(), 0,
                   (sockaddr *) ca.get_server().addr(), ca.get_server().len());
            payloads.pop();
            polls[0].revents &= ~POLLOUT;
            if (payloads.empty())
                polls[0].events &= ~POLLOUT;
        }

        if (to_gui())
        {
            send(polls[1].fd, guicommands.front().c_str(), guicommands.front().length(), 0);
            guicommands.pop();
            polls[1].revents &= ~POLLOUT;
            if (guicommands.empty())
                polls[1].events &= ~POLLOUT;
        }

        if (Helper::GetTimeStamp()-last_client_info_send >= 20000)
        {
            Payload p;
            p.add64(session_id);
            p.add8(turn_direction);
            p.add32(next_expected_event_no);
            p.add_str(ca.get_playername(), false);



            payloads.push(p);
            polls[0].events |= POLLOUT;
            last_client_info_send = Helper::GetTimeStamp();
        }
    } while(1);
}

int Client::from_server() {
    return polls[0].revents & (POLLIN | POLLERR);
}

int Client::to_server() {
    return polls[0].revents & POLLOUT;
}

int Client::from_gui() {
    return polls[1].revents & POLLIN;
}

int Client::to_gui() {
    return polls[1].revents & POLLOUT;
}

void Client::InitPolls() {
    polls[0].fd = server_fd;
    polls[0].events = POLLIN;
    polls[0].revents = 0;

    polls[1].fd = gui_fd;
    polls[1].events = POLLIN;
    polls[1].revents = 0;
}

void Client::ConnectToGui() {
    if (connect(gui_fd, (sockaddr *)ca.get_gui().addr(), sizeof (sockaddr_in6))<0) {
        std::cout << "nie mozna polaczyc sie z gui\n";
        exit(1);
    }
}

void Client::OptimizeGuiSocket() {
    int flag = 1;
    setsockopt(gui_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
}

void Client::ConnectToServer() {
    if ((connect(server_fd, (sockaddr *)ca.get_server().addr(), ca.get_server().len())) < 0) {
        std::cout << "Nie mozna polaczyc sie z serwerem\n";
        exit(1);
    }
}


#include <fcntl.h>
Client::Client(int argc, char **argv) {
    ca = Client_Arguments(argc, argv);
    session_id = Helper::GetTimeStamp();
    turn_direction = 0;
    next_expected_event_no = 0;
    last_client_info_send = Helper::GetTimeStamp()-20000;
    server_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    int flags = fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK;
    fcntl(server_fd, F_SETFL, flags);
    ConnectToServer();
    gui_fd = socket(ca.get_gui().get_family(), SOCK_STREAM, 0);
    ConnectToGui();
    InitPolls();
    OptimizeGuiSocket();
    message = "";
}



bool Client::check_crc(void *pos, ssize_t len, uint32_t crc32) {
    int crc32_computed = Helper::GetCrc32(pos, len);
    return crc32 == crc32_computed;
}
