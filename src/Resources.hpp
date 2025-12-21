#pragma once

#include <SFML/Graphics.hpp>

//Загрузка текстур (если есть что грузить)
struct Resources {
    sf::Texture texGround16;
    sf::Texture texGroundGround;
    sf::Texture texWendy;
    sf::Texture texGoblin;
    sf::Texture texSkeleton;
    sf::Texture texStarry; 
    sf::Texture texRamona;

    bool haveG16 = false;
    bool haveGG = false;
    bool haveW = false;
    bool haveGobl = false;
    bool haveSkel = false;
    bool haveStarry = false;
    bool haveRamona = false;

    bool load();
};