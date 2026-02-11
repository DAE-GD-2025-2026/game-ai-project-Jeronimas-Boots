#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();

	// Debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		FVector2D Direction = Steering.LinearVelocity;
		Direction.Normalize();

		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ Direction * 150.f, 0.f },
			500.f,
			FColor::Green,
			false,
			-1.f,
			0,
			2.f
		);
	}

	return Steering;
}

//FLEE
//*******

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Agent.GetPosition() - Target.Position;

	// Debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		FVector2D Direction = Steering.LinearVelocity;
		Direction.Normalize();

		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ Direction * 150.f, 0.f },
			500.f,
			FColor::Green,
			false,
			-1.f,
			0,
			2.f
		);
	}

	return Steering;
}

// ARRIVE
//*******
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	Steering.LinearVelocity.Normalize();
	const float distance = Steering.LinearVelocity.Length();

	if (distance < m_TargetRadius)
	{
		// Inside target radius - stop completely
		Steering.LinearVelocity = FVector2D::ZeroVector;
	}
	else if (distance < m_SlowRadius)
	{
		// Inside slow radius - gradually slow down
		Steering.LinearVelocity *= Agent.GetMaxLinearSpeed() * (distance / m_SlowRadius);
	}
	else
	{
		// Outside slow radius - full speed
		Steering.LinearVelocity *= Agent.GetMaxLinearSpeed();
	}

	// Debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		// Outer circle
		DrawDebugCircle(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			m_SlowRadius,
			10,						// Segments
			FColor::Blue,			// Color
			false,					// Persistent lines
			-1.f,					// Lifetime
			0,						// Depth priority
			2.f,					// Thickness
			FVector(1.f, 0.f, 0.f), // Y-Axis (X direction in world)
			FVector(0.f, 1.f, 0.f), // Z-Axis (Y direction in world)
			false);					// Draw axis

		// Inner Circle
		DrawDebugCircle(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			m_TargetRadius,
			10,						// Segments
			FColor::Red,			// Color
			false,					// Persistent lines
			-1.f,					// Lifetime
			0,						// Depth priority
			2.f,					// Thickness
			FVector(1.f, 0.f, 0.f), // Y-Axis (X direction in world)
			FVector(0.f, 1.f, 0.f), // Z-Axis (Y direction in world)
			false);					// Draw axis

		// Direction arrow
		FVector2D Direction = Steering.LinearVelocity;
		Direction.Normalize();

		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ Direction * 150.f, 0.f },
			500.f,
			FColor::Green,
			false,
			-1.f,
			0,
			2.f
		);
	}

	return Steering;
}
