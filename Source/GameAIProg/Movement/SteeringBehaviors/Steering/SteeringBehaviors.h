#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Your own SteeringBehaviors should follow here...

// Seek
class Seek : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Flee
class Flee : public ISteeringBehavior
{
public:

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Arrive
class Arrive : public ISteeringBehavior
{
public:
	//arrive behaviour
	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	//arrive functions
	void SetSlowRadius(float radius) { m_SlowRadius = radius; }
	void SetTargetRadius(float radius) { m_TargetRadius = radius; }

protected:
	float m_SlowRadius = 500.f;
	float m_TargetRadius = 100.f;
};

// Persuit
class Pursuit : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Evade
class Evade : public Pursuit
{
public:
	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Wander
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

private:
	float m_OffsetDistance = 6.f;
	float m_Radius = 50.f;
	float m_MaxAngleChange = 45 * 3.1415926535f / 180;
	float m_WanderAngle = 0.f;
};