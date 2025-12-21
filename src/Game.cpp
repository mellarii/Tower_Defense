#include <SFML/Graphics.hpp>  
#include "Game.hpp"

Game::Game(sf::RenderWindow& win)
    : window(win)
    , grid(win.getSize().x, win.getSize().y)
    , towerCooldown(win.getSize().x, win.getSize().y)
{
    res.load(); //Загрузка текстур (если есть что грузить)
    initDefaultPath();
}

/* --- Заранее сгенерированная дорожка --- */
void Game::initDefaultPath() {
    std::vector<std::pair<int,int>> defaultPathCells = {
        {0, 1}, {1, 1}, {2, 1}, {3, 1},
        {3, 2}, {3, 3},
        {4, 3}, {5, 3}, {6, 3}, {7, 3}
    };
    int defaultStartIndex = 0;
    int defaultEndIndex   = int(defaultPathCells.size()) - 1;

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
}

bool Game::buildPathBFS(std::vector<sf::Vector2f>& outPath) {
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
}

void Game::handleEvent(const sf::Event& ev) {
    (void)ev; // пока тут ничего не делаем
}


void Game::update(float dt) {
    float dummy = dt; // чтобы комментарии ниже были к месту

    float dtReal = clock.restart().asSeconds();//Вычисление времени
    (void)dummy; // dtReal фактически и есть dt, но оставляем логику clock, как в твоём main

    // Обработка ввода мыши/клавиш
    bool leftNow  = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool rightNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    bool shift    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    bool ctrl     = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
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

    leftWas  = leftNow;
    rightWas = rightNow;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
        currentPathCells.clear();
        if (buildPathBFS(currentPathCells)) {
            wave++;
            toSpawn = 5 + wave * 2;
            spawnTimer = 0.f;
        }
    } //5*

    spawnWaveLogic(dtReal);

    for (auto &e : enemies) e->updateFollow(currentPathCells, dtReal);

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

    towersShoot();

    updateProjectiles(dtReal);
}

void Game::spawnWaveLogic(float dt) {
    if (toSpawn > 0 && !currentPathCells.empty()) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.f) { //Тут время время не вышло, враги спавнятся по логике ниже 
            spawnTimer = 0.8f;

            sf::Vector2f startPos = currentPathCells.front();//Создаются в currentPathCells т.е. в начале пути

            bool  spawnSkeleton = (wave % 2 == 1) && (toSpawn % 2 == 0);
            const float goblinSpeed = 60.f + wave * 6.f;
            const int goblinHP = 100 + wave * 2;

            const float skeletonSpeed = 100.f + wave * 8.f;
            const int skeletonHP = 1;
            const sf::Texture* enemyTex = nullptr;
            float enemySpeed = goblinSpeed;
            int enemyHP  = goblinHP;

            if (spawnSkeleton && res.haveSkel) {
                enemyTex   = &res.texSkeleton;
                enemySpeed = skeletonSpeed;
                enemyHP  = skeletonHP;
            } else {
                enemyTex = res.haveGobl ? &res.texGoblin : nullptr;
                enemySpeed = goblinSpeed;
                enemyHP = goblinHP;
            }

            enemies.emplace_back(std::make_unique<Enemy>(enemyTex, startPos, enemySpeed, enemyHP));
            toSpawn--;
        } //Спавн врагов либо скелетончика либо гоблина. У гоблина больше хп меньше мувспид, у скелета наоборот
    }
}

void Game::towersShoot() {
    // --- Стрельба башен ---
    const float towerFireInterval = 0.7f;//Венди
    const int towerDamage = 25;
    const float towerRange = CELL * 6.f;
    const float projectileSpeed = 300.f;
    //Старри: намного медленнее, но больнее
    const float starryFireInterval = towerFireInterval * 4.f;
    const int starryDamage = int(towerDamage * 10.0f + wave * 10.0f);
    
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
            float  bestDist2 = towerRange * towerRange;
            
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
            int   damage;
            float fireIntervalSec;
            if (v == 4) {//Венди
                damage  = towerDamage;
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
}

void Game::updateProjectiles(float dt) {
    for (auto& p : projectiles) {
        p.update(dt);
    }
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p){ return !p.alive; }),
        projectiles.end()
    ); //Удаление пулек
}

void Game::drawBackground() {
    for (int y = 0; y < grid.rows; ++y) {
        for (int x = 0; x < grid.cols; ++x) {
            sf::Vector2f pos(float(x * CELL), float(y * CELL));
            if (res.haveG16) {
                sf::Sprite s(*std::addressof(res.texGround16));
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
}

void Game::drawTilesAndTowers() {
    for (int y = 0; y < grid.rows; ++y) {
        for (int x = 0; x < grid.cols; ++x) {
            int v = grid.atc(x, y);
            sf::Vector2f pos(float(x * CELL), float(y * CELL));
            if (v == 1 || v == 2 || v == 3) {
                if (res.haveGG) {
                    sf::Sprite s(*std::addressof(res.texGroundGround));
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
                if (res.haveW) {
                    sf::Sprite s(*std::addressof(res.texWendy));
                    auto ts = res.texWendy.getSize();
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
                if (res.haveStarry) {
                    sf::Sprite s(res.texStarry);
                    auto ts = res.texStarry.getSize();
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
}

void Game::drawPathLine() {
    if (!currentPathCells.empty()) {
        sf::VertexArray va(sf::PrimitiveType::LineStrip, static_cast<unsigned int>(currentPathCells.size()));
        for (size_t i = 0; i < currentPathCells.size(); ++i) va[i].position = currentPathCells[i];
        window.draw(va);
    } //Рисуем линию с маршрутов для врагов
}

void Game::render(sf::RenderWindow& win) {
    window.clear(sf::Color(30, 30, 30));

    drawBackground();
    drawTilesAndTowers();
    drawPathLine();

    for (auto &e : enemies) e->draw(window);
    for (const auto& p : projectiles) p.draw(window); //Ну и в конце враги и пули

    window.display();
}