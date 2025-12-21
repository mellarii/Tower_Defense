#include "Enemy.hpp"
#include <cmath>

Enemy::Enemy(const sf::Texture* t, sf::Vector2f pos, float spd, int hp_)
    : speed(spd), hp(hp_), pathIdx(0), alive(true)
{
    if (t) {
        spr.emplace(*t);
        auto ts = t->getSize();
        spr->setOrigin(sf::Vector2f(float(ts.x) / 2.f, float(ts.y) / 2.f));
        spr->setPosition(pos);
        spr->setScale(sf::Vector2f(float(SCALE), float(SCALE)));
    } else {
        fallback.setRadius(CELL / 8.f);
        fallback.setOrigin(sf::Vector2f(CELL / 8.f, CELL / 8.f));
        fallback.setPosition(pos);
        fallback.setFillColor(sf::Color::Green);
    }
} 

sf::Vector2f Enemy::getPos() const {
    return spr.has_value() ? spr->getPosition() : fallback.getPosition();
}

void Enemy::setPos(const sf::Vector2f& p) {
    if (spr.has_value())
        spr->setPosition(p);
    else
        fallback.setPosition(p);
}

void Enemy::updateFollow(const std::vector<sf::Vector2f>& path, float dt) {
    if (!alive) return;
    if (pathIdx >= path.size()) return;
    sf::Vector2f target = path[pathIdx];
    sf::Vector2f pos = getPos();
    sf::Vector2f d = target - pos;
    float dist = std::hypot(d.x, d.y);
    if (dist < 2.f) { pathIdx++; return; }
    d /= dist;
    setPos(pos + d * speed * dt);
    if (spr.has_value()) {
        float ang = std::atan2(d.y, d.x) * 180.f / 3.14159265f;
        spr->setRotation(sf::degrees(ang + 90.f));
    }
}
//Остальное функции логики по типу отрисовки, местонахождения
void Enemy::draw(sf::RenderTarget& rt) const { 
    if (!alive) return;
    if (spr.has_value())
        rt.draw(*spr);
    else
        rt.draw(fallback);
}