#pragma once

class CCrossVector
{
public:
	CCrossVector();
	CCrossVector( const D3DXMATRIX &Matrix );
	void Reset();

#ifdef _XBOX
	D3DX_ALIGN16 D3DXVECTOR3		m_XVector;	
	D3DX_ALIGN16 D3DXVECTOR3		m_YVector;	
	D3DX_ALIGN16 D3DXVECTOR3		m_ZVector;	
	D3DX_ALIGN16 D3DXVECTOR3		m_PosVector;
#else
	D3DXVECTOR3	m_XVector;	
	float		m_fDummy1;
	D3DXVECTOR3	m_YVector;	
	float		m_fDummy2;
	D3DXVECTOR3	m_ZVector;	
	float		m_fDummy3;
	D3DXVECTOR3	m_PosVector;
	float		m_fDummy4;
#endif

public:
	void RotateYaw(int nAngle);
	void RotateRoll(int nAngle);
	void RotatePitch(int nAngle);
	void RotateYawByWorld(int nAngle);
	void RotateRollByWorld(int nAngle);
	void RotatePitchByWorld(int nAngle);
	void MoveFrontBack(float fDist) { m_PosVector+=fDist*m_ZVector; }
	void MoveUpDown(float fDist) { m_PosVector+=fDist*m_YVector; }
	void MoveRightLeft(float fDist) { m_PosVector+=fDist*m_XVector; }
	void SetPosition(float fX, float fY, float fZ);
	void SetPosition(D3DXVECTOR3 Vector) { m_PosVector=Vector; }
	D3DXVECTOR3 GetPosition() { return m_PosVector; };
	D3DXVECTOR2 GetPositionXZ()	{	return D3DXVECTOR2(m_PosVector.x, m_PosVector.z);	}

	operator D3DXMATRIX *();
	CCrossVector &operator =(D3DXMATRIX &Matrix);	
	void CopyCameraMatrix(D3DXMATRIX &Matrix); // ī�޶� ���ϸ��̼� BA�� ����� ��Ʈ������ �޾Ƽ� �����մϴ�
	D3DXMATRIX ConvertViewMatrix();
	void CopyRotateInfo(CCrossVector *pCross);
	void UpdateVectors();		// Z Axis �����ϸ鼭, ������ǥ��� ��ȯ
	void UpdateVectorsY();		// Y Axis �����ϸ鼭, ������ǥ��� ��ȯ
	void UpdateVectorsX();		// X Axis �����ϸ鼭, ������ǥ��� ��ȯ

	void LookAt(D3DXVECTOR3 *pAt, D3DXVECTOR3 *pUpVec=NULL);
};
