#pragma once
#include "CoreMinimal.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"

// Struct representing the rectangular bounds for our Quadtree nodes
struct FQuadtreeAABB
{
	FVector2D Center;
	FVector2D HalfDimension; // Half of width/height

	bool ContainsPoint(const FVector2D& Point) const;

	bool IntersectsAABB(const FQuadtreeAABB& Other) const;
};

class Quadtree
{
public:
	Quadtree(FQuadtreeAABB Boundary, int Capacity = 4);
	~Quadtree();

	// Insert an agent. Returns false if agent is outside the boundary.
	bool Insert(ASteeringAgent* Agent);

	// Find all agents within the provided range box
	void Query(const FQuadtreeAABB& Range, TArray<ASteeringAgent*>& Found) const;

	// Reset tree for the next frame
	void Clear();

	// Render the node bounds for debugging
	void RenderDebug(UWorld* World) const;

private:
	void Subdivide();

	FQuadtreeAABB Boundary;
	int Capacity;
	TArray<ASteeringAgent*> Agents;
	bool bDivided = false;

	// Child nodes
	Quadtree* NorthWest = nullptr;
	Quadtree* NorthEast = nullptr;
	Quadtree* SouthWest = nullptr;
	Quadtree* SouthEast = nullptr;
};