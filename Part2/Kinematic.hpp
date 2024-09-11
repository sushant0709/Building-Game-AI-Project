#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <cmath>

class SteeringOutput
{
    public:
        sf::Vector2f linear;
        float angular;

        // default constructor
        SteeringOutput()
        {
            this->linear = sf::Vector2f(0.0f, 0.0f);
            this->angular = 0.0f;
        }

        SteeringOutput(sf::Vector2f l, float a)
        {
            this->linear = l;
            this->angular = a;
        }
};

class Kinematic
{
    public:
        sf::Vector2f position; /* 2D position vector */
        float orientation; /* Orientation */
        sf::Vector2f velocity; /* Velocity */
        float rotation; /* Rotation */

        // Default constructor
        Kinematic();

        Kinematic(sf::Vector2f p, float o, sf::Vector2f v, float r);

        void update(SteeringOutput steering, float time);
};
#endif /*KINEMATIC_HPP*/
