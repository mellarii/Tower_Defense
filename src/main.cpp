#include <SFML/Graphics.hpp>
#include "Game.hpp"

int main() { //Мой мейн
    const unsigned int W = 1920;
    const unsigned int H = 1200; //Разрешение экрана, лучше оставить такое, иначе при изначальном рендере меньшего окна при открытии в полноэкранку игра имеет ненулевые шансы сломаться

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(W, H)),
                            "TD - grid path (Ground16x16/GroundGround)");
    window.setFramerateLimit(60);

    Game game(window);

    sf::Clock clock; //Нужно для правильной работы кадров, там что то типо что берется каждые две ближайшие секунды и по ним идут кадры шаг за шагом

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            // ev: std::optional<sf::Event>
            if (ev->is<sf::Event::Closed>()) {
                window.close(); //Обработка закрытия окошка
            } else {
                game.handleEvent(*ev);
            }
        }

        float dt = clock.restart().asSeconds(); //Вычисление времени
        game.update(dt);

        game.render(window);
    }

    return 0; //Кон
}