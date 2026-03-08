#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	TrimWorld->SetTrimWorldSize(3000.f);
	TrimWorld->bShouldTrimWorld = true;

	TSubclassOf<ASteeringAgent> AgentClassToUse = SteeringAgentClass
		? SteeringAgentClass
		: TSubclassOf<ASteeringAgent>(ASteeringAgent::StaticClass());

	if (GetWorld() && AgentClassToUse)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Place agent roughly in the center of the trimmed world, Z raised so character doesn't intersect ground
		FVector SpawnLocation(TrimWorld->GetTrimWorldSize() * 0.5f, TrimWorld->GetTrimWorldSize() * 0.5f, 90.f);

		pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(AgentClassToUse, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (IsValid(pAgentToEvade))
		{
			// Create the evade agent's behavior and assign it (non-owning pointer stored on the actor)
			pAgentToEvadeBehavior = std::make_unique<Wander>();

			pAgentToEvade->SetSteeringBehavior(pAgentToEvadeBehavior.get());
			pAgentToEvade->SetIsAutoOrienting(true);

			// If you need to explicitly set position API on your ABaseAgent, use SetActorLocation:
			pAgentToEvade->SetActorLocation(SpawnLocation);
			pAgentToEvade->SetDebugRenderingEnabled(true);
		}
	}

	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
			);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	if (bUseMouseTarget)
		pFlock->SetTarget_Seek(MouseTarget);
}

