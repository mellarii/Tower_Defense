#include "Resources.hpp"

bool Resources::load() {
    haveG16 = texGround16.loadFromFile("../../src/Images/Ground/Ground16x16.png");
    haveGG = texGroundGround.loadFromFile("../../src/Images/Ground/GroundGround.png");
    haveW = texWendy.loadFromFile("../../src/Images/Wendy.png");
    haveGobl = texGoblin.loadFromFile("../../src/Images/LitleGoblin.png");
    haveSkel = texSkeleton.loadFromFile("../../src/Images/Skeleton.png");
    haveStarry = texStarry.loadFromFile("../../src/Images/Starry.png");
    haveRamona = texRamona.loadFromFile("../../src/Images/Ramona.png");
    //Загрузка текстур (если есть что грузить)

    return haveG16 || haveGG || haveW || haveGobl || haveSkel || haveStarry;
}