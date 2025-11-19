#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

int main()
{
    sf::RenderWindow window(sf::VideoMode({1920, 1080}), "My window");

    //class Ground
    sf::Texture deafultGroundTexture;
    if (!deafultGroundTexture.loadFromFile("./src/Images/Ground/Ground16x16.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite defGround(deafultGroundTexture);
    defGround.setScale(sf::Vector2f(10,10));

    sf::Texture leftGroundTexture;
    if (!leftGroundTexture.loadFromFile("./src/Images/Ground/GroundLeftGround.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite leftGround(leftGroundTexture);
    leftGround.setScale(sf::Vector2f(10,10));
    leftGround.setPosition(sf::Vector2f(480,0));

    sf::Texture rightGroundTexture;
    if (!rightGroundTexture.loadFromFile("./src/Images/Ground/GroundRightGround.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite rightGround(rightGroundTexture);
    rightGround.setScale(sf::Vector2f(10,10));
    rightGround.setPosition(sf::Vector2f(160,0));

    sf::Texture GroundGroundTexture;
    if (!GroundGroundTexture.loadFromFile("./src/Images/Ground/GroundGround.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite GroundGround(GroundGroundTexture);
    GroundGround.setScale(sf::Vector2f(10,10));
    //GroundGround.setPosition(sf::Vector2f(320,0));
    //sword.setOrigin(sf::Vector2f(8, 8));

    //end Ground

    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("./src/Images/Wendy.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite player(playerTexture);
    player.setScale(sf::Vector2f(10,10));
    player.setOrigin(sf::Vector2f(8, 8));

    sf::Texture enemyTexture;
    if (!enemyTexture.loadFromFile("./src/Images/LitleGoblin.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite enemy(enemyTexture);
    enemy.setScale(sf::Vector2f(10,10));
    enemy.setPosition(sf::Vector2f(960,540));
    enemy.setOrigin(sf::Vector2f(8, 8));

    sf::Texture wSwordTexture;
    if (!wSwordTexture.loadFromFile("./src/Images/Wendy'sSword.png")) {
        std::cerr << "Error\n";
        return -1;
    }
    sf::Sprite sword(wSwordTexture);
    sword.setScale(sf::Vector2f(10,10));
    sword.setOrigin(sf::Vector2f(8, 8));

    sf::Clock clock;
    float fSpeed = 100.0f;

    while (window.isOpen())
    {
        sf::Time timeSinceLastFrame = clock.restart();
        while (const std::optional event = window.pollEvent()) 
        {
            if (event->is<sf::Event::Closed>()) 
                window.close();
        }
        
        sf::Vector2i vMousePosition = sf::Mouse::getPosition(window);
        //std::cout << "x: "<< vMousePosition.x << "y: " << vMousePosition.y << std::endl; 
        GroundGround.setPosition((sf::Vector2f)vMousePosition);    
        sf::Vector2f vRequestedOlayerMovement(0.0f, 0.0f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        {
            vRequestedOlayerMovement += sf::Vector2f(0.0f, -1.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            vRequestedOlayerMovement += sf::Vector2f(1.0f, 0.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        {
            vRequestedOlayerMovement += sf::Vector2f(0.0f, 1.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            vRequestedOlayerMovement += sf::Vector2f(-1.0f, 0.0f);
        }
        player.move(vRequestedOlayerMovement*timeSinceLastFrame.asSeconds()*fSpeed);

        //window.clear(/*sf::Color::Red*/);
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 12; j++) {
                window.draw(defGround);
                defGround.setPosition(sf::Vector2f(160*j,160*i));
            }
        }
        window.draw(GroundGround);
        window.draw(leftGround);
        window.draw(rightGround);
        window.draw(player);
        window.draw(enemy);
        window.draw(sword);
        window.display();
    }

    return 0;
}