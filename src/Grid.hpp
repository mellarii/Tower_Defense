#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

constexpr int TILE_PIX = 16;
constexpr int SCALE    = 10;
constexpr int CELL     = TILE_PIX * SCALE;

struct PathGrid { //Рисование сетки (6 на 12) через одномерный массив. От числа зависит чо в сетке происходит 0 - травка, 1 - дорожка, 2/3 - старт/финишь, 4/5 - Венди/Старри (башенки ну)
    int cols, rows;
    std::vector<int> cells;

    PathGrid(int w, int h);

    int &at(int x, int y);
    int  atc(int x, int y) const; //Эта и предидущая это чтение и рисование сетки
    bool inBounds(int x, int y) const; //Проверка границ сетки
    sf::Vector2f cellCenter(int x, int y) const; //Координаты центра сетки
};
