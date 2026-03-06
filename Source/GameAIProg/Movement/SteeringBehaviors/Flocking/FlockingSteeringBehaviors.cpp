#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborPos() - pAgent.GetPosition();
	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Seperation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	FVector2D seperationForce = FVector2D::ZeroVector;

	const TArray<ASteeringAgent*>& neighbors = m_pFlock->GetNeighbors();
	int nrOfNeighbors = m_pFlock->GetNrOfNeighbors();

	for (int i = 0; i < nrOfNeighbors; i++)
	{
		ASteeringAgent* pNeighbor = neighbors[i];
		FVector2D toAgent = pAgent.GetPosition() - pNeighbor->GetPosition();
		float distance = toAgent.Length();

		if (distance > 0)
			seperationForce += toAgent.GetSafeNormal() / distance;
	}

	seperationForce.Normalize();
	seperationForce *= pAgent.GetMaxLinearSpeed();
	steering.LinearVelocity = seperationForce;

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	return steering;
}
