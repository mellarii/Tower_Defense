#include "Grid.hpp"

PathGrid::PathGrid(int w, int h) {
    cols = w / CELL;
    rows = h / CELL;
    cells.assign(cols * rows, 0);
}

int &PathGrid::at(int x, int y) {
    return cells[y * cols + x];
}

int PathGrid::atc(int x, int y) const { //Эта и предидущая это чтение и рисование сетки
    return cells[y * cols + x];
}

bool PathGrid::inBounds(int x, int y) const { //Проверка границ сетки
    return x >= 0 && y >= 0 && x < cols && y < rows;
}

sf::Vector2f PathGrid::cellCenter(int x, int y) const { //Координаты центра сетки
    return { x * CELL + CELL / 2.f, y * CELL + CELL / 2.f };
}
