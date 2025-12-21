#include "Projectile.hpp"
#include <cmath>

Projectile::Projectile(sf::Vector2f startPos, Enemy* tgt, float spd, int dmg)
    : speed(spd), damage(dmg), alive(true), target(tgt)
{
    shape.setRadius(CELL / 10.f);
    shape.setOrigin(sf::Vector2f(shape.getRadius(), shape.getRadius()));
    shape.setFillColor(sf::Color::Yellow);
    shape.setPosition(startPos);

    if (target) {
        sf::Vector2f dir = target->getPos() - startPos;
        float len = std::hypot(dir.x, dir.y);
        if (len > 0.0001f) {
            velocity = (dir / len) * speed;
        } else {
            velocity = sf::Vector2f(0.f, 0.f);
        }
    } else {
        velocity = sf::Vector2f(0.f, 0.f);
    }
}

void Projectile::update(float dt) {
    if (!alive) return;
    if (!target || !target->alive) {
        alive = false;
        return;
    }

    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f toTarget = target->getPos() - pos;
    float dist = std::hypot(toTarget.x, toTarget.y);
    if (dist < 1.f) dist = 1.f;
    sf::Vector2f dir = toTarget / dist;
    velocity = dir * speed;

    shape.move(velocity * dt);

    pos = shape.getPosition();
    toTarget = target->getPos() - pos;
    dist = std::hypot(toTarget.x, toTarget.y);
    if (dist < CELL / 4.f) {
        target->hp -= damage;
        if (target->hp <= 0) {
            target->alive = false;
        }
        alive = false;
    }
}

void Projectile::draw(sf::RenderTarget& rt) const {//Далее тут системные функции для её двиджения, вычитания хп у таргета, отрисовкеи и пропадения после того как она попала во врага
    if (!alive) return;
    rt.draw(shape);
}