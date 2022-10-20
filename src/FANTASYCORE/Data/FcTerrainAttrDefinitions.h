#ifndef __FC_TERRAIN_ATTR_DEFINITIONS_H__
#define __FC_TERRAIN_ATTR_DEFINITIONS_H__

enum FC_TILE_TYPE
{
	FCTT_AB_00 = 0,//(����⺻��)
	FCTT_AB_01 = 1,//(����ٴ�1))
	FCTT_AB_02 = 2,//(���赹)
	FCTT_AB_03 = 3,//(���踶�����)
	FCTT_AB_04 = 4,//(������)
	FCTT_CS_00 = 5,//(���⵵�� �⺻��)
	FCTT_CS_01 = 6,//(���⵵�� �ΰ����ٴ�
	FCTT_CS_02 = 7,//(���⵵�� �ΰ����ٴ�
	FCTT_CS_03 = 8,//(���⵵�� ����)
	FCTT_CS_04 = 9,//(���⵵�� Ǯ)
	FCTT_FM_00 = 10,//(���� �⺻��)
	FCTT_FM_01 = 11,//(���� �ΰ����ٴ�3)
	FCTT_FM_02 = 12,//(���� ��)
	FCTT_FM_03 = 13,//(���� �⺻Ǯ)
	FCTT_FM_04 = 14,//(���� ��ģǮ)
	FCTT_FR_00 = 15,//(�� �⺻Ǯ)
	FCTT_FR_01 = 16,//(�� �̳�Ǯ)
	FCTT_FR_02 = 17,//(�� ��)
	FCTT_FR_03 = 18,//(�� �⺻��)
	FCTT_FR_04 = 19,//(�� ����� ���ٴ�)
	FCTT_GV_00 = 20,//(������ ������)
	FCTT_GV_01 = 21,//(������ Ǯ&������)
	FCTT_GV_02 = 22,//(������ ��)
	FCTT_GV_03 = 23,//(������ ��ģ������)
	FCTT_GV_04 = 24,//(������ Ǯ&��������)
	FCTT_MP_00 = 25,//(��� ��)
	FCTT_MP_01 = 26,//(��� ��&��)
	FCTT_MP_02 = 27,//(��� ��)
	FCTT_MP_03 = 28,//(��� ��ģ��)
	FCTT_MP_04 = 29,//(��� Ǯ)
	FCTT_OC_00 = 30,//(���� ��)
	FCTT_OC_01 = 31,//(���� ��2)
	FCTT_OC_02 = 32,//(���� ��)
	FCTT_OC_03 = 33,//(���� ����)
	FCTT_OC_04 = 34,//(���� �ΰ� ���ٴ�)
	FCTT_PL_00 = 35,//(��� �⺻Ǯ)
	FCTT_PL_01 = 36,//(��� ��ģǮ)
	FCTT_PL_02 = 37,//(��� ��)
	FCTT_PL_03 = 38,//(��� ��)
	FCTT_PL_04 = 39,//(��� ��&��)
	FCTT_VF_02 = 40,//(���� ��2)
	FCTT_WATER = 41,//�ӽ÷� ��
	FC_TILE_TYPE_LAST
};

int g_nTerrainParticleTable[ FC_TILE_TYPE_LAST ] =
{
	0, // ��ο� ��		FCTT_AB_00,//(����⺻��)
	5, //X����		FCTT_AB_01,//(����ٴ�1))
	5, //X����		FCTT_AB_02,//(���赹)
	5, //X����		FCTT_AB_03,//(���踶�����)
	5, //X����		FCTT_AB_04,//(������)
	1, // Ȳ��		FCTT_CS_00,//(���⵵�� �⺻��)
	5, //X����		FCTT_CS_01,//(���⵵�� �ΰ����ٴ�
	5, //X����		FCTT_CS_02,//(���⵵�� �ΰ����ٴ�
	5, //X����		FCTT_CS_03,//(���⵵�� ����)
	3, // Ǯ		FCTT_CS_04,//(���⵵�� Ǯ)
	1, // Ȳ��		FCTT_FM_00,//(���� �⺻��)
	5, //X����		FCTT_FM_01,//(���� �ΰ����ٴ�3)
	5, //X����		FCTT_FM_02,//(���� ��)
	3, // Ǯ		FCTT_FM_03,//(���� �⺻Ǯ)
	6, // ª�� Ǯ	FCTT_FM_04,//(���� ��ģǮ)
	3, // Ǯ		FCTT_FR_00,//(�� �⺻Ǯ)
	6, // ª�� Ǯ	FCTT_FR_01,//(�� �̳�Ǯ)
	5, //X����		FCTT_FR_02,//(�� ��)
	0, // ��ο� ��	FCTT_FR_03,//(�� �⺻��)
	5, //X����		FCTT_FR_04,//(�� ����� ���ٴ�)
	7, // ���� ��	FCTT_GV_00,//(������ ������)
	8, // ���� Ǯ	FCTT_GV_01,//(������ Ǯ&������)
	5, //X����		FCTT_GV_02,//(������ ��)
	7, // ���� ��	FCTT_GV_03,//(������ ��ģ������)
	8, // ���� Ǯ	FCTT_GV_04,//(������ Ǯ&��������)
	0, // ��ο� ��	FCTT_MP_00,//(��� ��)
	0, // ��ο� ��	FCTT_MP_01,//(��� ��&��)
	5, //X����		FCTT_MP_02,//(��� ��)
	0, // ��ο� ��	FCTT_MP_03,//(��� ��ģ��)
	3, // Ǯ		FCTT_MP_04,//(��� Ǯ)
	2, // ��		FCTT_OC_00,//(���� ��)
	2, // ��		FCTT_OC_01,//(���� ��2)
	5, //X����		FCTT_OC_02,//(���� ��)
	5, //X����		FCTT_OC_03,//(���� ����)
	5, //X����		FCTT_OC_04,//(���� �ΰ� ���ٴ�)
	3, // Ǯ		FCTT_PL_00,//(��� �⺻Ǯ)
	6, // ª�� Ǯ	FCTT_PL_01,//(��� ��ģǮ)
	0, // ��ο� ��	FCTT_PL_02,//(��� ��)
	5, //X����		FCTT_PL_03,//(��� ��)
	0, // ��ο� ��	FCTT_PL_04,//(��� ��&��)
	5,  //X����		FCTT_VF_02,//(���� ��2)
	4 //��
};

#endif