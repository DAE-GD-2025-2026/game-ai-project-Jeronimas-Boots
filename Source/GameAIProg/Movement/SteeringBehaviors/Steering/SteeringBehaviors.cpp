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
	const float distance = Steering.LinearVelocity.Length();
	Steering.LinearVelocity.Normalize();

	float fullGass = Agent.GetMaxLinearSpeed();

	if (distance < m_TargetRadius)
	{
		// Inside target radius - stop completely
		Steering.LinearVelocity = FVector2D::ZeroVector;
	}
	else if (distance < m_SlowRadius)
	{
		Agent.SetMaxLinearSpeed(200);
	}
	else
	{
		Agent.SetMaxLinearSpeed(600);
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

// PURSUIT
//********
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	// Calculate distance to target
	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float distance = ToTarget.Length();

	// Calculate time to reach target: t = d/v
	float speed = Agent.GetLinearVelocity().Length();
	if (speed <= 0.f)
		speed = Agent.GetMaxLinearSpeed(); // Use max speed if currently stationary

	float timeToTarget = distance / speed;

	// Predict future position: futurePosition = currentPosition + velocity * time
	FVector2D PredictedPosition = Target.Position + (Target.LinearVelocity * timeToTarget);

	// Seek to the predicted position
	Steering.LinearVelocity = PredictedPosition - Agent.GetPosition();

	// Debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		// Draw arrow to predicted position
		FVector2D Direction = Steering.LinearVelocity;
		Direction.Normalize();

		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{ Direction * 150.f, 0.f },
			500.f,
			FColor::Purple,
			false,
			-1.f,
			0,
			2.f
		);

		// Draw predicted target position
		DrawDebugCircle(
			Agent.GetWorld(),
			FVector{ PredictedPosition, 0.f },
			50.f,
			12,
			FColor::Yellow,
			false,
			-1.f,
			0,
			3.f,
			FVector(1.f, 0.f, 0.f), // Y-Axis (X direction in world)
			FVector(0.f, 1.f, 0.f), // Z-Axis (Y direction in world)
			false);					// Draw axis
	}

	return Steering;
}

SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = Pursuit::CalculateSteering(DeltaT, Agent);
	Steering.LinearVelocity = Steering.LinearVelocity * (-1);
	return Steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};

	// Update wander angle with random offset (smooth wandering)
	float angleChange = FMath::RandRange(-m_MaxAngleChange, m_MaxAngleChange);
	m_WanderAngle += angleChange;

	// Calculate circle center position (in front of the agent)
	FVector2D agentVelocity = Agent.GetLinearVelocity();
	if (agentVelocity.Length() > 0.f)
	{
		agentVelocity.Normalize();
	}
	else
	{
		// If agent is stationary, use forward direction
		agentVelocity = FVector2D(1.f, 0.f);
	}

	FVector2D circleCenter = Agent.GetPosition() + (agentVelocity * m_OffsetDistance);

	// Calculate point on circle using angle
	FVector2D circleOffset;
	circleOffset.X = FMath::Cos(m_WanderAngle) * m_Radius;
	circleOffset.Y = FMath::Sin(m_WanderAngle) * m_Radius;

	// Target is the point on the circle
	FVector2D wanderTarget = circleCenter + circleOffset;

	// Seek towards the wander target
	Steering.LinearVelocity = wanderTarget - Agent.GetPosition();

	// Debug Rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		// Draw the wander circle
		DrawDebugCircle(
			Agent.GetWorld(),
			FVector{ circleCenter, 0.f },
			m_Radius,
			16,						// Segments
			FColor::Cyan,			// Color
			false,					// Persistent lines
			-1.f,					// Lifetime
			0,						// Depth priority
			2.f,					// Thickness
			FVector(1.f, 0.f, 0.f), // Y-Axis (X direction in world)
			FVector(0.f, 1.f, 0.f), // Z-Axis (Y direction in world)
			false);					// Draw axis

		// Draw the target point on the circle
		DrawDebugCircle(
			Agent.GetWorld(),
			FVector{ wanderTarget, 0.f },
			25.f,					// Small circle at target
			8,
			FColor::Red,
			false,
			-1.f,
			0,
			3.f,
			FVector(1.f, 0.f, 0.f),
			FVector(0.f, 1.f, 0.f),
			false);

		// Draw direction arrow
		FVector2D Direction = Steering.LinearVelocity;
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

	return Steering;
}
