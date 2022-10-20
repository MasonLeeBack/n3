#pragma		 once

// attach data�� save�� data�� �̹� ��ϵ� �����̱� ������ �ڿ� �߰��� �ϴ� ���� �����ϳ� 
// ���� �ٲٴ� ���� �Ұ���.

enum	{
	CAM_ATTACH_FREE = 0,					// ���� ��ǥ
	CAM_ATTACH_LOCAL_AXIS,					// �ٸ� Attach�������� axis
	CAM_ATTACH_LOCAL_CRD,					// �ٸ� Attach�������� Local coord.
	CAM_ATTACH_PLAYER_TROOP,				// Player �δ�
	CAM_ATTACH_PLAYER,						// Player
	CAM_ATTACH_TARGET_TROOP,				// Target troop	
	CAM_ATTACH_TARGET_LEADER,				// Target leader
	CAM_ATTACH_TROOP_UNIT,					// �δ�� (���Ƿ�)
	CAM_ATTACH_TROOP_UNIT_ALIVE,			// �δ�� (����ִ� �༮)
	CAM_ATTACH_NEAREST_ENEMY,				// ���� ����� ��
	CAM_ATTACH_TROOP_PROJECTILE,			// �δ뿡�� �� projectile
	CAM_ATTACH_ACTION_EYE,					// Action ��1: �÷��̾� ���
	CAM_ATTACH_ORBSPARK_EYE,				// orb spark�� �÷��̾�� �ٶ� ����
	CAM_ATTACH_LEADER_FORWARD,				// ���� �ణ �� ��
	CAM_ATTACH_NEAREST_ENEMY_LEADER,		// ���� ����� Enemy leader
	CAM_ATTACH_MARKED_AREA,					// �̴ϸʿ� ǥ�õ� Area
	CAM_ATTACH_ORBSPARK_TARGET,				// orb spark�� ��ǥ
	CAM_ATTACH_MARKED_POINT_FROM_CURRENT,	// ǥ�õ� ������ �߽��� ���� �δ뿡�� �ٶ� ����
	CAM_ATTACH_PLAYER_TROOP_ACTUAL,			// Player �δ� ���ֵ� ���� ��ġ�� ��Ƹ�Ƽ� ���
	CAM_ATTACH_TROOP_UNIT_ALIVE_3D,			// Ÿ�� �δ��� ��Ƴ��� �༮ 3D ��ǥ
	CAM_ATTACH_AXIS_PLAYER_TO_TARGET,		// �÷��̾�� Ÿ�� ���������� ��. eye�� ���.
	CAM_ATTACH_AXIS_PLAYERTROOP_TO_TARGET,	// �÷��̾� �δ뿡�� Ÿ�� ���������� ��. eye�� ���.
	CAM_ATTACH_AXIS_TARGETTROOP_TO_TARGET,	// �÷��̾� �δ뿡�� Ÿ�� ���������� ��. eye�� ���.
	CAM_ATTACH_AXIS_MARKEDAREA_TO_TARGET,	// �÷��̾� �δ뿡�� Ÿ�� ���������� ��. eye�� ���.


	CAM_ATTACH_MAX
};

bool	IsLocalAttach(int nAttach);