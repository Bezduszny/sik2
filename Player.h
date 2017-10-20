#ifndef SIK2_PLAYER_H
#define SIK2_PLAYER_H
#include <iostream>
#include "Address.h"

class Player {
private:
    std::string name;
    uint64_t session_id;
    uint32_t next_expected_event_no;
    char turn_direction;
    double x, y;
    double rotation;
    bool ready;
    Address address;
    uint64_t last_message;
    bool disconnected;
public:
    bool isDisconnected() const;

    void setDisconnected(bool disconnected);

public:
    uint64_t getLast_message() const;

    void setLast_message(uint64_t last_message);

public:
    const Address &getAddress() const;

    void setAddress(const Address &address);

public:
    Player(const Address& address, const std::string &name, uint64_t session_id, uint32_t next_expected_event_no, int turn_direction);

private:
    int fakex, fakey;
    uint8_t id;
public:
    bool Move(uint16_t turningSpeed);

    void setName(const std::string &name);

    void setRotation(double val);

    uint64_t getSession_id() const;

    void setSession_id(uint64_t session_id);

    uint32_t getNext_expected_event_no() const;

    void setNext_expected_event_no(uint32_t next_expected_event_no);

    char getTurn_direction() const;

    void setTurn_direction(char turn_direction);

    double getX() const;

    void setX(double x);

    double getY() const;

    void setY(double y);

    int getFakex() const;

    void setFakex(int fakex);

    int getFakey() const;

    void setFakey(int fakey);

    std::string getName() const;

    void setID(uint8_t i);

    uint8_t getId() const;

    bool isReady() const;

    void setReady(bool ready);

};


#endif //SIK2_PLAYER_H
