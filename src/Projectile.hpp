#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

#include "Grid.hpp"
#include "Enemy.hpp"

struct Projectile { //Структура для пулек башен, они у всех одинаковые
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;
    int damage;
    bool alive;
    Enemy* target;//У них есть скорость, урон, есть они или нет и цель, за которой они будут лететь (из за этого есть некоторые баги, но я не знаю пока что как их исправить, они не критичные)

    Projectile(sf::Vector2f startPos, Enemy* tgt, float spd, int dmg);

    void update(float dt);
    void draw(sf::RenderTarget& rt) const;//Далее тут системные функции для её двиджения, вычитания хп у таргета, отрисовкеи и пропадения после того как она попала во врага
};