#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <cstdint>

#include "Grid.hpp"

struct Enemy { //Структура для врагов со всем всем всем, размером, хитбоксом, замена текстуры (если нет картинки), отслеживанием позиции и остальным
    std::optional<sf::Sprite> spr;
    sf::CircleShape fallback;
    float speed;
    int hp;
    size_t pathIdx;
    bool alive; //Тут  конкретно скорость хп, местонахождение, жив или мертв враже объявляется

    Enemy(const sf::Texture* t, sf::Vector2f pos, float spd, int hp_);

    sf::Vector2f getPos() const;
    void setPos(const sf::Vector2f& p);
    void updateFollow(const std::vector<sf::Vector2f>& path, float dt);
    void draw(sf::RenderTarget& rt) const; //Остальное функции логики по типу отрисовки, местонахождения
};