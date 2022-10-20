/* Copyright (C) Greg Snook, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#define NAVIGATIONMESH_CPP
/****************************************************************************************\
	NavigationMesh.cpp

	NavigationMesh component implementation for the Navimesh sample program.
	Included as part of the Game Programming Gems sample code.

	Created 3/18/00 by Greg Snook
	greg@mightystudios.com
    ------------------------------------------------------------------------------------- 
	Notes/Revisions:

\****************************************************************************************/

#include "stdafx.h"
#include "navigationmesh.h"
#include "navigationpath.h"
#include <assert.h>
#include "bstreamext.h"
#include "DebugUtil.h"

//:	SnapPointToCell
//----------------------------------------------------------------------------------------
//
// Force a point to be inside the cell
//
//-------------------------------------------------------------------------------------://

void NavigationMesh::InitCell( int nNum )
{
	BsAssert( m_ppCells == NULL );
	m_nCellNum = nNum;
	m_ppCells = new NavigationCell* [nNum];
}


D3DXVECTOR3 NavigationMesh::SnapPointToCell(NavigationCell* Cell, const D3DXVECTOR3& Point)
{
	D3DXVECTOR3 PointOut = Point;

	if (!Cell->IsPointInCellCollumn(PointOut))
	{
		Cell->ForcePointToCellCollumn(PointOut);
	}

	Cell->MapVectorHeightToCell(PointOut);
	return (PointOut);
}

//:	SnapPointToMesh
//----------------------------------------------------------------------------------------
//
// Force a point to be inside the nearest cell on the mesh
//
//-------------------------------------------------------------------------------------://
D3DXVECTOR3 NavigationMesh::SnapPointToMesh(NavigationCell** CellOut, const D3DXVECTOR3& Point)
{
	D3DXVECTOR3 PointOut = Point;
	*CellOut = FindClosestCell(PointOut);
	assert( *CellOut );
//	DebugString("Cell can't find! %f, %f\n", Point.x, Point.z );
	return (SnapPointToCell(*CellOut, PointOut));
}

//:	FindClosestCell
//----------------------------------------------------------------------------------------
//
// Find the closest cell on the mesh to the given point
//
//-------------------------------------------------------------------------------------://
NavigationCell* NavigationMesh::FindClosestCell(const D3DXVECTOR3& Point)const
{
	float ClosestDistance = 3.4E+38f;
	float ClosestHeight = 3.4E+38f;
	bool FoundHomeCell = false;
	float ThisDistance;
	NavigationCell* ClosestCell=0;

	for( int i=0; i<m_nCellNum; i++ )
	{
		NavigationCell* pCell = m_ppCells[i];
		if (pCell->IsPointInCellCollumn(Point))
		{
			D3DXVECTOR3 NewPosition(Point);
			pCell->MapVectorHeightToCell(NewPosition);
			ThisDistance = fabsf(NewPosition.y - Point.y);
			if (FoundHomeCell)
			{
				if (ThisDistance < ClosestHeight)
				{
					ClosestCell = pCell;
					ClosestHeight = ThisDistance;
				}
			}
			else
			{
				ClosestCell = pCell;
				ClosestHeight = ThisDistance;
				FoundHomeCell = true;
			}
		}

		if (!FoundHomeCell)
		{
			D3DXVECTOR2 Start(pCell->CenterPoint().x, pCell->CenterPoint().z);
			D3DXVECTOR2 End(Point.x, Point.z);
			Line2D MotionPath(Start, End);
			NavigationCell* NextCell;
			NavigationCell::CELL_SIDE WallHit;
			D3DXVECTOR2 PointOfIntersection;

			NavigationCell::PATH_RESULT Result = pCell->ClassifyPathToCell(MotionPath, &NextCell, WallHit, &PointOfIntersection);

			if (Result == NavigationCell::EXITING_CELL)
			{
				D3DXVECTOR3 ClosestPoint3D(PointOfIntersection.x,0.0f,PointOfIntersection.y);
				pCell->MapVectorHeightToCell(ClosestPoint3D);
				ClosestPoint3D -= Point;
				ThisDistance = D3DXVec3Length( &ClosestPoint3D );
				if (ThisDistance<ClosestDistance)
				{
					ClosestDistance=ThisDistance;
					ClosestCell = pCell;
				}
			}
		}
	}
	return (ClosestCell);
}

//:	BuildNavigationPath
//----------------------------------------------------------------------------------------
//
// Build a navigation path using the provided points and the A* method
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::BuildNavigationPath(NavigationPath& NavPath, NavigationCell* StartCell, const D3DXVECTOR3& StartPos, NavigationCell* EndCell, const D3DXVECTOR3& EndPos)
{
	bool FoundPath = false;

	// Increment our path finding session ID
	// This Identifies each pathfinding session
	// so we do not need to clear out old data
	// in the cells from previous sessions.
	++m_PathSession;

	if( !StartCell->IsUse() || !EndCell->IsUse() )
	{
		return false;
	}

	// load our data into the NavigationHeap object
	// to prepare it for use.
	m_NavHeap.Setup(m_PathSession, StartPos);

	// We are doing a reverse search, from EndCell to StartCell.
	// Push our EndCell onto the Heap at the first cell to be processed
	EndCell->QueryForPath(&m_NavHeap, 0, 0);

	// process the heap until empty, or a path is found
	while(m_NavHeap.NotEmpty() && !FoundPath)
	{
		NavigationNode ThisNode;

		// pop the top cell (the open cell with the lowest cost) off the Heap
		m_NavHeap.GetTop(ThisNode);

		// if this cell is our StartCell, we are done
		if(ThisNode.cell == StartCell)
		{
			FoundPath = true;
		}
		else
		{
			// Process the Cell, Adding it's neighbors to the Heap as needed
			ThisNode.cell->ProcessCell(&m_NavHeap);
		}
	}

	// if we found a path, build a waypoint list
	// out of the cells on the path
	if (FoundPath)
	{
		NavigationCell* TestCell = StartCell;
		D3DXVECTOR3 NewWayPoint;

		// Setup the Path object, clearing out any old data
		NavPath.Setup(this, StartPos, StartCell, EndPos, EndCell);

		// Step through each cell linked by our A* algorythm 
		// from StartCell to EndCell
		while (TestCell && TestCell != EndCell)
		{
			// add the link point of the cell as a way point (the exit wall's center)
			int LinkWall = TestCell->ArrivalWall();

			NewWayPoint = TestCell->WallMidpoint(LinkWall);
			NewWayPoint = SnapPointToCell(TestCell, NewWayPoint); // just to be sure

			NavPath.AddWayPoint(NewWayPoint, TestCell);

			// and on to the next cell
			TestCell = TestCell->Link(LinkWall);
		}

		// cap the end of the path.
		NavPath.EndPath();
		return(true);
	}
	return(false);
}

//:	ResolveMotionOnMesh
//----------------------------------------------------------------------------------------
//
// Resolve a movement vector on the mesh
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::ResolveMotionOnMesh(const D3DXVECTOR3& StartPos, NavigationCell* StartCell, D3DXVECTOR3& EndPos, NavigationCell** EndCell)
{
	// create a 2D motion path from our Start and End positions, tossing out their Y values to project them 
	// down to the XZ plane.
	Line2D MotionPath(D3DXVECTOR2(StartPos.x,StartPos.z), D3DXVECTOR2(EndPos.x,EndPos.z));

	// these three will hold the results of our tests against the cell walls
	NavigationCell::PATH_RESULT Result = NavigationCell::NO_RELATIONSHIP;
	NavigationCell::CELL_SIDE WallNumber;
	D3DXVECTOR2 PointOfIntersection;
	NavigationCell* NextCell;

	// TestCell is the cell we are currently examining.
	NavigationCell* TestCell = StartCell;

	bool hitSoldWall = false;


	bool m_bNorelationship = false;
	D3DXVECTOR2 m_LastNewOrigin;

	//
	// Keep testing until we find our ending cell or stop moving due to friction
	//
	//while ((Result != NavigationCell::ENDING_CELL) && (MotionPath.EndPointA() != MotionPath.EndPointB()))
	while ((Result != NavigationCell::ENDING_CELL) && (MotionPath.Length() > 0.5f))
	{
		// use NavigationCell to determine how our path and cell interact
		Result = TestCell->ClassifyPathToCell(MotionPath, &NextCell, WallNumber, &PointOfIntersection);

		// if exiting the cell...
		if (Result == NavigationCell::EXITING_CELL)
		{
			// Set if we are moving to an adjacent cell or we have hit a solid (unlinked) edge
			if(NextCell)
			{
				// moving on. Set our motion origin to the point of intersection with this cell
				// and continue, using the new cell as our test cell.
				MotionPath.SetEndPointA(PointOfIntersection);
				TestCell = NextCell;

			}
			else
			{
				// we have hit a solid wall. Resolve the collision and correct our path.
				MotionPath.SetEndPointA(PointOfIntersection);
				TestCell->ProjectPathOnCellWall(WallNumber, MotionPath);
				hitSoldWall = true;

				// add some friction to the new MotionPath since we are scraping against a wall.
				// we do this by reducing the magnatude of our motion by 10%
				D3DXVECTOR2 Direction = MotionPath.EndPointB() - MotionPath.EndPointA();
				Direction *= 0.9f;
				MotionPath.SetEndPointB(MotionPath.EndPointA() + Direction);



			}
			m_bNorelationship = false;
		}
		else if (Result == NavigationCell::NO_RELATIONSHIP)
		{
			// Although theoretically we should never encounter this case,
			// we do sometimes find ourselves standing directly on a vertex of the cell.
			// This can be viewed by some routines as being outside the cell.
			// To accomodate this rare case, we can force our starting point to be within
			// the current cell by nudging it back so we may continue.
			D3DXVECTOR2 NewOrigin = MotionPath.EndPointA();

			if( m_bNorelationship )
			{
				if( m_LastNewOrigin.x == NewOrigin.x && m_LastNewOrigin.y == NewOrigin.y )
				{
					break;
				}
			}
			m_LastNewOrigin = NewOrigin;
			m_bNorelationship = true;
/*

			// 좌표가 같은 경우 무한루프 빠지는 경우가 있다.
			D3DXVECTOR3 CenterPos = TestCell->CenterPoint();

DebugString( "Nav panic1 %f, %f, %f, %f\n", CenterPos.x, CenterPos.z, NewOrigin.x, NewOrigin.y );

			if( CenterPos.x == NewOrigin.x && CenterPos.z == NewOrigin.y )
				break;
*/
			TestCell->ForcePointToCellCollumn(NewOrigin);
			MotionPath.SetEndPointA(NewOrigin);
		}
	}

	// we now have our new host cell
	*EndCell = TestCell;

	// Update the new control point position, 
	// solving for Y using the Plane member of the NavigationCell
	EndPos.x = MotionPath.EndPointB().x;
	EndPos.z = MotionPath.EndPointB().y;
	TestCell->MapVectorHeightToCell(EndPos);

	return hitSoldWall;
}



bool NavigationMesh::LineOfSightTest(NavigationCell* StartCell, const D3DXVECTOR3& StartPos, NavigationCell* EndCell, const D3DXVECTOR3& EndPos)
{
	Line2D MotionPath(D3DXVECTOR2(StartPos.x,StartPos.z), D3DXVECTOR2(EndPos.x,EndPos.z));
	NavigationCell* NextCell = StartCell;
	NavigationCell::CELL_SIDE WallNumber;
	NavigationCell::PATH_RESULT Result;
 	while((Result = NextCell->ClassifyPathToCell(MotionPath, &NextCell, WallNumber, 0)) == NavigationCell::EXITING_CELL)
	{
		if (!NextCell)
		{
			return(false);
		}
		else
		{
			if( NextCell == EndCell )
			{
				return true;
			}
		}
	}
	return (Result == NavigationCell::ENDING_CELL);
}


/*

//:	LineOfSightTest
//----------------------------------------------------------------------------------------
//
// Test to see if two points on the mesh can view each other
//
//-------------------------------------------------------------------------------------://
bool NavigationMesh::LineOfSightTest(NavigationCell* StartCell, const D3DXVECTOR3& StartPos, NavigationCell* EndCell, const D3DXVECTOR3& EndPos)
{
	Line2D MotionPath(D3DXVECTOR2(StartPos.x,StartPos.z), D3DXVECTOR2(EndPos.x,EndPos.z));
	NavigationCell* NextCell = StartCell;
	NavigationCell::CELL_SIDE WallNumber;
	NavigationCell::PATH_RESULT Result;

	while((Result = NextCell->ClassifyPathToCell(MotionPath, &NextCell, WallNumber, 0)) == NavigationCell::EXITING_CELL)
	{
		if (!NextCell) return(false);
	}

	return (Result == NavigationCell::ENDING_CELL);
}
*/
//:	LinkCells
//----------------------------------------------------------------------------------------
//
// Link all the cells that are in our pool
//
//-------------------------------------------------------------------------------------://
void NavigationMesh::LinkCells()
{
	for( int i=0; i<m_nCellNum; i++ )
	{
		NavigationCell* pCellA = m_ppCells[i];
		for( int j=0; j<m_nCellNum; j++ )
		{
			NavigationCell* pCellB = m_ppCells[j];
			if(i != j)
			{
				if (!pCellA->Link(NavigationCell::SIDE_AB) && pCellB->RequestLink(pCellA->Vertex(0), pCellA->Vertex(1), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_AB, pCellB);
				}
				else if (!pCellA->Link(NavigationCell::SIDE_BC) && pCellB->RequestLink(pCellA->Vertex(1), pCellA->Vertex(2), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_BC, pCellB);
				}
				else if (!pCellA->Link(NavigationCell::SIDE_CA) && pCellB->RequestLink(pCellA->Vertex(2), pCellA->Vertex(0), pCellA))
				{
					pCellA->SetLink(NavigationCell::SIDE_CA, pCellB);
				}
			}
		}
	}
}


bool NavigationMesh::Save( BStream* pStream )
{
	if(!pStream->Valid()){
//		CBsConsole::GetInstance().AddFormatString("%s File Not Found", pFullName);
		return false;
	}

	pStream->Write( &m_nCellNum, sizeof(int), 4 );
	for( int i=0; i<m_nCellNum; i++ )
	{
		m_ppCells[i]->Save( pStream );
	}
	for( int i=0; i<m_nCellNum; i++ )
	{
		for( int j=0; j<3; j++ )
		{
			NavigationCell* pCurCell = m_ppCells[i]->GetLinkCell( j );
			int nIndex = GetCellIndex( pCurCell );
			pStream->Write( &nIndex, sizeof(int), 4 );
		}
	}
	return true;
}

bool NavigationMesh::Load( BStream* pStream )
{
	assert( (int)m_nCellNum == 0 );
	pStream->Read( &m_nCellNum, sizeof(int), 4 );

	InitCell( m_nCellNum );

	for( int i=0; i<m_nCellNum; i++ )
	{
		NavigationCell* NewCell = new NavigationCell;
		
		m_ppCells[i] =  NewCell;
		m_ppCells[i]->Load( pStream );
	}
	for( int i=0; i<m_nCellNum; i++ )
	{
		NavigationCell* pCurCell = Cell( i );
		for( int j=0; j<3; j++ )
		{
			int nIndex;
			pStream->Read( &nIndex, sizeof(int), 4 );
			if( nIndex != -1 )
			{
				NavigationCell* pCell = Cell( nIndex );
				pCurCell->SetLink( (NavigationCell::CELL_SIDE)j, pCell );
			}
		}
	}
	return true;
}



int NavigationMesh::GetCellIndex( NavigationCell* pCell )
{
	if( pCell == NULL )
		return -1;

	for( int i=0; i<m_nCellNum; i++ )
	{
		if( m_ppCells[i] == pCell )
			return i;
	}
	return -1;
}

//****************************************************************************************
// end of file      ( NavigationMesh.cpp )

