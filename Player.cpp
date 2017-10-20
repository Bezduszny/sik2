#include "Player.h"
#include "Helper.h"
#include <cmath>
std::string Player::getName() const{
    return name;
}

void Player::setName(const std::string &name) {
    Player::name = name;
}

uint64_t Player::getSession_id() const {
    return session_id;
}

void Player::setSession_id(uint64_t session_id) {
    Player::session_id = session_id;
}

uint32_t Player::getNext_expected_event_no() const {
    return next_expected_event_no;
}

void Player::setNext_expected_event_no(uint32_t next_expected_event_no) {
    Player::next_expected_event_no = next_expected_event_no;
}

char Player::getTurn_direction() const {
    return turn_direction;
}

void Player::setTurn_direction(char turn_direction) {
    Player::turn_direction = turn_direction;
}

double Player::getX() const {
    return x;
}

void Player::setX(double x) {
    Player::x = x;
    setFakex(x);
}

double Player::getY() const {
    return y;
}

void Player::setY(double y) {
    Player::y = y;
    setFakey(y);
}

int Player::getFakex() const {
    return fakex;
}

void Player::setFakex(int fakex) {
    Player::fakex = fakex;
}

int Player::getFakey() const {
    return fakey;
}

void Player::setFakey(int fakey) {
    Player::fakey = fakey;
}

void Player::setRotation(double val) {
    Player::rotation = val;
}

bool Player::Move(uint16_t turningSpeed) {
    rotation += double(turningSpeed)*double(turn_direction);
    int old_x = getFakex();
    int old_y = getFakey();
    setX(x+cos(rotation*M_PI/180.0));
    setY(y+sin(rotation*M_PI/180.0));

    return old_x != getFakex() || old_y != getFakey();
}

void Player::setID(uint8_t i) {
    id = i;
}

bool Player::isReady() const {
    return ready;
}

void Player::setReady(bool ready) {
    Player::ready = ready;
}

uint8_t Player::getId() const {
    return id;
}

Player::Player(const Address& address, const std::string &name, uint64_t session_id, uint32_t next_expected_event_no, int turn_direction)
        : address(address), name(name), session_id(session_id), next_expected_event_no(next_expected_event_no),
          turn_direction(turn_direction)
{
    setLast_message(Helper::GetTimeStamp());
    setDisconnected(false);
}

const Address &Player::getAddress() const {
    return address;
}

void Player::setAddress(const Address &address) {
    Player::address = address;
}

uint64_t Player::getLast_message() const {
    return last_message;
}

void Player::setLast_message(uint64_t last_message) {
    Player::last_message = last_message;
}

bool Player::isDisconnected() const {
    return disconnected;
}

void Player::setDisconnected(bool disconnected) {
    Player::disconnected = disconnected;
}
