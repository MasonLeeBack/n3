#ifndef	__RANGESCAN_H__
#define	__RANGESCAN_H__

/*
	RangeScan.h
	Find objects within given circle(Center, Radius) or rect(Left, Top, Right, Bottom)

	Created 2001/05, by JiYoung
*/

#include <vector>
#include <assert.h>

/*	Name: RangeScan
	Description: Find object within given search circle

	Note: RangeScan stores objects ID, which are in 2D space (R, R).
		When a search circle (Center, Radius) is given by Find() method, 
		it will return list of objects lies within the square containing the circle.

		To run properly, class INTF must have following form

			struct INTF {
				typedef [object type] ID;
				typedef [coord type]  R;

				static R GetX( ID obj );
				static R GetY( ID obj );
				static bool Valid( ID Obj );
			};

		Template parameter CELLWIDTH, CELLHEIGHT determines size of a search cell.
		Template parameter LEAF_MAXOBJ, NONLEAF_MINOBJ determines number of
		objects in a search node. Change them for performance optimization.

		* This template approximates search by bounding-rectangles, for performance reason

	Advanced Note: A search map is consistes of search cell. Each search cell is a
		square, of size (CELLWIDTH, CELLHEIGHT). Each search cell is a QuadTree.
		Each QT-node will actually contain objects. QT-Node(the Node) will be
		divided into smaller one if number of objects in the node exceeds LEAF_MAXOBJ.
		And 4 QT-Nodes will be merged into one if object count of all 4 child nodes 
		goes below NONLEAF_MINOBJ.
*/

template< class INTF, int NONLEAF_MINOBJ = 4, int LEAF_MAXOBJ = 40, bool EXTENDED_DEBUG = false >
class RangeScan 
{
public:

	typedef typename INTF::ID ID;
	typedef typename INTF::R  R;

	//====================================================================
	//===     RangeScan function body starts from here
	//====================================================================

	/*	Name: RangeScan()
		Description: Constructor
		Parameters: -none-
		Return value: -none-
	*/
	RangeScan() {
		m_rWidth=m_rHeight=0;
		m_rCellWidth=m_rCellHeight=0;
		m_nCellHorizontalCount=m_nCellVerticalCount=0;
		m_pNode=NULL;
		m_nObjectCount=0;
	}

	/*	Name: ~RangeScan()
		Description: Destructor
		Parameters: -none-
		Return value: -none-
	*/
	~RangeScan() {
		Close();
	}

	/*	Name: Initialize()
		Description: Initialize search space
		Parameters: rWidth - Width of whole map. 0 <= X < rWidth
			rHeight - Height of whole map. 0 <= Y < rHeight
			rCellWidth - Width of a cell
			rCellHeight - Height of a cell
			rMinNodeWidth - Minimin width of a terminal node
		Return value: TRUE of successful
		Note: The map is consisted by cells.
			Cell is a square that actual search activity will be performed
	*/
	bool Initialize(R rWidth, R rHeight, R rCellWidth, R rCellHeight, R rMinNodeWidth = 0) {

		// Clear previous data
		Close();

		// Set basic parameters
		m_rWidth=rWidth;
		m_rHeight=rHeight;
		m_rCellWidth=rCellWidth;
		m_rCellHeight=rCellHeight;
		m_nCellHorizontalCount=(int)(rWidth/rCellWidth)+1;
		m_nCellVerticalCount=(int)(rHeight/rCellHeight)+1;

		if ( rMinNodeWidth == 0 ) 
			rMinNodeWidth = rCellWidth / 1024;

		// Allocate search nodes
		int nNodeCount;
		nNodeCount=m_nCellHorizontalCount*m_nCellVerticalCount;
		m_pNode=new QuadNode[nNodeCount];
		if (m_pNode==NULL) return false;

		int i,j,k;

		// Initialize search nodes
		k=0;
		for (i=0; i<m_nCellVerticalCount; i++) {
			for (j=0; j<m_nCellHorizontalCount; j++) {
				m_pNode[k].rLeft=rCellWidth*j;
				m_pNode[k].rRight=rCellWidth*(j+1);
				m_pNode[k].rTop=rCellHeight*i;
				m_pNode[k].rBottom=rCellHeight*(i+1);
				m_pNode[k].rMinNodeWidth = rMinNodeWidth;
				k++;
			}
		}

		return true;
	}

	/*	Name: Close()
		Description: Close initialized data
		Parameters: -none-
		Return value: TRUE if successful
	*/
	bool Close() {
		// Delete allocated search cells
		if (m_pNode!=NULL) {
			delete [] m_pNode;
			m_pNode=NULL;
		}
		// Clear other variables
		m_rWidth=m_rHeight=0;
		m_rCellWidth=m_rCellHeight=0;
		m_nCellHorizontalCount=m_nCellVerticalCount=0;
		m_nObjectCount=0;
		return true;
	}

	/*	Name: GetMapWidth() / GetMapHeight()
		Description: Get map size
		Parameters: -none-
		Return value: corresponding information.
	*/
	R GetMapWidth() {
		return m_rWidth;
	}

	R GetMapHeight() {
		return m_rHeight;
	}

	/*	Name: AddObject()
		Description: Add a new object to search list
		Parameters: obj - The object to be added
		Return value: true if successful
		Note: {INTF::GetX(obj), INTF::GetY(obj)} will be used as a KEY to the search list
	*/
	bool AddObject(ID obj) {
		ASSERT(m_pNode!=NULL);
		if( !INTF::Valid(obj) ) return false;
		R rX=INTF::GetX(obj), rY=INTF::GetY(obj);
		return m_pNode[GetCellIndex(rX, rY)].AddObject(obj, rX, rY);
	}

	/*	Name: MoveObject()
		Description: Move a existing object to new position
		Parameters: rOldX - old X position
			rOldY - old Y position
			obj - An object which remains at old position
	*/
	bool MoveObject(R rOldX, R rOldY, ID obj) {
		ASSERT(m_pNode!=NULL);
		if( !INTF::Valid(obj) ) return false;
		R rX=INTF::GetX(obj), rY=INTF::GetY(obj);
		int nOldCell, nNewCell;
		nOldCell=GetCellIndex(rOldX, rOldY);
		nNewCell=GetCellIndex(rX, rY);
		if (nOldCell==nNewCell) {
			return m_pNode[nOldCell].MoveObject(rOldX, rOldY, obj, rX, rY);
		}
		if( m_pNode[nOldCell].RemoveObject(obj, rOldX, rOldY) == false )
			return false;
		return m_pNode[nNewCell].AddObject(obj, rX, rY);
	}

	/*	Name: RemoveObject()
		Description: Remove an object from search list
		Parameters: obj - The object to be removed
		Return value: true if successful
	*/
	bool RemoveObject(ID obj) {
		ASSERT(m_pNode!=NULL);
		if( !INTF::Valid(obj) ) return false;
		R rX=INTF::GetX(obj), rY=INTF::GetY(obj);
		return m_pNode[GetCellIndex(rX, rY)].RemoveObject(obj, rX, rY);
	}

	/*	Name: RemoveObject()
		Description: Remove an object at (rOldX, rOldY) from search list
		Parameters: rOldX - old X position
		    rOldY - old Y position
		    obj - The object to be removed
		Return value: true if successful
	*/
	bool RemoveObject(R rOldX, R rOldY, ID obj) {
		ASSERT(m_pNode!=NULL);
		return m_pNode[GetCellIndex(rOldX, rOldY)].RemoveObject(obj, rOldX, rOldY);
	}

	/*	Name: Find()
		Description: Find object within certain search range
		Parameters: 
			STACK_OF_ID					 - A stack of ID type. Must provide push_back( ID id ) method
			rCenterX, rCenterY, rRadius  - Center and radius of search circle
			rLeft, rTop, rRight, rBottom - Boundary of search rectangle
			stackResult					 - Found objects will be stored here using push_back() method call.
		Return value: Number of object found. -1 if error.
	*/
	template< class STACK_OF_ID >
	int Find(R rCenterX, R rCenterY, R rRadius, STACK_OF_ID& stackResult) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rCenterX=rCenterX; fs.rCenterY=rCenterY; fs.rRadius=rRadius;
		fs.Calculate();
		return DoFind( fs, &stackResult );
	}

	template< class STACK_OF_ID >
	int Find(R rLeft, R rTop, R rRight, R rBottom, STACK_OF_ID& stackResult) {	
		// Build findstruct
		FINDSTRUCT fs;
		fs.rLeft=rLeft; fs.rTop=rTop; fs.rRight=rRight; fs.rBottom=rBottom;
		return DoFind( fs, &stackResult );
	}

	/*	Name: Count()
		Description: Return object count within certain search range
		Parameters: 
			rCenterX, rCenterY, rRadius  - Center and radius of search circle
			rLeft, rTop, rRight, rBottom - Boundary of search rectangle
		Return value: Number of object found. -1 if error.
	*/
	int Count(R rCenterX, R rCenterY, R rRadius) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rCenterX=rCenterX; fs.rCenterY=rCenterY; fs.rRadius=rRadius;
		fs.Calculate();
		return DoFind( fs, (std::vector<INTF::ID>*)NULL );
	}

	int Count(R rLeft, R rTop, R rRight, R rBottom) {	
		// Build findstruct
		FINDSTRUCT fs;
		fs.rLeft=rLeft; fs.rTop=rTop; fs.rRight=rRight; fs.rBottom=rBottom;
		return DoFind( fs, (std::vector<INTF::ID>*)NULL );
	}

	/*	Name: FindFor()
		Description: Find objects for fulfilling given constraint, within given search range.
		Parameters: 
			STACK_OF_ID					 - A stack of ID type. Must provide push_back( ID id ) method
			rCenterX, rCenterY, rRadius  - Center and radius of search circle
			rLeft, rTop, rRight, rBottom - Boundary of search rectangle
			fnConstraint				 - function pointer or STL function object conforming to the following form:
												=======>  bool fnConstraint( const ID& obj ); 
			stackResult					 - Found objects will be stored here using push_back() method call.
		Return value: Number of object found. -1 if error.
	*/
	template< class FUNC, class STACK_OF_ID >
	int ExcuteFor(R rCenterX, R rCenterY, R rRadius, FUNC fnConstraint, STACK_OF_ID *pResult) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rCenterX=rCenterX; fs.rCenterY=rCenterY; fs.rRadius=rRadius;
		fs.Calculate();
		return DoFindFor( fs, fnConstraint, pResult );
	}
/*	template< class FUNC, class STACK_OF_ID >
	int ExcuteFor(R rLeft, R rTop, R rRight, R rBottom, FUNC fnConstraint) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rLeft=rLeft; fs.rTop=rTop; fs.rRight=rRight; fs.rBottom=rBottom;
		return DoFindFor( fs, fnConstraint, NULL );
	}*/

	template< class FUNC, class STACK_OF_ID >
	int FindFor(R rCenterX, R rCenterY, R rRadius, FUNC fnConstraint, STACK_OF_ID& stackResult) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rCenterX=rCenterX; fs.rCenterY=rCenterY; fs.rRadius=rRadius;
		fs.Calculate();
		return DoFindFor( fs, fnConstraint, &stackResult );
	}

	template< class FUNC, class STACK_OF_ID >
	int FindFor(R rLeft, R rTop, R rRight, R rBottom, FUNC fnConstraint, STACK_OF_ID& stackResult) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rLeft=rLeft; fs.rTop=rTop; fs.rRight=rRight; fs.rBottom=rBottom;
		return DoFindFor( fs, fnConstraint, &stackResult );
	}

	/*	Name: CountFor()
		Description: Count objects for fulfilling given constraint, within certain search range
		Parameters: 
			rCenterX, rCenterY, rRadius  - Center and radius of search circle
			rLeft, rTop, rRight, rBottom - Boundary of search rectangle
			fnConstraint				 - function pointer or STL function object conforming to the following form:
												=======>  bool fnConstraint( const ID& obj ); 
		Return value: Number of object found. -1 if error.
	*/
	template< class FUNC >
	int CountFor(R rCenterX, R rCenterY, R rRadius, FUNC fnConstraint) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rCenterX=rCenterX; fs.rCenterY=rCenterY; fs.rRadius=rRadius;
		fs.Calculate();
		return DoFindFor( fs, fnConstraint, (std::vector<INTF::ID>*)NULL );
	}

	template< class FUNC >
	int CountFor(R rLeft, R rTop, R rRight, R rBottom, FUNC fnConstraint) {
		// Build findstruct
		FINDSTRUCT fs;
		fs.rLeft=rLeft; fs.rTop=rTop; fs.rRight=rRight; fs.rBottom=rBottom;
		return DoFindFor( fs, fnConstraint, (std::vector<INTF::ID>*)NULL );
	}

	/*	Name: CalcMemory()
		Description: Calculate total memory consumption of this instance of RangeScan
		Parameters: 
		Return value: Occupying memory in bytes
	*/

	int CalcMemory()
	{
		int nNodeCount = m_nCellHorizontalCount * m_nCellVerticalCount;
		int nSum = sizeof( this ) + sizeof( QuadNode ) * nNodeCount;

		for ( int i = 0; i < nNodeCount; i++ ) 
			nSum += m_pNode[ i ].CalcMemory();

		return nSum;
	}

private:
	
	//-------- Initernal data structures
	struct FINDSTRUCT {
		// This is input
		R rCenterX, rCenterY, rRadius;

		// This is Calculate's output
		R rLeft, rTop, rRight, rBottom;	// Bounding Rect
		R rCorner;	// Length of corner

		//          .N._____
		//          N /  |
		//          ./   R
		//           |   |
		//           |-R-+ Center
		//           |
		// N = Distance from corner of bounding rect to bounding octagon == rCorner

		void Calculate() {
			// Bounding Rect
			rLeft=rCenterX-rRadius;
			rRight=rCenterX+rRadius;
			rTop=rCenterY-rRadius;
			rBottom=rCenterY+rRadius;

			rCorner=(rRadius*(R)58)/(R)100;		// < 1-tan(PI/8)
		}
	};

	struct QuadNode {

		//------ Member variables
		// Dimension of node
		R rLeft, rTop, rRight, rBottom;
			// Left <= R < Right, Top <= R < Bottom

		// Minimum width of terminal node 
		R rMinNodeWidth;

		// TRUE when this node is terminal. i.e. No child
		bool m_bIsTerminal;

		// Number of objects, including all child nodes
		int nObjectCount;	

		// Objects in this node. if m_bIsTerminal==false, m_Object should be empty
		std::vector<ID> m_Object;

		// Pointer to child nodes
		QuadNode *pNode[4];
		enum {
			  LU=0, RU, LD, RD	// Left-Up, Right-Up, Left-Down, Right-Down
		};

		//------- Member functions
		QuadNode() {
			int i;
			rLeft=rTop=rRight=rBottom=0;
			rMinNodeWidth=0;
			nObjectCount=0;
			m_bIsTerminal=true;
			for (i=0; i<4; i++)
				pNode[i]=NULL;
		}
		~QuadNode() {
			int i;
			for (i=0; i<4; i++)
				if (pNode[i]!=NULL) delete pNode[i];
		}

		int GetObjectCount() { return nObjectCount; }

		bool AddObject(ID obj, R rX, R rY) {
			if (m_bIsTerminal) {
				// For terminal node, just add to THIS
				m_Object.push_back(obj);
				nObjectCount++;
				if (nObjectCount>LEAF_MAXOBJ)
					DivideNode();
				return true;
			}
			// Add to child node
			QuadNode *pChild;
			bool bReturn;
			pChild=ChildNodeAt(rX,rY);
			ASSERT(pChild!=NULL);
			bReturn=pChild->AddObject(obj,rX,rY);
			if (bReturn) nObjectCount++;
			return bReturn;
		}

		bool RemoveObject(ID obj, R rX, R rY) {
			// For terminal node, check self's objects
			if (m_bIsTerminal) {
				std::vector<ID>::iterator it;
				for (it=m_Object.begin(); it!=m_Object.end(); it++) {
					if (obj==*it) {
						// object found: remove it
						m_Object.erase(it);
						nObjectCount--;
						return true;
					}
				}
				// Reach here when no object found
				return false;
			}
			// Check child objects
			QuadNode *pChild;
			bool bReturn;
			pChild=ChildNodeAt(rX, rY);
			ASSERT(pChild!=NULL);
			bReturn=pChild->RemoveObject(obj, rX, rY);
			if (bReturn) {
				if (--nObjectCount<NONLEAF_MINOBJ)
					MergeNode();
			}
			return bReturn;
		}

		bool MoveObject(R rOldX, R rOldY, ID obj, R rNewX, R rNewY) {
			if (m_bIsTerminal) {
				// For debug mode, check obj's validity when extended debug flag is set
				ASSERT( EXTENDED_DEBUG ? CheckIntegrity(obj) : true );
				return true;
			} // else

			// Check movement occurs in same child node
			QuadNode *pChildOld, *pChildNew;
			pChildOld=ChildNodeAt(rOldX, rOldY);
			pChildNew=ChildNodeAt(rNewX, rNewY);
			if (pChildOld==pChildNew) {
				// In same node: pass it down
				return pChildOld->MoveObject(rOldX, rOldY, obj, rNewX, rNewY);
			}
			if (pChildOld->RemoveObject(obj, rOldX, rOldY)==false) {
				ASSERT(0);
				return false;
			}
			return pChildNew->AddObject(obj, rNewX, rNewY);
		}

		template< class STACK_OF_ID >
		int Dump(STACK_OF_ID* pStackResult) {
			if (m_bIsTerminal) {
				std::vector<ID>::iterator it;
				for (it=m_Object.begin(); it!=m_Object.end(); it++) {
					pStackResult->push_back(*it);
				}
			} else {
				pNode[LU]->Dump(pStackResult);
				pNode[RU]->Dump(pStackResult);
				pNode[LD]->Dump(pStackResult);
				pNode[RD]->Dump(pStackResult);
			}
			return nObjectCount;
		}

		template< class FUNC, class STACK_OF_ID >
		int DumpFor(FUNC fnConstraint, STACK_OF_ID* pStackResult) {
			int nObjCount = 0;
			if (m_bIsTerminal) {
				std::vector<ID>::iterator it;
				if (pStackResult == NULL) {
					for (it=m_Object.begin(); it!=m_Object.end(); it++) {
						if (fnConstraint(*it)) 
							nObjCount++;
					}
				} else {
					for (it=m_Object.begin(); it!=m_Object.end(); it++) 
						if (fnConstraint(*it)) {
							pStackResult->push_back(*it);
							nObjCount++;
						}
				}
			} else {
				nObjCount += pNode[LU]->DumpFor(fnConstraint, pStackResult);
				nObjCount += pNode[RU]->DumpFor(fnConstraint, pStackResult);
				nObjCount += pNode[LD]->DumpFor(fnConstraint, pStackResult);
				nObjCount += pNode[RD]->DumpFor(fnConstraint, pStackResult);
			}
			return nObjectCount;
		}

		template< class STACK_OF_ID >
		int Find(const FINDSTRUCT& fs, STACK_OF_ID* pStackResult) {
			int nFoundCount=0;

			if (m_bIsTerminal) {
				std::vector<ID>::iterator it;
				// Check relationship of node and fs
				if ((fs.rLeft <= rLeft)
						&&(fs.rRight >= rRight)
						&&(fs.rTop <= rTop)
						&&(fs.rBottom >= rBottom)) {
					// A cell is completely included in fs, so add all object
					if (pStackResult==NULL) {
						// No return array: count only
						nFoundCount=m_Object.size();
					} else {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							pStackResult->push_back(*it);
							nFoundCount++;
						}
					}
				} else {
					// Enum objects one by one
					R rX, rY;
					if (pStackResult!=NULL) {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if( !INTF::Valid(*it) ) continue;
							rX=INTF::GetX(*it), rY=INTF::GetY(*it);
							if ( (rX>=fs.rLeft)&&(rX<fs.rRight)&&(rY>=fs.rTop)&&(rY<fs.rBottom)) {
								// Add object to return array
								pStackResult->push_back(*it);
								nFoundCount++;
							}
						}
					} else {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if( !INTF::Valid(*it) ) continue;
							rX=INTF::GetX(*it), rY=INTF::GetY(*it);
							if ( (rX>=fs.rLeft)&&(rX<fs.rRight)&&(rY>=fs.rTop)&&(rY<fs.rRight)) {
								// Count number of object in fs
								nFoundCount++;
							}
						}
					}
				}
				return nFoundCount;
			} // else

			// Pass search to child nodes

			// Check if this node is fully contained in FS
			if ((fs.rLeft <= rLeft)
				&&(fs.rRight >= rRight)
				&&(fs.rTop <= rTop)
				&&(fs.rBottom >= rBottom)) {
				if (pStackResult==NULL) return nObjectCount;
				return Dump(pStackResult);
			}

			// Check child node's status
			if (pNode[0]->rRight < fs.rLeft) {
				// FS lies on right-side
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on RD node only
					nFoundCount+=pNode[RD]->Find(fs, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on RU node only
					nFoundCount+=pNode[RU]->Find(fs, pStackResult);
				} else {
					// FS lies on RD & RU nodes
					nFoundCount+=pNode[RU]->Find(fs, pStackResult);
					nFoundCount+=pNode[RD]->Find(fs, pStackResult);
				}
			} else if (pNode[0]->rRight >= fs.rRight) {
				// FS lies on left-side
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on LD node only
					nFoundCount+=pNode[LD]->Find(fs, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on LU node only
					nFoundCount+=pNode[LU]->Find(fs, pStackResult);
				} else {
					// FS lies on LD & LU nodes
					nFoundCount+=pNode[LU]->Find(fs, pStackResult);
					nFoundCount+=pNode[LD]->Find(fs, pStackResult);
				}
			} else {
				// FS lies on both left-and-right
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on LD/RD nodes
					nFoundCount+=pNode[LD]->Find(fs, pStackResult);
					nFoundCount+=pNode[RD]->Find(fs, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on LU/RU nodes
					nFoundCount+=pNode[LU]->Find(fs, pStackResult);
					nFoundCount+=pNode[RU]->Find(fs, pStackResult);
				} else {
					// FS lies on all 4 nodes
					nFoundCount+=pNode[LU]->Find(fs, pStackResult);
					nFoundCount+=pNode[RU]->Find(fs, pStackResult);
					nFoundCount+=pNode[LD]->Find(fs, pStackResult);
					nFoundCount+=pNode[RD]->Find(fs, pStackResult);
				}
			}

			/*
			// Note: this works same as above expansion, but much more slower
			int i;
			for (i=0; i<4; i++) {
				// Check relationship of child node and fs
				if (! ((pNode[i]->rLeft>=fs.rRight)
						&&(pNode[i]->rRight<=fs.rLeft)
						&&(pNode[i]->rTop>=fs.rBottom)
						&&(pNode[i]->rBottom<=fs.rTop)) ) {
					nFoundCount+=pNode[i]->Find(fs, pStackResult);
				}
			}
			*/

			return nFoundCount;
		}

		template< class FUNC, class STACK_OF_ID >
		int FindFor(const FINDSTRUCT& fs, FUNC fnConstraint, STACK_OF_ID* pStackResult) {
			int nFoundCount=0;

			if (m_bIsTerminal) {
				std::vector<ID>::iterator it;
				// Check relationship of node and fs
				if ((fs.rLeft <= rLeft)
						&&(fs.rRight >= rRight)
						&&(fs.rTop <= rTop)
						&&(fs.rBottom >= rBottom)) {
					// A cell is completely included in fs, so add all object
					if (pStackResult==NULL) {
						// No return array: count only
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if (fnConstraint(*it))
								nFoundCount++;
						}
					} else {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if (fnConstraint(*it)) {
								pStackResult->push_back(*it);
								nFoundCount++;
							}
						}
					}
				} else {
					// Enum objects one by one
					R rX, rY;
					if (pStackResult!=NULL) {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if( !INTF::Valid(*it) ) continue;
							rX=INTF::GetX(*it), rY=INTF::GetY(*it);
							if ( fnConstraint(*it)
									&&(rX>=fs.rLeft)&&(rX<fs.rRight)&&(rY>=fs.rTop)&&(rY<fs.rBottom) ) {
								// Add object to return array
								pStackResult->push_back(*it);
								nFoundCount++;
							}
						}
					} else {
						for (it=m_Object.begin(); it!=m_Object.end(); it++) {
							if( !INTF::Valid(*it) ) continue;
							rX=INTF::GetX(*it), rY=INTF::GetY(*it);
							if ( fnConstraint(*it)
									&&(rX>=fs.rLeft)&&(rX<fs.rRight)&&(rY>=fs.rTop)&&(rY<fs.rBottom) ) {
								// Count number of object in fs
								nFoundCount++;
							}
						}
					}
				}
				return nFoundCount;
			} // else

			// Pass search to child nodes

			// Check if this node is fully contained in FS
			if ((fs.rLeft <= rLeft)
				&&(fs.rRight >= rRight)
				&&(fs.rTop <= rTop)
				&&(fs.rBottom >= rBottom)) {
				return DumpFor(fnConstraint, pStackResult);
			}

			// Check child node's status
			if (pNode[0]->rRight < fs.rLeft) {
				// FS lies on right-side
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on RD node only
					nFoundCount+=pNode[RD]->FindFor(fs, fnConstraint, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on RU node only
					nFoundCount+=pNode[RU]->FindFor(fs, fnConstraint, pStackResult);
				} else {
					// FS lies on RD & RU nodes
					nFoundCount+=pNode[RU]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[RD]->FindFor(fs, fnConstraint, pStackResult);
				}
			} else if (pNode[0]->rRight >= fs.rRight) {
				// FS lies on left-side
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on LD node only
					nFoundCount+=pNode[LD]->FindFor(fs, fnConstraint, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on LU node only
					nFoundCount+=pNode[LU]->FindFor(fs, fnConstraint, pStackResult);
				} else {
					// FS lies on LD & LU nodes
					nFoundCount+=pNode[LU]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[LD]->FindFor(fs, fnConstraint, pStackResult);
				}
			} else {
				// FS lies on both left-and-right
				if (pNode[0]->rBottom < fs.rTop) {
					// FS lies on LD/RD nodes
					nFoundCount+=pNode[LD]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[RD]->FindFor(fs, fnConstraint, pStackResult);
				} else if (pNode[0]->rBottom >= fs.rBottom) {
					// FS lies on LU/RU nodes
					nFoundCount+=pNode[LU]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[RU]->FindFor(fs, fnConstraint, pStackResult);
				} else {
					// FS lies on all 4 nodes
					nFoundCount+=pNode[LU]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[RU]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[LD]->FindFor(fs, fnConstraint, pStackResult);
					nFoundCount+=pNode[RD]->FindFor(fs, fnConstraint, pStackResult);
				}
			}

			/*
			// Note: this works same as above expansion, but much more slower
			int i;
			for (i=0; i<4; i++) {
				// Check relationship of child node and fs
				if (! ((pNode[i]->rLeft>=fs.rRight)
						&&(pNode[i]->rRight<=fs.rLeft)
						&&(pNode[i]->rTop>=fs.rBottom)
						&&(pNode[i]->rBottom<=fs.rTop)) ) {
					nFoundCount+=pNode[i]->FindFor(fs, fnConstraint, pStackResult);
				}
			}
			*/

			return nFoundCount;
		}

		bool CheckIntegrity(ID obj) {
			std::vector<ID>::iterator it;
			for (it=m_Object.begin(); it!=m_Object.end(); it++) {
				if (obj==*it) {
					// object found
					return true;
				}
			}
			// Reach here when no object found
			return false;
		}

		int CalcMemory() {
			int nSum = sizeof( this ) + sizeof(ID) * m_Object.capacity();
			for ( int i = 0; i < 4; i++ ) 
				if ( pNode[ i ] != NULL )
					nSum += pNode[ i ]->CalcMemory();
			return nSum;
		}

	private:
		QuadNode *ChildNodeAt(R rX, R rY) {
			int nIndex;
			nIndex=((rX>=(rLeft+rRight)/(R)2)?1:0)
				+((rY>=(rTop+rBottom)/(R)2)?2:0);
			return pNode[nIndex];
		}

		bool DivideNode() {
			if (!m_bIsTerminal) return false;					// Can't divide non-leaf node
			if (rRight-rLeft < rMinNodeWidth) return false;		// to prevent stack overflow

			int i;

			// Create child nodes
			for (i=0; i<4; i++) {
				pNode[i]=new QuadNode;
				pNode[i]->rMinNodeWidth = rMinNodeWidth;
				ASSERT(pNode[i]!=NULL);
			}
			// Assign node dimension
			R rMidX, rMidY;
			rMidX=(rLeft+rRight)/(R)2;
			rMidY=(rTop+rBottom)/(R)2;
			pNode[LU]->rLeft=pNode[LD]->rLeft=rLeft;
			pNode[LU]->rTop=pNode[RU]->rTop=rTop;
			pNode[RU]->rRight=pNode[RD]->rRight=rRight;
			pNode[LD]->rBottom=pNode[RD]->rBottom=rBottom;
			pNode[LU]->rRight=pNode[RU]->rLeft
				=pNode[LD]->rRight=pNode[RD]->rLeft=rMidX;
			pNode[LU]->rBottom=pNode[RU]->rBottom
				=pNode[LD]->rTop=pNode[RD]->rTop=rMidY;

			// Move objects to child
			std::vector<ID>::iterator it;
			for (it=m_Object.begin(); it!=m_Object.end(); it++) {
				if( !INTF::Valid(*it) ) continue;
				R rX=INTF::GetX(*it), rY=INTF::GetY(*it);
				ChildNodeAt(rX, rY)->AddObject(*it, rX, rY);
			}

			// clear this's object list
			m_Object.clear();
			m_bIsTerminal=false;		// Mark this is non-terminal

			return true;
		}

		bool MergeNode() {
			int i;

			if (m_bIsTerminal) return false;		// can't merge leaf node

			// Check 'mergability'; all leafs must be terminal
			for (i=0; i<4; i++) {
				if (pNode[i]->m_bIsTerminal==false)
					return false;
			}

			// Merge nodes
			std::vector<ID>::iterator itchild;
			for (i=0; i<4; i++) {
				for (itchild=pNode[i]->m_Object.begin()
						; itchild!=pNode[i]->m_Object.end()
						; itchild++) {
					m_Object.push_back(*itchild);
				}
				delete pNode[i];
				pNode[i]=NULL;
			}

			// Mark this is terminal
			m_bIsTerminal=true;

			return true;
		}
	};

	//-- Utilities

	template< class STACK_OF_ID >
	int DoFind(const FINDSTRUCT& fs, STACK_OF_ID* pStackResult) {
		
		int nL, nR, nT, nB;	// Left, Right, Top, Bottom
		int j,i;
		int nCellNumber;
		int nFoundCount;

		// Determine target areas
		nL=(int)(fs.rLeft/m_rCellWidth);
		nT=(int)(fs.rTop/m_rCellHeight);
		nR=(int)(fs.rRight/m_rCellWidth);
		nB=(int)(fs.rBottom/m_rCellHeight);
		if (nL<0) nL=0;
		if (nR>=m_nCellHorizontalCount) nR=m_nCellHorizontalCount-1;
		if (nT<0) nT=0;
		if (nB>=m_nCellVerticalCount) nB=m_nCellVerticalCount-1;

		// Do find
		nFoundCount=0;
		for (i=nT; i<=nB; i++) {
			nCellNumber=i*m_nCellHorizontalCount + nL;
			for (j=nL; j<=nR; j++) {
				// Call each cell's find function
				nFoundCount+=m_pNode[nCellNumber].Find(fs, pStackResult);
				// Advance to next cell
				nCellNumber++;
			}
		}
		return nFoundCount;
	}

	template< class FUNC, class STACK_OF_ID >
	int DoFindFor(const FINDSTRUCT& fs, FUNC fnConstraint, STACK_OF_ID* pStackResult) {	

		int nL, nR, nT, nB;	// Left, Right, Top, Bottom
		int j,i;
		int nCellNumber;
		int nFoundCount;

		// Determine target areas
		nL=(int)(fs.rLeft/m_rCellWidth);
		nT=(int)(fs.rTop/m_rCellHeight);
		nR=(int)(fs.rRight/m_rCellWidth);
		nB=(int)(fs.rBottom/m_rCellHeight);
		if (nL<0) nL=0;
		if (nR>=m_nCellHorizontalCount) nR=m_nCellHorizontalCount-1;
		if (nT<0) nT=0;
		if (nB>=m_nCellVerticalCount) nB=m_nCellVerticalCount-1;

		// Do find
		nFoundCount=0;
		for (i=nT; i<=nB; i++) {
			nCellNumber=i*m_nCellHorizontalCount + nL;
			for (j=nL; j<=nR; j++) {
				// Call each cell's find function
				nFoundCount+=m_pNode[nCellNumber].FindFor(fs, fnConstraint, pStackResult);
				// Advance to next cell
				nCellNumber++;
			}
		}
		return nFoundCount;
	}

	int GetCellIndex(R rX, R rY) {
		int nH, nV;
		nH=(int)(rX/m_rCellWidth);
		nV=(int)(rY/m_rCellHeight);
		ASSERT(nH>=0&&nH<m_nCellHorizontalCount&&nV>=0&&nV<m_nCellVerticalCount);
		return (nV*m_nCellHorizontalCount)+nH;
	}

	// Size of whole map
	R m_rWidth, m_rHeight;

	// Size of a cell
	R m_rCellWidth, m_rCellHeight;
	// Number of cells in a row/column
	int m_nCellHorizontalCount, m_nCellVerticalCount;

	// Array of cells
	QuadNode *m_pNode;

	// Total # of object in all cells
	int m_nObjectCount;
};


/*
	RangeScan클래스의 특성에 관한 노트:

	RangeScan클래스는, 격자형 cell로 분할된 맵과, 각 cell에 대한 quadtree를 사용하여 object를 저장한다.
	Find()오퍼레이션시, 주어진 검색영역 FS{(Left, Top)-[Right, Bottom]}에 대해 다음과 같은 순서로 검색 시도한다.

	1. FS와 겹치는 모든 cell을 찾는다.
	2. (1)에서 찾은 각 cell에 대해, FS와 겹치는 Quadtree-node를 찾는다.
	3. (2)에서 찾은 각 node에 대해, FS와 겹치는 object를 linear iteration으로 찾는다.

	따라서 RangeScan클래스는 다음과 같은 경우 효율적이라 생각할 수 있다.

	A. 한 cell에 들어가는 object의 갯수가 충분히 작을 경우: 각 cell이 10개 이하의 object를 가질 때,
		Find()오퍼레이션은 object의 좌표에 대한 직접 mapping처럼 동작한다.
	B. 한 cell에 들어가는 object의 갯수가 충분히 클 경우: 각 cell은 독립된 quadtree이다.
		따라서 cell에 포함된 object의 갯수 n이 증가할 때 검색시간은 O(sqrt(n))으로 천천히 증가한다.

	실제로 cell의 크기와 object의 갯수를 바꾸어가며 검색시간 변화를 실험해 보았을 때,
	A와 B의 요소가 상호 보완하며 전체 검색시간을 일정하게 유지시키는 것으로 보인다.
	본 클래스를 실제 문제에 적용할 때에는 해당하는 문제의 object수 변화 추이를 생각하여
	cell size를 결정하는 것이 좋을 것이다.

	FindFor 메소드의 fnConstraint 는, ID 하나를 인수로 받아 bool 형을 되돌리는 함수 형식이어야 하며,
	한번 수행에 R 형의 크기 비교 4번 보다 적은 시간을 소모할 것으로 간주된다. 따라서 함수 포인터 
	보다는 STL 형식의 함수 객체를 사용하기를 권장한다.
*/

#endif	//__RANGESCAN_H__

