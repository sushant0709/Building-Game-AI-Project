#include "./SteeringBehavior.hpp"
#include <SFML/System/Vector2.hpp>

class OrientationMatching : public SteeringBehavior
{
    public:
        float maxAngularAcc;
        float maxRot;
        float slowRadius;
        float targetRadius;
        float timeToTarget;
        OrientationMatching()
        {
            this->maxAngularAcc = 100.0f;
            this->maxRot = 0.5f;
            this->slowRadius = 0.2f;
            this->targetRadius = 0.001f;
            this->timeToTarget = 1.0f;
        }
        OrientationMatching(float ma, float mr, float sr, float tr, float ttg)
        {
            this->maxAngularAcc = ma;
            this->maxRot = mr;
            this->slowRadius = sr;
            this->targetRadius = tr;
            this->timeToTarget = ttg;
        }
        SteeringOutput CalculateSteering(Kinematic &character, Kinematic target) const override;
        float MapToRange(float theta) const;
};