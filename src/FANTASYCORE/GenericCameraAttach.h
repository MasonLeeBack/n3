#pragma		 once

// attach data는 save된 data에 이미 기록된 상태이기 때문에 뒤에 추가를 하는 것은 가능하나 
// 값을 바꾸는 것은 불가능.

enum	{
	CAM_ATTACH_FREE = 0,					// 절대 좌표
	CAM_ATTACH_LOCAL_AXIS,					// 다른 Attach기준으로 axis
	CAM_ATTACH_LOCAL_CRD,					// 다른 Attach기준으로 Local coord.
	CAM_ATTACH_PLAYER_TROOP,				// Player 부대
	CAM_ATTACH_PLAYER,						// Player
	CAM_ATTACH_TARGET_TROOP,				// Target troop	
	CAM_ATTACH_TARGET_LEADER,				// Target leader
	CAM_ATTACH_TROOP_UNIT,					// 부대원 (임의로)
	CAM_ATTACH_TROOP_UNIT_ALIVE,			// 부대원 (살아있는 녀석)
	CAM_ATTACH_NEAREST_ENEMY,				// 가장 가까운 적
	CAM_ATTACH_TROOP_PROJECTILE,			// 부대에서 쏜 projectile
	CAM_ATTACH_ACTION_EYE,					// Action 뷰1: 플레이어 등뒤
	CAM_ATTACH_ORBSPARK_EYE,				// orb spark를 플레이어에서 바라본 방향
	CAM_ATTACH_LEADER_FORWARD,				// 리더 약간 앞 쪽
	CAM_ATTACH_NEAREST_ENEMY_LEADER,		// 가장 가까운 Enemy leader
	CAM_ATTACH_MARKED_AREA,					// 미니맵에 표시된 Area
	CAM_ATTACH_ORBSPARK_TARGET,				// orb spark의 좌표
	CAM_ATTACH_MARKED_POINT_FROM_CURRENT,	// 표시된 영역의 중심을 현재 부대에서 바라본 방향
	CAM_ATTACH_PLAYER_TROOP_ACTUAL,			// Player 부대 유닛들 실제 위치를 모아모아서 평균
	CAM_ATTACH_TROOP_UNIT_ALIVE_3D,			// 타겟 부대의 살아남은 녀석 3D 좌표
	CAM_ATTACH_AXIS_PLAYER_TO_TARGET,		// 플레이어에서 타겟 방향으로의 축. eye에 사용.
	CAM_ATTACH_AXIS_PLAYERTROOP_TO_TARGET,	// 플레이어 부대에서 타겟 방향으로의 축. eye에 사용.
	CAM_ATTACH_AXIS_TARGETTROOP_TO_TARGET,	// 플레이어 부대에서 타겟 방향으로의 축. eye에 사용.
	CAM_ATTACH_AXIS_MARKEDAREA_TO_TARGET,	// 플레이어 부대에서 타겟 방향으로의 축. eye에 사용.


	CAM_ATTACH_MAX
};

bool	IsLocalAttach(int nAttach);