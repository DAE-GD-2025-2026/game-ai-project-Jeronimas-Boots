#include "Quadtree.h"

bool FQuadtreeAABB::ContainsPoint(const FVector2D& Point) const
{
	return (Point.X >= Center.X - HalfDimension.X && Point.X <= Center.X + HalfDimension.X &&
		Point.Y >= Center.Y - HalfDimension.Y && Point.Y <= Center.Y + HalfDimension.Y);
}

bool FQuadtreeAABB::IntersectsAABB(const FQuadtreeAABB& Other) const
{
	return !(Center.X + HalfDimension.X < Other.Center.X - Other.HalfDimension.X ||
		Center.X - HalfDimension.X > Other.Center.X + Other.HalfDimension.X ||
		Center.Y + HalfDimension.Y < Other.Center.Y - Other.HalfDimension.Y ||
		Center.Y - HalfDimension.Y > Other.Center.Y + Other.HalfDimension.Y);
}

#include "DrawDebugHelpers.h"

Quadtree::Quadtree(FQuadtreeAABB InBoundary, int InCapacity)
	: Boundary(InBoundary), Capacity(InCapacity)
{
}

Quadtree::~Quadtree()
{
	Clear();
}

bool Quadtree::Insert(ASteeringAgent* Agent)
{
	if (!Agent || !Boundary.ContainsPoint(Agent->GetPosition()))
		return false;

	// Add agent to this node if we haven't reached capacity
	if (Agents.Num() < Capacity && !bDivided)
	{
		Agents.Add(Agent);
		return true;
	}

	// Subdivide if not already divided
	if (!bDivided)
	{
		Subdivide();
	}

	// Attempt insertion in children
	if (NorthWest->Insert(Agent)) return true;
	if (NorthEast->Insert(Agent)) return true;
	if (SouthWest->Insert(Agent)) return true;
	if (SouthEast->Insert(Agent)) return true;

	return false;
}

void Quadtree::Subdivide()
{
	FVector2D Center = Boundary.Center;
	FVector2D HalfDim = Boundary.HalfDimension;
	FVector2D QHalfDim(HalfDim.X * 0.5f, HalfDim.Y * 0.5f);

	NorthWest = new Quadtree(FQuadtreeAABB{ FVector2D(Center.X - QHalfDim.X, Center.Y + QHalfDim.Y), QHalfDim }, Capacity);
	NorthEast = new Quadtree(FQuadtreeAABB{ FVector2D(Center.X + QHalfDim.X, Center.Y + QHalfDim.Y), QHalfDim }, Capacity);
	SouthWest = new Quadtree(FQuadtreeAABB{ FVector2D(Center.X - QHalfDim.X, Center.Y - QHalfDim.Y), QHalfDim }, Capacity);
	SouthEast = new Quadtree(FQuadtreeAABB{ FVector2D(Center.X + QHalfDim.X, Center.Y - QHalfDim.Y), QHalfDim }, Capacity);

	bDivided = true;
}

void Quadtree::Query(const FQuadtreeAABB& Range, TArray<ASteeringAgent*>& Found) const
{
	if (!Boundary.IntersectsAABB(Range))
		return;

	// Add contained agents 
	for (ASteeringAgent* Agent : Agents)
	{
		if (Range.ContainsPoint(Agent->GetPosition()))
		{
			Found.Add(Agent);
		}
	}

	if (bDivided)
	{
		NorthWest->Query(Range, Found);
		NorthEast->Query(Range, Found);
		SouthWest->Query(Range, Found);
		SouthEast->Query(Range, Found);
	}
}

void Quadtree::Clear()
{
	Agents.Empty();
	if (bDivided)
	{
		delete NorthWest;
		delete NorthEast;
		delete SouthWest;
		delete SouthEast;
		bDivided = false;
	}
}

void Quadtree::RenderDebug(UWorld* World) const
{
	if (!World) return;

	FVector Center(Boundary.Center.X, Boundary.Center.Y, 20.f);
	FVector Extent(Boundary.HalfDimension.X, Boundary.HalfDimension.Y, 0.f);

	DrawDebugBox(World, Center, Extent, FColor::Cyan, false, -1.f, 0, 8.f);

	if (bDivided)
	{
		NorthWest->RenderDebug(World);
		NorthEast->RenderDebug(World);
		SouthWest->RenderDebug(World);
		SouthEast->RenderDebug(World);
	}
}