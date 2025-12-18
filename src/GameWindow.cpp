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

struct PathGrid { //Рисование сетки (6 на 12) через одномерный массив. От числа зависит чо в сетке происходит 0 - травка, 1 - дорожка, 2/3 - старт/финишь, 4/5 - Венди/Старри (башенки ну)
    int cols, rows;
    std::vector<int> cells;
    PathGrid(int w, int h) {
        cols = w / CELL;
        rows = h / CELL;
        cells.assign(cols * rows, 0);
    }
    int &at(int x, int y) { return cells[y * cols + x]; } 
    int atc(int x, int y) const { return cells[y * cols + x]; } //Эта и предидущая это чтение и рисование сетки
    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < cols && y < rows; } //Проверка границ сетки
    sf::Vector2f cellCenter(int x, int y) const { return { x * CELL + CELL / 2.f, y * CELL + CELL / 2.f }; } //Координаты центра сетки
};

struct Enemy { //Структура для врагов со всем всем всем, размером, хитбоксом, замена текстуры (если нет картинки), отслеживанием позиции и остальным
    std::optional<sf::Sprite> spr;
    sf::CircleShape fallback;
    float speed;
    int hp;
    size_t pathIdx;
    bool alive; //Тут  конкретно скорость хп, местонахождение, жив или мертв враже объявляется
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
    void draw(sf::RenderTarget& rt) const { if (!alive) return; if (spr.has_value()) rt.draw(*spr); else rt.draw(fallback); } //Остальное функции логики по типу отрисовки, местонахождения
};

struct Projectile { //Структура для пулек башен, они у всех одинаковые
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed;
    int damage;
    bool alive;
    Enemy* target;//У них есть скорость, урон, есть они или нет и цель, за которой они будут лететь (из за этого есть некоторые баги, но я не знаю пока что как их исправить, они не критичные)

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

    void draw(sf::RenderTarget& rt) const {
        if (!alive) return;
        rt.draw(shape);
    }//Далее тут системные функции для её двиджения, вычитания хп у таргета, отрисовкеи и пропадения после того как она попала во врага
};

int main() { //Мой мейн
    const unsigned int W = 1920;
    const unsigned int H = 1200; //Разрешение экрана, лучше оставить такое, иначе при изначальном рендере меньшего окна при открытии в полноэкранку игра имеет ненулевые шансы сломаться
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(W, H)), "TD - grid path (Ground16x16/GroundGround)");
    window.setFramerateLimit(60); 

    int startX = -1, startY = -1, endX = -1, endY = -1;
    PathGrid grid(W, H);
    /* --- Заранее сгенерированная дорожка --- */
    std::vector<std::pair<int,int>> defaultPathCells = {
    {0, 1}, {1, 1}, {2, 1}, {3, 1},
    {3, 2}, {3, 3},
    {4, 3}, {5, 3}, {6, 3}, {7, 3}
};
    int defaultStartIndex = 0;
    int defaultEndIndex = int(defaultPathCells.size()) - 1;

    //Очистим сетку (на всякий случай)
    for (int y = 0; y < grid.rows; ++y) {
        for (int x = 0; x < grid.cols; ++x) {
            grid.at(x, y) = 0;
        }
    }
    
    //Проставляем путь
    for (size_t i = 0; i < defaultPathCells.size(); ++i) {
        int x = defaultPathCells[i].first;
        int y = defaultPathCells[i].second;
        if (!grid.inBounds(x, y)) continue;
        
        if (int(i) == defaultStartIndex) {
            grid.at(x, y) = 2; // START
            startX = x;
            startY = y;
        } else if (int(i) == defaultEndIndex) {
            grid.at(x, y) = 3; // END
            endX = x;
            endY = y;
        } else {
            grid.at(x, y) = 1; //Обычная дорожка GroundGround
        }
    }


    PathGrid towerCooldown(W, H);

    sf::Texture texGround16;
    sf::Texture texGroundGround;
    sf::Texture texWendy;
    sf::Texture texGoblin;
    sf::Texture texSkeleton;
    sf::Texture texStarry; 

    bool haveG16 = false, haveGG = false, haveW = false, haveGobl = false, haveSkel = false, haveStarry = false;
    if (texGround16.loadFromFile("../../src/Images/Ground/Ground16x16.png")) haveG16 = true;
    if (texGroundGround.loadFromFile("../../src/Images/Ground/GroundGround.png")) haveGG = true;
    if (texWendy.loadFromFile("../../src/Images/Wendy.png")) haveW = true;
    if (texGoblin.loadFromFile("../../src/Images/LitleGoblin.png")) haveGobl = true;
    if (texSkeleton.loadFromFile("../../src/Images/Skeleton.png")) haveSkel = true;
    if (texStarry.loadFromFile("../../src/Images/Starry.png")) haveStarry = true;
    //Загрузка текстур (если есть что грузить)

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;

    bool leftWas = false, rightWas = false;
    int wave = 0;
    float spawnTimer = 0.f;
    int toSpawn = 0;

    /*-- Экономика --*/
    int gold = 30;               //Стартовое золото
    const int TOWER_COST = 15;   //Цена Wendy и Starry
    const int GOLD_PER_KILL = 5; //За убийство монстра
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

    sf::Clock clock; //Нужно для правильной работы кадров, там что то типо что берется каждые две ближайшие секунды и по ним идут кадры шаг за шагом

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();//Вычисление времени
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }//Обработка закрытия окошка

        bool leftNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool rightNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
        bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
        //Обработка нажатия на клавиши
        /* Перечислю что впринципе можно делать, но сами функции будут ниже
        1. ЛКМ - поставить дорожку (с логикой начало - конец -> потом обычные дорожки)
        2. ПКМ - удалить обект на сетке под курсором
        3. ШИФТ + ЛКМ - поставить башню венди *
        4. КНТРЛ + ЛКМ - поставить башню Старри *
        5. К (eng R) - запустить волну
        */

        if (leftNow && !leftWas) {
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            int gx = mp.x / CELL;
            int gy = mp.y / CELL;
            if (grid.inBounds(gx, gy)) {
                int cell = grid.atc(gx, gy);
                if (shift) {
                    // Wendy
                    if (cell == 0 && gold >= TOWER_COST) {
                        gold -= TOWER_COST;
                        grid.at(gx, gy) = 4; 
                    } //3*
                } else if (ctrl) {
                    // Starry
                    if (cell == 0 && gold >= TOWER_COST) {
                        gold -= TOWER_COST;
                        grid.at(gx, gy) = 5;
                    } //4*
                } else {
                    if (cell == 0) {
                        grid.at(gx, gy) = 1;
                        if (startX < 0) { startX = gx; startY = gy; grid.at(gx, gy) = 2; }
                        else if (endX < 0) { endX = gx; endY = gy; grid.at(gx, gy) = 3; }
                    } 
                }
            }
        }
        //Если НЕ шифт и кнтрл то ставить не башни
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
        } //5*

        if (toSpawn > 0 && !currentPathCells.empty()) {
            spawnTimer -= dt;
            if (spawnTimer <= 0.f) { //Тут время время не вышло, враги спавнятся по логике ниже 
                spawnTimer = 0.8f;

                sf::Vector2f startPos = currentPathCells.front();//Создаются в currentPathCells т.е. в начале пути

                bool spawnSkeleton = (wave % 2 == 1) && (toSpawn % 2 == 0);
                const float goblinSpeed = 60.f + wave * 6.f;
                const int goblinHP = 100;

                const float skeletonSpeed = 100.f + wave * 8.f;
                const int skeletonHP = 50;
                const sf::Texture* enemyTex = nullptr;
                float enemySpeed = goblinSpeed;
                int enemyHP = goblinHP;

                if (spawnSkeleton && haveSkel) {
                    enemyTex = &texSkeleton;
                    enemySpeed = skeletonSpeed;
                    enemyHP = skeletonHP;
                } else {
                    enemyTex = haveGobl ? &texGoblin : nullptr;
                    enemySpeed = goblinSpeed;
                    enemyHP = goblinHP;
                }

                enemies.emplace_back(std::make_unique<Enemy>(enemyTex, startPos, enemySpeed, enemyHP));
                toSpawn--;
            } //Спавн врагов либо скелетончика либо гоблина. У гоблина больше хп меньше мувспид, у скелета наоборот
        }

        for (auto &e : enemies) e->updateFollow(currentPathCells, dt);

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [&](const std::unique_ptr<Enemy>& e) {
        if (!e->alive) {
            gold += GOLD_PER_KILL;
            return true;
        }
        if (e->pathIdx >= currentPathCells.size()) {
            return true;
        }
        return false;
    }), enemies.end()); //Смерть врага и выдача золота если враг умер от вышки


        // --- Стрельба башен ---
        const float towerFireInterval = 0.7f;//Венди
        const int towerDamage = 25;
        const float towerRange = CELL * 6.f;
        const float projectileSpeed = 300.f;
        //Старри: намного медленнее, но больнее
        const float starryFireInterval  = towerFireInterval * 4.f;
        const int starryDamage = int(towerDamage * 3.5f);
        
        for (int y = 0; y < grid.rows; ++y) {
            for (int x = 0; x < grid.cols; ++x) {
                int v = grid.atc(x, y);
                if (v != 4 && v != 5) continue; //Не башня
                int& cd = towerCooldown.at(x, y);
                if (cd > 0) {
                    cd -= 1;
                    continue; //Ещё перезаряжается
                }
                sf::Vector2f towerPos = grid.cellCenter(x, y);
                //Ищем ближайшего врага
                Enemy* bestEnemy = nullptr;
                float bestDist2 = towerRange * towerRange;
                
                for (auto& ePtr : enemies) {
                    if (!ePtr->alive) continue;
                    sf::Vector2f ep = ePtr->getPos();
                    sf::Vector2f d  = ep - towerPos;
                    float d2 = d.x * d.x + d.y * d.y;
                    if (d2 < bestDist2) {
                        bestDist2 = d2;
                        bestEnemy = ePtr.get();
                    }
                }
                if (!bestEnemy) continue;
                //Параметры зависят от типа башни
                int damage;
                float fireIntervalSec;
                if (v == 4) {//Венди
                    damage = towerDamage;
                    fireIntervalSec = towerFireInterval;
                } else {//Старри
                    damage = starryDamage;
                    fireIntervalSec = starryFireInterval;
                }
                //Стреляем пиу пиу
                projectiles.emplace_back(towerPos, bestEnemy, projectileSpeed, damage);
                //Выставляем перезарядку в "тиках": например, 60 тиков = 1 сек при 60 FPS
                cd = int(fireIntervalSec * 60.f);
            }
        }


        for (auto& p : projectiles) {
            p.update(dt);
        }
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p){ return !p.alive; }),
            projectiles.end()
        ); //Удаление пулек

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
                }//Рисуем по клеткам травку, но только если рисунок травки есть, иначе закрашиваем
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
                    } //в Трех выше рисуем дорожку, и на первую и вторую (тобеш на начало и стар соответственно) поверх рисуем полупрозрачные квадраты, помечая начало и старт
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
                    } //Рисуем башни и из текстуры, блок елс иф в==4 это венди, а ниже с елс иф в==5 это старри
                } else if (v == 5) {
                    sf::Vector2f center = grid.cellCenter(x, y);
                    if (haveStarry) {
                        sf::Sprite s(texStarry);
                        auto ts = texStarry.getSize();
                        s.setOrigin(sf::Vector2f(float(ts.x) / 2.f, float(ts.y) / 2.f));
                        sf::Vector2f offset(0.f, CELL * 0.3f);
                        s.setPosition(center + offset);
                        s.setScale(sf::Vector2f(float(SCALE)*0.68f, float(SCALE)*0.68f));
                        window.draw(s);
                    } else {
                        sf::CircleShape c(CELL / 6.f);
                        c.setOrigin(sf::Vector2f(CELL / 6.f, CELL / 6.f));
                        c.setPosition(center);
                        c.setFillColor(sf::Color(255, 215, 0));
                        window.draw(c);
                    }
                }
            }
        }

        if (!currentPathCells.empty()) {
            sf::VertexArray va(sf::PrimitiveType::LineStrip, static_cast<unsigned int>(currentPathCells.size()));
            for (size_t i = 0; i < currentPathCells.size(); ++i) va[i].position = currentPathCells[i];
            window.draw(va);
        } //Рисуем линию с маршрутов для врагов

        for (auto &e : enemies) e->draw(window);
        for (const auto& p : projectiles) p.draw(window); //Ну и в конце враги и пули

        window.display();
    }

    return 0; //Кон
} 