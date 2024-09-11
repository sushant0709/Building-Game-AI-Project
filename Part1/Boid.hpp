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

        Boid(sf::Texture *t, Kinematic d);
        Boid(const Boid& other);
        void setPosition(sf::Vector2f position);
        sf::Vector2f checkBorderP(sf::Vector2f location);
        void move();
};
#endif