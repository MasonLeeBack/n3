/* Copyright (C) Greg Snook, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#ifndef NAVIGATIONMESH_H
#define NAVIGATIONMESH_H
/****************************************************************************************\
	NavigationMesh.h

	NavigationMesh component interface for the Navimesh sample program.
	Included as part of the Game Programming Gems sample code.

	Created 3/18/00 by Greg Snook
	greg@mightystudios.com
    ------------------------------------------------------------------------------------- 
	Notes/Revisions:

\****************************************************************************************/
#include "navigationcell.h"
#include "navigationheap.h"
#include <vector>

// forward declaration required
class NavigationPath;

/*	NavigationMesh
------------------------------------------------------------------------------------------
	
	A NavigationMesh is a collecion of NavigationCells used to control object movement while
	also providing path finding line-of-sight testing. It serves as a parent to all the
	Actor objects which exist upon it.
	
------------------------------------------------------------------------------------------
*/

class NavigationMesh
{
public:
	// ----- CREATORS ---------------------

	NavigationMesh();
	~NavigationMesh();

	// ----- OPERATORS --------------------

	// ----- MUTATORS ---------------------
	void Clear();

	void InitCell( int nNum );

	int AddCell(const D3DXVECTOR3& PointA, const D3DXVECTOR3& PointB, const D3DXVECTOR3& PointC, int nIndex );
	int AddCell(NavigationCell* Cell, int nIndex);
	void RemoveCell(int index, int num = 1);


	void SetUseCell( std::vector<NavigationCell*>& CellArray, int index, int num );
	void SetUnuseCell(int index, int num);


	void LinkCells();	

	D3DXVECTOR3 SnapPointToCell(NavigationCell* Cell, const D3DXVECTOR3& Point);
	D3DXVECTOR3 SnapPointToMesh(NavigationCell** CellOut, const D3DXVECTOR3& Point);
	NavigationCell* FindClosestCell(const D3DXVECTOR3& Point)const;

	bool LineOfSightTest(NavigationCell* StartCell, const D3DXVECTOR3& StartPos, NavigationCell* EndCell, const D3DXVECTOR3& EndPos);
	bool BuildNavigationPath(NavigationPath& NavPath, NavigationCell* StartCell, const D3DXVECTOR3& StartPos, NavigationCell* EndCell, const D3DXVECTOR3& EndPos);

	bool ResolveMotionOnMesh(const D3DXVECTOR3& StartPos, NavigationCell* StartCell, D3DXVECTOR3& EndPos, NavigationCell** EndCell);

	void Update(float elapsedTime = 1.0f);
	void Render();

	// ----- ACCESSORS --------------------
	int TotalCells()const;
	NavigationCell* Cell(int index);

	bool Save( BStream* pStream );
	bool Load( BStream* pStream );

	int GetCellIndex( NavigationCell* pCell );


private:

	// ----- DATA -------------------------
	int m_nCellNum;
	NavigationCell** m_ppCells;

	// path finding data...
	int m_PathSession;
	NavigationHeap m_NavHeap;

	// ----- HELPER FUNCTIONS -------------

	// ----- UNIMPLEMENTED FUNCTIONS ------

	NavigationMesh( const NavigationMesh& Src);
	NavigationMesh& operator=( const NavigationMesh& Src);

};

//- Inline Functions ---------------------------------------------------------------------

//= CREATORS =============================================================================

/*	NavigationMesh
------------------------------------------------------------------------------------------
	
	Default Object Constructor
	
------------------------------------------------------------------------------------------
*/
inline NavigationMesh::NavigationMesh()
:m_PathSession(0)
{
	m_nCellNum = 0;
	m_ppCells = NULL;
}





/*	~NavigationMesh
------------------------------------------------------------------------------------------
	
	Default Object Destructor
	
------------------------------------------------------------------------------------------
*/
inline NavigationMesh::~NavigationMesh()
{
	Clear();
}

//= OPERATORS ============================================================================

//= MUTATORS =============================================================================

//:	Clear
//----------------------------------------------------------------------------------------
//
//	Delete all cells associated with this mesh 
//
//-------------------------------------------------------------------------------------://
inline void NavigationMesh::Clear()
{
/*
	CELL_ARRAY::iterator	CellIter = m_CellArray.begin();
	for(;CellIter != m_CellArray.end(); ++CellIter)
	{
		NavigationCell* Cell = *CellIter;
		delete Cell;
	}
	m_CellArray.clear();
*/
//	m_nCellNum = 0;
//	m_ppCells = NULL;
	for( int i=0; i<m_nCellNum; i++ )
	{
		if( m_ppCells[i] )
			delete m_ppCells[i];
	}
	delete[] m_ppCells;
	m_ppCells = NULL;
	m_nCellNum = 0;
}


//:	AddCell
//----------------------------------------------------------------------------------------
//
//	Add a new cell, defined by the three vertices in clockwise order, to this mesh 
//
//-------------------------------------------------------------------------------------://
inline int NavigationMesh::AddCell(const D3DXVECTOR3& PointA, const D3DXVECTOR3& PointB, const D3DXVECTOR3& PointC, int nIndex )
{
	NavigationCell* NewCell = new NavigationCell;
	NewCell->Initialize(PointA, PointB, PointC);
	m_ppCells[nIndex] = NewCell;
	NewCell->m_nIndex = nIndex;
	return nIndex;
}

inline int NavigationMesh::AddCell(NavigationCell* Cell, int nIndex)
{
	m_ppCells[nIndex] = Cell;
	return nIndex;
}

//:	RemoveCell
//----------------------------------------------------------------------------------------
//
//	Remoe a new cell, sever links
//
//-------------------------------------------------------------------------------------://
// 안씀
inline void NavigationMesh::RemoveCell(int index, int num)
{
	BsAssert( 0 );
/*
	for( int i=0; i<num; i++ )
	{
		NavigationCell* Cell = m_CellArray[index+i];
		if ( Cell->Link(NavigationCell::SIDE_AB) ) 
			Cell->Link(NavigationCell::SIDE_AB)->UnlinkCell( Cell );
		if ( Cell->Link(NavigationCell::SIDE_BC) ) 
			Cell->Link(NavigationCell::SIDE_BC)->UnlinkCell( Cell );
		if ( Cell->Link(NavigationCell::SIDE_CA) ) 
			Cell->Link(NavigationCell::SIDE_CA)->UnlinkCell( Cell );

		delete Cell;
	}
	m_CellArray.erase( m_CellArray.begin() + index, m_CellArray.begin() + index + num - 1 );
*/
}

inline void NavigationMesh::SetUseCell( std::vector<NavigationCell*>& CellArray, int index, int num )
{
	for ( int i=0; i<num; i++ ) {
		NavigationCell* pSrcCell = CellArray[i];
		NavigationCell* cell = Cell( index + i );

//		*Cell( index + i ) = *pSrcCell;		// Cell 대입해서 값 받으면 안됨 ( = 연산자 참조)

//		cell->SetLinkCell( NavigationCell* Cell, int nIndex )


		if (cell->Link(0))
			cell->Link(0)->RequestLink(cell->Vertex(0), cell->Vertex(1), cell);

		if (cell->Link(1))
			cell->Link(1)->RequestLink(cell->Vertex(1), cell->Vertex(2), cell);

		if (cell->Link(2))
			cell->Link(2)->RequestLink(cell->Vertex(2), cell->Vertex(0), cell);

		cell->SetUse( true );
	}
}


inline void NavigationMesh::SetUnuseCell(int index, int num)
{
	for( int i=0; i<num; i++ )
	{
		NavigationCell* Cell = m_ppCells[index+i];
		if ( Cell->Link(NavigationCell::SIDE_AB) ) 
			Cell->Link(NavigationCell::SIDE_AB)->UnlinkCell( Cell );
		if ( Cell->Link(NavigationCell::SIDE_BC) ) 
			Cell->Link(NavigationCell::SIDE_BC)->UnlinkCell( Cell );
		if ( Cell->Link(NavigationCell::SIDE_CA) ) 
			Cell->Link(NavigationCell::SIDE_CA)->UnlinkCell( Cell );

		Cell->SetUse( false );
	}
}

//:	Render
//----------------------------------------------------------------------------------------
//
//	Render the mesh geometry to screen. The mesh is assumed to exist in world corrdinates 
//	for the purpose of this demo 
//
//-------------------------------------------------------------------------------------://
inline void NavigationMesh::Render()
{
/*    glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);

	glBegin(GL_TRIANGLES);

	// render each cell triangle
	CELL_ARRAY::const_iterator	iter = m_CellArray.begin();
	for(;iter != m_CellArray.end(); ++iter)
	{
		const NavigationCell* Cell = *iter;
		int i;

		for (i=0;i<3;++i)
		{
			char shade = (char)Cell->Vertex(i).y;
			shade = 128+shade;

			glColor4ub(shade, shade, shade, 0xff);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(Cell->Vertex(i).x, Cell->Vertex(i).y, Cell->Vertex(i).z);
		}
	}

	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);

	// render cell edges as wireframe for added visibility
	iter = m_CellArray.begin();
	for(;iter != m_CellArray.end(); ++iter)
	{
		const NavigationCell* Cell = *iter;
		int i;

		for (i=0;i<3;++i)
		{
			char shade = (char)Cell->Vertex(i).y;
			shade = 64+shade;

			glColor4ub(shade, shade, shade, 0x80);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(Cell->Vertex(i).x, Cell->Vertex(i).y + 0.2, Cell->Vertex(i).z);
		}
	}

	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   
	glPopMatrix();*/
}

//:	Update
//----------------------------------------------------------------------------------------
//
//	Does noting at this point. Stubbed for future use in animating the mesh 
//
//-------------------------------------------------------------------------------------://
inline void NavigationMesh::Update(float elapsedTime)
{
}


//= ACCESSORS ============================================================================
inline int NavigationMesh::TotalCells()const
{
	return m_nCellNum;
}

inline NavigationCell* NavigationMesh::Cell(int index)
{
	return m_ppCells[index];
}


//- End of NavigationMesh ----------------------------------------------------------------

//****************************************************************************************

#endif  // end of file      ( NavigationMesh.h )

