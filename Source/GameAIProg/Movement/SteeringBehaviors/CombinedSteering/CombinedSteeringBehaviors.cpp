
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	float totalWeight{};
	// TODO: Calculate the weighted average steeringbehavior
	for (const BlendedSteering::WeightedBehavior& weightedBehavior : WeightedBehaviors)
	{
		SteeringOutput steering = weightedBehavior.pBehavior->CalculateSteering(DeltaT, Agent);
		BlendedSteering.LinearVelocity += steering.LinearVelocity * weightedBehavior.Weight;
		BlendedSteering.AngularVelocity += steering.AngularVelocity * weightedBehavior.Weight;
		totalWeight += weightedBehavior.Weight;
	}
	if (totalWeight > 0.f)
	{
		BlendedSteering /= totalWeight;
	}

	// TODO: Add debug drawing
	if (Agent.GetDebugRenderingEnabled())
	{
		// Draw direction arrow
		FVector2D Direction = BlendedSteering.LinearVelocity;
		Direction.Normalize();

		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ Direction * 150.f, 0.f },
			500.f,
			FColor::Magenta,
			false,
			-1.f,
			0,
			2.f
		);
	}
	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}