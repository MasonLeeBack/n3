/*
 *	Separated from NavigationPath.h to make compiling easier
 *  - Jiesang Song
 */

/* Copyright (C) Greg Snook, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Greg Snook, 2000"
 */
#ifndef NAVIGATIONWAYPOINT_H
#define NAVIGATIONWAYPOINT_H

class NavigationCell;

// definition of a waypoint
struct WAYPOINT
{
	D3DXVECTOR3 Position;		// 3D position of waypoint	
	NavigationCell* Cell;	// The cell which owns the waypoint
};

typedef std::list<WAYPOINT> WAYPOINT_LIST;
typedef WAYPOINT_LIST::const_iterator WayPointID;
typedef WAYPOINT_LIST::const_iterator NavWayPointID;


#endif