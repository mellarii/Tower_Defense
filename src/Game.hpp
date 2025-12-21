#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <queue>
#include <algorithm>
#include <cstdint>

#include "Grid.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "Resources.hpp"

class Game {
public:
    Game(sf::RenderWindow& window);

    void handleEvent(const sf::Event& ev);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    sf::RenderWindow& window;

    int startX = -1, startY = -1, endX = -1, endY = -1;
    PathGrid grid;
    PathGrid towerCooldown;

    Resources res; 

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Projectile> projectiles;

    bool leftWas  = false;
    bool rightWas = false;
    int  wave = 0;
    float spawnTimer = 0.f;
    int toSpawn = 0;

    /*-- Экономика --*/
    int gold = 30;                //Стартовое золото
    const int TOWER_COST = 15;  //Цена Wendy и Starry и Ramona
    const int GOLD_PER_KILL = 5;  //За убийство монстра

    std::vector<sf::Vector2f> currentPathCells;

    sf::Clock clock; //Нужно для правильной работы кадров, там что то типо что берется каждые две ближайшие секунды и по ним идут кадры шаг за шагом

    void initDefaultPath();
    bool buildPathBFS(std::vector<sf::Vector2f>& outPath);
    void spawnWaveLogic(float dt);
    void towersShoot();
    void updateProjectiles(float dt);
    void drawBackground();
    void drawTilesAndTowers();
    void drawPathLine();
};