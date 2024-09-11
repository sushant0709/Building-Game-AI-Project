#include <SFML/Graphics.hpp>
#include "./Crumbs.hpp"
#include <vector>

crumb::crumb(int id, sf::RenderWindow* window) : id(id), window(window)
{
    this->setRadius(4.0f);
    this->setFillColor(sf::Color(sf::Color(rand() % 255, rand() % 255, rand() % 255)));
    this->setPosition(-100, -100);
}

void crumb::draw()
{
    this->window->draw(*this);
}

void crumb::drop(float x, float y)
{
    this->setPosition(x, y);
}

void crumb::drop(sf::Vector2f position)
{
    this->setPosition(position);
}
