#ifndef CRUMBS_HPP
#define CRUMBS_HPP

#include <SFML/Graphics.hpp>

class crumb : public sf::CircleShape
{
public:
    crumb(int id, sf::RenderWindow* window);

    void draw();

    void drop(float x, float y);

    void drop(sf::Vector2f position);

private:
    int id;
    sf::RenderWindow* window;
};

#endif