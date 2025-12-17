#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <memory>
#include <optional>
#include <sstream>
#include <algorithm>
#include <cstdint>

constexpr int TILE_PIX = 16;
constexpr int SCALE = 10;
constexpr int CELL = TILE_PIX * SCALE;

struct PathGrid {
    int cols, rows;
    std::vector<int> cells;
    PathGrid(int w, int h) {
        cols = w / CELL;
        rows = h / CELL;
        cells.assign(cols * rows, 0);
    }
    int &at(int x, int y) { return cells[y * cols + x]; }
    int atc(int x, int y) const { return cells[y * cols + x]; }
    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < cols && y < rows; }
    sf::Vector2f cellCenter(int x, int y) const { return { x * CELL + CELL / 2.f, y * CELL + CELL / 2.f }; }
};

struct Enemy {
    std::optional<sf::Sprite> spr;
    sf::CircleShape fallback;
    float speed;
    int hp;
    size_t pathIdx;
    bool alive;
    Enemy(const sf::Texture* t, sf::Vector2f pos, float spd, int hp_) : speed(spd), hp(hp_), pathIdx(0), alive(true) {
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
    sf::Vector2f getPos() const { return spr.has_value() ? spr->getPosition() : fallback.getPosition(); }
    void setPos(const sf::Vector2f& p) { if (spr.has_value()) spr->setPosition(p); else fallback.setPosition(p); }
    void updateFollow(const std::vector<sf::Vector2f>& path, float dt) {
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
    void draw(sf::RenderTarget& rt) const { if (!alive) return; if (spr.has_value()) rt.draw(*spr); else rt.draw(fallback); }
};

struct Projectile {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;
    int damage;
    bool alive;
    Enemy* target;

    Projectile(sf::Vector2f startPos, Enemy* tgt, float spd, int dmg)
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

    void update(float dt) {
        if (!alive) return;
        if (!target || !target->alive) {
            alive = false;
            return;
        }

        // Перенаправляемся на цель каждый кадр (простейший homing)
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f toTarget = target->getPos() - pos;
        float dist = std::hypot(toTarget.x, toTarget.y);
        if (dist < 1.f) dist = 1.f;
        sf::Vector2f dir = toTarget / dist;
        velocity = dir * speed;

        shape.move(velocity * dt);

        // Проверка попадания (по расстоянию)
        pos = shape.getPosition();
        toTarget = target->getPos() - pos;
        dist = std::hypot(toTarget.x, toTarget.y);
        if (dist < CELL / 4.f) { // радиус попадания
            target->hp -= damage;
            if (target->hp <= 0) {
                target->alive = false;
            }
            alive = false;
        }
    }

    void draw(sf::RenderTarget& rt) const {
        if (!alive) return;
        rt.draw(shape);
    }
};


int main() {
    const unsigned int W = 1920;
    const unsigned int H = 1200;
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(W, H)), "TD - grid path (Ground16x16/GroundGround)");
    window.setFramerateLimit(60);

    PathGrid grid(W, H);

    sf::Texture texGround16;
    sf::Texture texGroundGround;
    sf::Texture texWendy;
    sf::Texture texGoblin;
    bool haveG16 = false, haveGG = false, haveW = false, haveGobl = false;
    if (texGround16.loadFromFile("../../src/Images/Ground/Ground16x16.png")) haveG16 = true;
    if (texGroundGround.loadFromFile("../../src/Images/Ground/GroundGround.png")) haveGG = true;
    if (texWendy.loadFromFile("../../src/Images/Wendy.png")) haveW = true;
    if (texGoblin.loadFromFile("../../src/Images/LitleGoblin.png")) haveGobl = true;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;

    sf::Font font;
    bool gotFont = false;
    if (font.openFromFile("resources/arial.ttf") || font.openFromFile("./arial.ttf")) gotFont = true;
    // Create Texts using lvalue font (even if not loaded, font is an lvalue)
    sf::Text ui(font, "", 16);
    ui.setFillColor(sf::Color::White);
    sf::Text hint(font, "ЛКМ: ставить тропу GroundGround (первый - START, второй - END). R: старт волны. Shift+ЛКМ: поставить Wendy. ПКМ: удалить.", 14);
    hint.setFillColor(sf::Color::White);
    hint.setPosition(sf::Vector2f(6.f, float(H - 22)));

    bool leftWas = false, rightWas = false;
    int startX = -1, startY = -1, endX = -1, endY = -1;
    int wave = 0;
    float spawnTimer = 0.f;
    int toSpawn = 0;
    float towerShootTimer = 0.f;
    std::vector<sf::Vector2f> currentPathCells;

    auto buildPathBFS = [&](std::vector<sf::Vector2f>& outPath)->bool {
        if (startX < 0 || endX < 0) return false;
        int C = grid.cols, R = grid.rows;
        std::vector<int> prev(C * R, -1);
        std::queue<std::pair<int,int>> q;
        q.push({startX,startY});
        prev[startY * C + startX] = -2;
        const int dx[4] = {1,-1,0,0}, dy[4] = {0,0,1,-1};
        bool found = false;
        while (!q.empty()) {
            auto [x,y] = q.front(); q.pop();
            if (x == endX && y == endY) { found = true; break; }
            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i], ny = y + dy[i];
                if (!grid.inBounds(nx, ny)) continue;
                int v = grid.atc(nx, ny);
                if ((v == 1 || v == 2 || v == 3) && prev[ny * C + nx] == -1) {
                    prev[ny * C + nx] = y * C + x;
                    q.push({nx, ny});
                }
            }
        }
        if (!found) return false;
        std::vector<std::pair<int,int>> cells;
        int cur = endY * C + endX;
        while (cur != -2) {
            int cx = cur % C, cy = cur / C;
            cells.push_back({cx, cy});
            cur = prev[cur];
        }
        std::reverse(cells.begin(), cells.end());
        outPath.clear();
        for (auto &p : cells) outPath.push_back(grid.cellCenter(p.first, p.second));
        return true;
    };

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        towerShootTimer += dt;
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        bool leftNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool rightNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

        if (leftNow && !leftWas) {
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            int gx = mp.x / CELL;
            int gy = mp.y / CELL;
            if (grid.inBounds(gx, gy)) {
                if (shift) {
                    if (grid.atc(gx, gy) == 0) {
                        grid.at(gx, gy) = 4;
                    }
                } else {
                    if (grid.atc(gx, gy) == 0) {
                        grid.at(gx, gy) = 1;
                        if (startX < 0) { startX = gx; startY = gy; grid.at(gx, gy) = 2; }
                        else if (endX < 0) { endX = gx; endY = gy; grid.at(gx, gy) = 3; }
                    }
                }
            }
        }

        if (rightNow && !rightWas) {
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            int gx = mp.x / CELL;
            int gy = mp.y / CELL;
            if (grid.inBounds(gx, gy)) {
                int v = grid.atc(gx, gy);
                if (v == 2) { startX = startY = -1; }
                if (v == 3) { endX = endY = -1; }
                grid.at(gx, gy) = 0;
            }
        }

        leftWas = leftNow;
        rightWas = rightNow;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            currentPathCells.clear();
            if (buildPathBFS(currentPathCells)) {
                wave++;
                toSpawn = 5 + wave * 2;
                spawnTimer = 0.f;
            }
        }

        if (toSpawn > 0 && !currentPathCells.empty()) {
            spawnTimer -= dt;
            if (spawnTimer <= 0.f) {
                spawnTimer = 0.8f;
                sf::Vector2f startPos = currentPathCells.front();
                enemies.emplace_back(std::make_unique<Enemy>(haveGobl ? &texGoblin : nullptr, startPos, 60.f + wave * 6.f, 100));
                toSpawn--;
            }
        }

        for (auto &e : enemies) e->updateFollow(currentPathCells, dt);

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [&](const std::unique_ptr<Enemy>& e) {
            return (!e->alive) || (e->pathIdx >= currentPathCells.size());
        }), enemies.end());

        // --- Стрельба башен ---
const float towerFireInterval = 0.7f;      // раз в 0.7 секунды
const float towerRange = CELL * 6.f;       // радиус действия башни
const int   towerDamage = 25;
const float projectileSpeed = 300.f;

if (towerShootTimer >= towerFireInterval && !enemies.empty()) {
    // Проходим по всему гриду, ищем клетки с башнями (v == 4)
    for (int y = 0; y < grid.rows; ++y) {
        for (int x = 0; x < grid.cols; ++x) {
            int v = grid.atc(x, y);
            if (v == 4) {
                sf::Vector2f towerPos = grid.cellCenter(x, y);

                // Находим ближайшего врага в радиусе
                Enemy* bestEnemy = nullptr;
                float bestDist2 = towerRange * towerRange;

                for (auto& ePtr : enemies) {
                    if (!ePtr->alive) continue;
                    sf::Vector2f ep = ePtr->getPos();
                    sf::Vector2f d = ep - towerPos;
                    float d2 = d.x * d.x + d.y * d.y;
                    if (d2 < bestDist2) {
                        bestDist2 = d2;
                        bestEnemy = ePtr.get();
                    }
                }

                if (bestEnemy) {
                    // Создаём снаряд
                    projectiles.emplace_back(towerPos, bestEnemy, projectileSpeed, towerDamage);
                }
            }
        }
    }

    towerShootTimer = 0.f;
}

// --- Обновление снарядов ---
for (auto& p : projectiles) {
    p.update(dt);
}

// Удаляем «мёртвые» снаряды
projectiles.erase(
    std::remove_if(projectiles.begin(), projectiles.end(),
                   [](const Projectile& p){ return !p.alive; }),
    projectiles.end()
);


        window.clear(sf::Color(30, 30, 30));

        for (int y = 0; y < grid.rows; ++y) {
            for (int x = 0; x < grid.cols; ++x) {
                sf::Vector2f pos(float(x * CELL), float(y * CELL));
                if (haveG16) {
                    sf::Sprite s(*std::addressof(texGround16));
                    s.setOrigin(sf::Vector2f(0.f, 0.f));
                    s.setPosition(pos);
                    s.setScale(sf::Vector2f(float(SCALE), float(SCALE)));
                    window.draw(s);
                } else {
                    sf::RectangleShape r{ sf::Vector2f(float(CELL), float(CELL)) };
                    r.setPosition(pos);
                    r.setFillColor(sf::Color(80, 120, 60));
                    window.draw(r);
                }
            }
        }

        for (int y = 0; y < grid.rows; ++y) {
            for (int x = 0; x < grid.cols; ++x) {
                int v = grid.atc(x, y);
                sf::Vector2f pos(float(x * CELL), float(y * CELL));
                if (v == 1 || v == 2 || v == 3) {
                    if (haveGG) {
                        sf::Sprite s(*std::addressof(texGroundGround));
                        s.setOrigin(sf::Vector2f(0.f, 0.f));
                        s.setPosition(pos);
                        s.setScale(sf::Vector2f(float(SCALE), float(SCALE)));
                        window.draw(s);
                    } else {
                        sf::RectangleShape r{ sf::Vector2f(float(CELL), float(CELL)) };
                        r.setPosition(pos);
                        r.setFillColor(sf::Color(140, 100, 60));
                        window.draw(r);
                    }
                    if (v == 2) {
                        sf::RectangleShape mark{ sf::Vector2f(float(CELL), float(CELL)) };
                        mark.setPosition(sf::Vector2f(float(x * CELL), float(y * CELL)));
                        mark.setFillColor(sf::Color(0, 255, 0, 80));
                        window.draw(mark);
                    } else if (v == 3) {
                        sf::RectangleShape mark{ sf::Vector2f(float(CELL), float(CELL)) };
                        mark.setPosition(sf::Vector2f(float(x * CELL), float(y * CELL)));
                        mark.setFillColor(sf::Color(255, 0, 0, 100));
                        window.draw(mark);
                    }
                } else if (v == 4) {
                    sf::Vector2f center = grid.cellCenter(x, y);
                    if (haveW) {
                        sf::Sprite s(*std::addressof(texWendy));
                        auto ts = texWendy.getSize();
                        s.setOrigin(sf::Vector2f(float(ts.x) / 2.f, float(ts.y) / 2.f));
                        s.setPosition(center);
                        s.setScale(sf::Vector2f(float(SCALE), float(SCALE)));
                        window.draw(s);
                    } else {
                        sf::CircleShape c(CELL / 6.f);
                        c.setOrigin(sf::Vector2f(CELL / 6.f, CELL / 6.f));
                        c.setPosition(center);
                        c.setFillColor(sf::Color(150, 150, 255));
                        window.draw(c);
                    }
                }
            }
        }

        if (!currentPathCells.empty()) {
            sf::VertexArray va(sf::PrimitiveType::LineStrip, static_cast<unsigned int>(currentPathCells.size()));
            for (size_t i = 0; i < currentPathCells.size(); ++i) va[i].position = currentPathCells[i];
            window.draw(va);
        }

        for (auto &e : enemies) e->draw(window);

        if (gotFont) {
            std::ostringstream ss;
            ss << "Wave: " << wave << "  To spawn: " << toSpawn << "  Start: " << (startX >= 0 ? std::to_string(startX) + "," + std::to_string(startY) : "none") << "  End: " << (endX >= 0 ? std::to_string(endX) + "," + std::to_string(endY) : "none");
            ui.setString(ss.str());
            ui.setPosition(sf::Vector2f(6.f, 6.f));
            window.draw(ui);
            window.draw(hint);
        }

        for (const auto& p : projectiles) {
            p.draw(window);
        }


        window.display();
    }

    return 0;
}