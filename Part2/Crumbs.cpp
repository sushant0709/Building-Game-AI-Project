#include <SFML/Graphics.hpp>
#include "./Crumbs.hpp"
#include <vector>

crumb::crumb(int id, sf::RenderWindow* window, bool isMonster) : id(id), window(window), isMonster(isMonster)
{
    this->setRadius(4.0f);
    if(isMonster)
        this->setFillColor(sf::Color(sf::Color(255,0,0)));
    else
        this->setFillColor(sf::Color(sf::Color(0,255,0)));
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
