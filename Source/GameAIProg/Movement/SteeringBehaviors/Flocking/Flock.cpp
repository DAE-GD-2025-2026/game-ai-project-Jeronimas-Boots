#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{ pWorld }
	, FlockSize{ FlockSize }
	, pAgentToEvade{ pAgentToEvade }
{
	Agents.SetNum(FlockSize);
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.SetNum(FlockSize);
#endif

	// Initialize steering behaviors
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);

	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();

	// initialize blended steering
	pBlendedSteering = std::make_unique<BlendedSteering>(
		std::vector<BlendedSteering::WeightedBehavior>{
			{pCohesionBehavior.get(), 0.80f},
			{ pSeparationBehavior.get(), 0.15f },
			{ pVelMatchBehavior.get(), 0.25f },
			{ pSeekBehavior.get(), 0.25f },
			{ pWanderBehavior.get(), 0.25f }
	});
	pEvadeBehavior->SetRadius(450.f);

	// priority steering
	pPrioritySteering = std::make_unique<PrioritySteering>(
		std::vector<ISteeringBehavior*>
	{
		pEvadeBehavior.get(), pBlendedSteering.get()
	});

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	// Create the partitioned space (using NrOfCellsX for both rows and cols)
	float FullSize = WorldSize * 2.f;
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, FullSize, FullSize, NrOfCellsX, NrOfCellsX, FlockSize);
	OldPositions.SetNum(FlockSize);
#endif

	// Initialize agents
	if (pWorld && AgentClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int i = 0; i < FlockSize; ++i)
		{
			// Spawn agent at random position within world bounds
			FVector SpawnLocation(
				FMath::RandRange(0.f, WorldSize / 2),
				FMath::RandRange(0.f, WorldSize / 2),
				10.f // Z-position raised above ground to prevent agents falling through
			);

			Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

			if (Agents[i])
			{
				Agents[i]->SetSteeringBehavior(pPrioritySteering.get());
				Agents[i]->SetIsAutoOrienting(true);

#ifdef GAMEAI_USE_SPACE_PARTITIONING
				pPartitionedSpace->AddAgent(*Agents[i]);
				OldPositions[i] = Agents[i]->GetPosition();
#endif
			}
		}
	}
}

Flock::~Flock()
{
	for (ASteeringAgent* pAgent : Agents)
	{
		pAgent->Destroy();
	}
	Agents.Empty();
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.Empty();
#endif
}

void Flock::Tick(float DeltaTime)
{
	// Update the agent to evade's data

	FSteeringParams AgentToEvadeParams;
	AgentToEvadeParams.Position = pAgentToEvade->GetPosition();
	AgentToEvadeParams.LinearVelocity = pAgentToEvade->GetLinearVelocity();
	AgentToEvadeParams.Orientation = pAgentToEvade->GetRotation();
	AgentToEvadeParams.AngularVelocity = pAgentToEvade->GetAngularVelocity();

	pEvadeBehavior->SetTarget(AgentToEvadeParams);

	// Update each agent in the flock
	for (int i = 0; i < Agents.Num(); ++i)
	{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		// Register neighbors via the cell space
		pPartitionedSpace->RegisterNeighbors(*Agents[i], NeighborhoodRadius);
#else
		// Register neighbors via brute-force
		RegisterNeighbors(Agents[i]);
#endif

		// Update the agent (steering behaviors use the neighbors in the memory pool)
		Agents[i]->Tick(DeltaTime);

#ifdef GAMEAI_USE_SPACE_PARTITIONING
		// Update the agent's cell after it has moved
		pPartitionedSpace->UpdateAgentCell(*Agents[i], OldPositions[i]);
		OldPositions[i] = Agents[i]->GetPosition();
#endif
	}

	// Update the agent to evade
	pAgentToEvade->Tick(DeltaTime);
}

void Flock::RenderDebug()
{
	if (DebugRenderSteering)
	{
		Agents[0]->SetDebugRenderingEnabled(true);
		pAgentToEvade->SetDebugRenderingEnabled(true);
	}
	else
	{
		for (ASteeringAgent* pAgent : Agents)
		{
			if (pAgent)
			{
				pAgent->SetDebugRenderingEnabled(false);
			}
		}
	}

	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();

#ifdef GAMEAI_USE_SPACE_PARTITIONING
		if (DebugRenderPartitions)
		{
			pPartitionedSpace->RenderCells();
		}
#endif
	}
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

		ImGui::Checkbox("Debug Steering", &DebugRenderSteering);
		ImGui::Checkbox("Debug Neighborhood", &DebugRenderNeighborhood);
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		ImGui::Checkbox("Debug Partitions", &DebugRenderPartitions);
#endif

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		auto& weightedBehaviors = pBlendedSteering->GetWeightedBehaviorsRef();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			weightedBehaviors[0].Weight, 0.f, 1.f,
			[&weightedBehaviors](float InVal) { weightedBehaviors[0].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			weightedBehaviors[1].Weight, 0.f, 1.f,
			[&weightedBehaviors](float InVal) { weightedBehaviors[1].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("VelocityMatch",
			weightedBehaviors[2].Weight, 0.f, 1.f,
			[&weightedBehaviors](float InVal) { weightedBehaviors[2].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			weightedBehaviors[3].Weight, 0.f, 1.f,
			[&weightedBehaviors](float InVal) { weightedBehaviors[3].Weight = InVal; }, "%.2f");
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			weightedBehaviors[4].Weight, 0.f, 1.f,
			[&weightedBehaviors](float InVal) { weightedBehaviors[4].Weight = InVal; }, "%.2f");
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
#ifdef GAMEAI_USE_SPACE_PARTITIONING
	// Register neighbors for the first agent via cell space
	pPartitionedSpace->RegisterNeighbors(*Agents[0], NeighborhoodRadius);
	const TArray<ASteeringAgent*>& neighbors = pPartitionedSpace->GetNeighbors();
	int nrOfNeighbors = pPartitionedSpace->GetNrOfNeighbors();
#else
	// Register neighbors for the first agent via brute-force
	RegisterNeighbors(Agents[0]);
	const TArray<ASteeringAgent*>& neighbors = Neighbors;
	int nrOfNeighbors = NrOfNeighbors;
#endif

	for (int i = 0; i < nrOfNeighbors; ++i)
	{
		ASteeringAgent* pNeighbor = neighbors[i];
		FVector FirstNeighborPos = FVector(pNeighbor->GetPosition().X, pNeighbor->GetPosition().Y, 20.f);
		DrawDebugCircle(pWorld, FirstNeighborPos, 30.f, 20, FColor::Green, false, -1.f, 0, 4.f, FVector(0, 1, 0), FVector(1, 0, 0));
	}

	// Draw the first agent as a blue circle
	FVector FirstAgentPos = FVector(Agents[0]->GetPosition().X, Agents[0]->GetPosition().Y, 20.f);
	DrawDebugCircle(pWorld, FirstAgentPos, 60, 32, FColor::Blue, false, -1.f, 0, 4.f, FVector(0, 1, 0), FVector(1, 0, 0));

	// Draw the neighborhood radius as a blue outline circle
	DrawDebugCircle(pWorld, FirstAgentPos, NeighborhoodRadius, 64, FColor::Blue, false, -1.f, 0, 1.f, FVector(0, 1, 0), FVector(1, 0, 0));

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	// Draw the neighborhood square and highlight overlapping cells
	pPartitionedSpace->RenderNeighborhood(Agents[0]->GetPosition(), NeighborhoodRadius);
#endif
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	for (auto& agent : Agents)
	{
		float distanceSquared = FVector2D::DistSquared(agent->GetPosition(), pAgent->GetPosition());
		if (distanceSquared < NeighborhoodRadius * NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors] = agent;
			++NrOfNeighbors;
		}
	}

}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	const TArray<ASteeringAgent*>& neighbors = pPartitionedSpace->GetNeighbors();
	int nrOfNeighbors = pPartitionedSpace->GetNrOfNeighbors();
#else
	const TArray<ASteeringAgent*>& neighbors = Neighbors;
	int nrOfNeighbors = NrOfNeighbors;
#endif

	for (int i = 0; i < nrOfNeighbors; i++)
	{
		avgPosition += neighbors[i]->GetPosition();
	}

	if (nrOfNeighbors > 0)
		avgPosition /= static_cast<float>(nrOfNeighbors);

	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	const TArray<ASteeringAgent*>& neighbors = pPartitionedSpace->GetNeighbors();
	int nrOfNeighbors = pPartitionedSpace->GetNrOfNeighbors();
#else
	const TArray<ASteeringAgent*>& neighbors = Neighbors;
	int nrOfNeighbors = NrOfNeighbors;
#endif

	for (int i = 0; i < nrOfNeighbors; i++)
	{
		avgVelocity += neighbors[i]->GetLinearVelocity();
	}

	if (nrOfNeighbors > 0)
		avgVelocity /= static_cast<float>(nrOfNeighbors);

	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pSeekBehavior->SetTarget(Target);
}