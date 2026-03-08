#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	Cells.reserve(Rows * Cols);
	for (int row = 0; row < Rows; row++)
	{
		for (int col = 0; col < Cols; col++)
		{
			float left = col * CellWidth;
			float bottom = row * CellHeight;
			Cells.emplace_back(left, bottom, CellWidth, CellHeight);
		}
	}
	Neighbors.SetNum(MaxEntities);
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// TODO Add the agent to the correct cell
	int cellIndex = PositionToIndex(Agent.GetPosition());

	Cells[cellIndex].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int oldCellIndex = PositionToIndex(OldPos);
	int newIndex = PositionToIndex(Agent.GetPosition());

	if (oldCellIndex != newIndex)
	{
		Cells[oldCellIndex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;

	FVector2D agentPos = Agent.GetPosition();

	// Calculate the neighborhood bounds
	float left		= agentPos.X - QueryRadius;
	float right		= agentPos.X + QueryRadius;
	float bottom	= agentPos.Y - QueryRadius;
	float top		= agentPos.Y + QueryRadius;

	// Calculate the rang of cells to check
	int startCol	= std::max(0, static_cast<int>(left / CellWidth));
	int endCol		= std::min(NrOfCols - 1, static_cast<int>(right / CellWidth));
	int startRow	= std::max(0, static_cast<int>(bottom / CellHeight));
	int endRow		= std::min(NrOfRows - 1, static_cast<int>(top / CellHeight));

	// Iterate through the cells within the neighborhood bounds
	for (int row = startRow; row < endRow; row++)
	{
		for (int col = startCol; col < endCol; col++)
		{
			int cellIndex = row * NrOfCols + col;
			Cell& cell = Cells[cellIndex];

			// Check each agent in the cell
			for (ASteeringAgent* agent : cell.Agents)
			{
				if (agent != &Agent)
				{
					float distanceSquared = (agent->GetPosition() - agentPos).SquaredLength();
					if (distanceSquared <= QueryRadius * QueryRadius)
					{
						Neighbors[NrOfNeighbors] = agent;
						++NrOfNeighbors;
					}
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// Render the cells with the number of agents inside of it
	for (const Cell& cell : Cells)
	{
		// Get the rectangle points of the cell
		std::vector<FVector2D> rectPoints = cell.GetRectPoints();

		// Draw the cell's bounding box using debug lines
		for (int i = 0; i < static_cast<int>(rectPoints.size()); ++i)
		{
			int nextIndex = (i + 1) % rectPoints.size();
			FVector Start = FVector(rectPoints[i].X, rectPoints[i].Y, 20.f);
			FVector End = FVector(rectPoints[nextIndex].X, rectPoints[nextIndex].Y, 20.f);

			DrawDebugLine(pWorld, Start, End, FColor::Red, false, -1.f, 0, 1.f);
		}

		// Draw the number of agents inside the cell
		float centerX = (cell.BoundingBox.Min.X + cell.BoundingBox.Max.X) * 0.5f;
		float centerY = (cell.BoundingBox.Min.Y + cell.BoundingBox.Max.Y) * 0.5f;
		FVector TextPos = FVector(centerX, centerY, 20.f);

		FString AgentCount = FString::FromInt(cell.Agents.size());
		DrawDebugString(pWorld, TextPos, AgentCount, nullptr, FColor::White, -1.f, true);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	// TODO Calculate the index of the cell based on the position
	int col = static_cast<int>(Pos.X / CellWidth);
	col = std::clamp(col, 0, NrOfCols - 1);

	int row = static_cast<int>(Pos.Y / CellHeight);
	row = std::clamp(row, 0, NrOfRows - 1);
	return row * NrOfCols + col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}