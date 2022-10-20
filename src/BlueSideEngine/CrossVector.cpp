#include "stdafx.h"
#include "CrossVector.h"
#include "BsSinTable.h"

CCrossVector::CCrossVector()
{
	Reset();
}

CCrossVector::CCrossVector(const D3DXMATRIX &Matrix)
{
	*(D3DXMATRIX*)this = Matrix;
}

void CCrossVector::Reset()
{
	D3DXMatrixIdentity((D3DXMATRIX*)this);
}

void CCrossVector::RotateYaw(int nAngle)
{
	D3DXVECTOR3 Rotate;

	while( nAngle < 0 ) {
		nAngle += 1024;	
	}
	while( nAngle >= 1024 ) {
		nAngle-= 1024;
	}

	Rotate.x=-FSIN(nAngle)*m_XVector.x+FCOS(nAngle)*m_ZVector.x;
	Rotate.y=-FSIN(nAngle)*m_XVector.y+FCOS(nAngle)*m_ZVector.y;
	Rotate.z=-FSIN(nAngle)*m_XVector.z+FCOS(nAngle)*m_ZVector.z;
	m_ZVector=Rotate;
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
	D3DXVec3Cross(&m_XVector, &m_YVector, &m_ZVector);
	D3DXVec3Normalize(&m_XVector, &m_XVector);
}

void CCrossVector::RotateRoll(int nAngle)
{
	D3DXVECTOR3 Rotate;

	while( nAngle < 0 ) {
		nAngle += 1024;	
	}
	while( nAngle >= 1024 ) {
		nAngle-= 1024;
	}

	Rotate.x=FSIN(nAngle)*m_XVector.x+FCOS(nAngle)*m_YVector.x;
	Rotate.y=FSIN(nAngle)*m_XVector.y+FCOS(nAngle)*m_YVector.y;
	Rotate.z=FSIN(nAngle)*m_XVector.z+FCOS(nAngle)*m_YVector.z;
	m_YVector=Rotate;
	D3DXVec3Normalize(&m_YVector, &m_YVector);
	D3DXVec3Cross(&m_XVector, &m_YVector, &m_ZVector);
	D3DXVec3Normalize(&m_XVector, &m_XVector);
}

void CCrossVector::RotateYawByWorld(int nAngle)
{
	D3DXMATRIX MatRotation;

	nAngle=-nAngle;
	if(nAngle<0){
		nAngle+=1024;
	}
	D3DXMatrixRotationY(&MatRotation, D3DXToRadian(nAngle*360.0f/NUM_SINTABLE));
	D3DXVec3TransformNormal(&m_ZVector, &m_ZVector, &MatRotation);
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
	D3DXVec3TransformNormal(&m_XVector, &m_XVector, &MatRotation);
	D3DXVec3Normalize(&m_XVector, &m_XVector);
	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
}

void CCrossVector::RotateRollByWorld(int nAngle)
{
	D3DXMATRIX MatRotation;

	nAngle=-nAngle;
	if(nAngle<0){
		nAngle+=1024;
	}
	D3DXMatrixRotationZ(&MatRotation, D3DXToRadian(nAngle*360.0f/NUM_SINTABLE));
	D3DXVec3TransformNormal(&m_ZVector, &m_ZVector, &MatRotation);
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
	D3DXVec3TransformNormal(&m_XVector, &m_XVector, &MatRotation);
	D3DXVec3Normalize(&m_XVector, &m_XVector);
	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
}

void CCrossVector::RotatePitchByWorld(int nAngle)
{
	D3DXMATRIX MatRotation;

	if(nAngle<0){
		nAngle+=1024;
	}
	D3DXMatrixRotationX(&MatRotation, D3DXToRadian(nAngle*360.0f/NUM_SINTABLE));
	D3DXVec3TransformNormal(&m_ZVector, &m_ZVector, &MatRotation);
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
	D3DXVec3TransformNormal(&m_XVector, &m_XVector, &MatRotation);
	D3DXVec3Normalize(&m_XVector, &m_XVector);
	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
}

void CCrossVector::RotatePitch(int nAngle)
{
	D3DXVECTOR3 Rotate;

	while( nAngle < 0 ) {
		nAngle += 1024;	
	}
	while( nAngle >= 1024 ) {
		nAngle-= 1024;
	}

	Rotate.x=-FSIN(nAngle)*m_YVector.x+FCOS(nAngle)*m_ZVector.x;
	Rotate.y=-FSIN(nAngle)*m_YVector.y+FCOS(nAngle)*m_ZVector.y;
	Rotate.z=-FSIN(nAngle)*m_YVector.z+FCOS(nAngle)*m_ZVector.z;
	m_ZVector=Rotate;
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
	D3DXVec3Normalize(&m_YVector, &m_YVector);
}

void CCrossVector::SetPosition(float fX, float fY, float fZ)
{
	m_PosVector.x=fX;
	m_PosVector.y=fY;
	m_PosVector.z=fZ;
}

D3DXMATRIX CCrossVector::ConvertViewMatrix()
{
	D3DXMATRIX matRet;
	D3DXMatrixLookAtLH(&matRet, &m_PosVector, &(m_ZVector+m_PosVector), &m_YVector);
	return matRet;
}

CCrossVector::operator D3DXMATRIX *()
{
	return (D3DXMATRIX*)this;
}

CCrossVector &CCrossVector::operator =(D3DXMATRIX &Matrix)
{
	*(D3DXMATRIX*)this = Matrix;
	return *this;
}

void CCrossVector::CopyCameraMatrix(D3DXMATRIX &Matrix)
{
	m_XVector.x   = Matrix._11; //  x
	m_XVector.y   = Matrix._12;
	m_XVector.z   = Matrix._13;

	m_YVector.x   = Matrix._31; //  z
	m_YVector.y   = Matrix._32;
	m_YVector.z   = Matrix._33;

	m_ZVector.x   =-Matrix._21; // -y
	m_ZVector.y   =-Matrix._22;
	m_ZVector.z   =-Matrix._23;

	m_PosVector.x = Matrix._41;
	m_PosVector.y = Matrix._42;
	m_PosVector.z = Matrix._43;
}

void CCrossVector::CopyRotateInfo(CCrossVector *pCross)
{
	if(pCross){
		m_XVector=pCross->m_XVector;
		m_YVector=pCross->m_YVector;
		m_ZVector=pCross->m_ZVector;
	}
}

void CCrossVector::UpdateVectors()
{	// Z Axis
	D3DXVec3Cross(&m_XVector, &m_YVector, &m_ZVector);
	D3DXVec3Normalize(&m_XVector, &m_XVector);

	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
	D3DXVec3Normalize(&m_YVector, &m_YVector);
}

void CCrossVector::UpdateVectorsY()
{	// Y Axis
	D3DXVec3Cross(&m_XVector, &m_YVector, &m_ZVector);
	D3DXVec3Normalize(&m_XVector, &m_XVector);

	D3DXVec3Cross(&m_ZVector, &m_XVector, &m_YVector);
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
}

void CCrossVector::UpdateVectorsX()
{	// X Axis
	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
	D3DXVec3Normalize(&m_YVector, &m_YVector);

	D3DXVec3Cross(&m_ZVector, &m_XVector, &m_YVector);
	D3DXVec3Normalize(&m_ZVector, &m_ZVector);
}

void CCrossVector::LookAt(D3DXVECTOR3 *pAt, D3DXVECTOR3 *pUpVec/*=NULL*/)
{
	D3DXVECTOR3 vVec = *pAt-m_PosVector;

	if(D3DXVec3Length(&vVec) < 0.01f)
		return;

	D3DXVec3Normalize(&m_ZVector, &vVec);
	if(pUpVec){
		m_YVector=*pUpVec;
	}

	D3DXVec3Cross(&m_XVector, &m_YVector, &m_ZVector);
	D3DXVec3Normalize(&m_XVector, &m_XVector);

	D3DXVec3Cross(&m_YVector, &m_ZVector, &m_XVector);
	D3DXVec3Normalize(&m_YVector, &m_YVector);
}


