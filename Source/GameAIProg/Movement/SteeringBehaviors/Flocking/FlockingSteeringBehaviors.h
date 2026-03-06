#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	Cohesion(Flock* const pFlock) :m_pFlock(pFlock) {};

	//Cohesion Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation : public Seek
{
public:
	Separation(Flock* const pFLock) : m_pFlock(pFLock) {};

	//Seperation Behvior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* m_pFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch : public Seek
{
public:
	VelocityMatch(Flock* const pFlock) : m_pFlock(pFlock) { };

	//Velocity Match Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;
private:
	Flock* m_pFlock;
};