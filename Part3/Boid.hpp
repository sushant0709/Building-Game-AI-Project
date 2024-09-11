#ifndef BOID_HPP
#define BOID_HPP
#include <SFML/Graphics.hpp>  
#include <cmath>  
#include "./Kinematic.hpp"

class Boid
{
    public:
        sf::Texture *texture;
        Kinematic data;
        sf::Sprite s;
        bool isMonster;

        Boid(sf::Texture *t, Kinematic d, bool isMonster);
        Boid(const Boid& other);
        void setPosition(sf::Vector2f position);
        sf::Vector2f checkBorderP(sf::Vector2f location);
        void move();
};
#endif