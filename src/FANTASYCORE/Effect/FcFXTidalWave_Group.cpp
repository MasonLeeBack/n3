#include "stdafx.h"
#include "FcFXTidalWave.h"
#include "3DDevice.h"
#include "BsKernel.h"
#include "BsObject.h"
#include "FCWorld.h"
#include "FCheroObject.h"
#include "FCGameObject.h"
#include "FcUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif //_DEBUG

extern	BSVECTOR	g_SphereTable[][SPHERE_TABLE_SIZE];


int		CFcFXTidalWave::_Group(int iX1, int iY1, int iX2, int iY2)
{

	
	m_G.iX1 = iX1;
	m_G.iY1 = iY1;
	m_G.iX2 = iX2;
	m_G.iY2 = iY2;

	m_G.iCol = iX2 - iX1 + 1;
	m_G.iRow = iY2 - iY1 + 1;

	m_G.iV = m_G.iCol * m_G.iRow;

	for(int iR = iY1; iR <= iY2; iR++)
		for(int iC = iX1; iC <= iX2; iC++)
			m_pVList[iR * m_iCol + iC].bNoEff = true;

	m_G.vPos = (BSVECTOR*) malloc (sizeof(BSVECTOR) * m_G.iV);
	memset(m_G.vPos,0,sizeof(BSVECTOR) * m_G.iV);
	

	return 0;
}


void	CFcFXTidalWave::Ungroup()
{
	if (m_G.iV > 0)
	{
		for(int iR = m_G.iY1; iR <= m_G.iY2; iR++)
			for(int iC = m_G.iX1; iC <= m_G.iX2; iC++)
				m_pVList[iR * m_iCol + iC].bNoEff = false;

		m_G.iV = 0;

		free(m_G.vPos);
	}
}

void D3DXVec3RotationAxisX( OUT D3DXVECTOR3* p_vOut, IN D3DXVECTOR3 p_vIn, IN D3DXVECTOR3 p_vInCenter, IN float p_fAngle, IN D3DXVECTOR3 p_vAxis )
{
	D3DXMATRIX tmpMat;
	D3DXMatrixRotationAxis(&tmpMat, &p_vAxis, p_fAngle);
	tmpMat._41 = p_vInCenter.x, tmpMat._42 = p_vInCenter.y, tmpMat._43 = p_vInCenter.z;
	D3DXVec3TransformCoord(p_vOut, &p_vIn, &tmpMat);
}

float Lerp( float p_fStart, float p_fEnd, float p_fPoint )
{
	return (p_fEnd * p_fPoint + p_fStart * (1.0f - p_fPoint));
}


void	CFcFXTidalWave::SetGroupPos( BSVECTOR *pPos, float fR)
{
	m_G.Pos = (*pPos);
	m_G.fRad = fR;


	int	iCols,iRows;

	iRows = m_G.iRow;
	iCols = m_G.iCol;

	bool bIsFirst = true;

	CCrossVector *Cross = m_hParent->GetCrossVector();
	D3DXVECTOR3 vCharDir ,vCharLeft, vCharPos;
	vCharDir = Cross->m_ZVector;
	vCharPos = Cross->m_PosVector;
	D3DXVec3Cross(&vCharLeft, &vCharDir, &D3DXVECTOR3(0,-1,0));

	for(int iR = 0; iR < iRows; iR++)
	{
		for(int iC = 0; iC < iCols; iC++)
		{
			if(GetWaterDrop() )
			{
				int n = iR * iCols + iC;

				D3DXVECTOR3	vert;
				vert = -D3DXVECTOR3(0,0,1) * 10.0f;
				vert -= D3DXVECTOR3(1,0,0) * 15.0f;
				float fiC = ( (float)iC/((float)iCols*0.5f) - 1.0f );
				
				if( iR < 25 )
				{
					float fiR = ((float)iR)/((float)iRows * 0.5f) * 2.0f;
					float fValue = fiC * (fiR+0.1f);
                    vert += D3DXVECTOR3( fValue, 0 , -(fValue*fValue) + 4.0f ) * 3.0f;
				}
				else
				{
					float fiR = (((float)iR)/((float)iRows * 0.5f) - 1.0f) * 2.0f;
					float fValue = fiC * (fiR+0.1f);
					vert += D3DXVECTOR3( fValue, 0 ,  (fValue*fValue) - 4.0f ) * 3.0f;;
				}

				vert.z *= 0.5f;

				D3DXVECTOR3 vResut = vCharLeft * vert.x + vCharDir * vert.z;

				//vResut.y += (float)(iR * iR * iR) / 1000.0f;

				m_G.vPos[n] = BSVECTOR(vResut.x,vResut.y, vResut.z);
			}
			else
			{

				// table에서 보간

				float	fX, fY;
				int		iX, iY;

				fX = (float)iC;
				fX *= SPHERE_TABLE_SIZE / (float) iCols;
				fY = (float)iR;
				fY *= SPHERE_TABLE_SIZE / (float) iRows;

				iX = (int) fX;
				iY = (int) fY;

				fX = fX - (float) iX;
				fY = fY - (float) iY;

				BSVECTOR	V[4];

				V[0] = g_SphereTable[iX][iY];

				if (iX < SPHERE_TABLE_SIZE - 1)
					V[1] = g_SphereTable[iX+1][iY];
				else
					V[1] = V[0];

				if (iY < SPHERE_TABLE_SIZE - 1)
					V[2] = g_SphereTable[iX][iY + 1];
				else
					V[2] = V[0];

				if (iX < SPHERE_TABLE_SIZE - 1 && iY < SPHERE_TABLE_SIZE - 1)
				{
					V[3] = g_SphereTable[iX + 1][iY + 1];
				}
				else
				{
					if (iX < SPHERE_TABLE_SIZE - 1)
						V[3] = V[1];
					else if (iY < SPHERE_TABLE_SIZE - 1)
						V[3] = V[2];
					else
						V[3] = V[0];
				}

				BSVECTOR	V1, V2;

				V1 = (1.0f - fX) * V[0] + fX * V[1];
				V2 = (1.0f - fX) * V[2] + fX * V[3];

				BSVECTOR	D1, D2, D3;

				D1 = (1.0f - fY) * V1;
				D2 = fY * V2;
				D3 = fR * (D1 + D2);

				m_G.vPos[iR * iCols + iC] = fR * ((1.0f - fY) * V1 + fY * V2);
			}
			
		}
	}

	if (m_bInitialized == false)
	{

		
		for (int iY = m_G.iY1; iY <= m_G.iY2; iY++)
		{
			for (int iX = m_G.iX1; iX <= m_G.iX2; iX++)
			{
				int	iFrom = iX - m_G.iX1 + (iY - m_G.iY1) * m_G.iCol;
				int	iTo = iY * m_iCol + iX;


				m_pV2[iTo].Pos = (*pPos) + m_G.vPos[iFrom]; 

				m_pVList[iTo].Acc = BSVECTOR(0.0f,0.0f,0.0f);
				m_pVList[iTo].Vel = BSVECTOR(0.0f,0.0f,0.0f);
			}
		}
		
		
	}


	m_bInitialized = true;
}


void	CFcFXTidalWave::Explode( BSVECTOR *pOrigin, float fSpeedMax)
{
	BSVECTOR	vO = *pOrigin;
	BSVECTOR	Diff;
	float		fRLimit = 1.8f;

	CCrossVector *Cross = m_hParent->GetCrossVector();
	D3DXVECTOR3 CharDir ,CharRight, CharPos;
	CharDir = Cross->m_ZVector;
	CharPos = Cross->m_PosVector;
	D3DXVec3Cross(&CharRight, &CharDir, &D3DXVECTOR3(0,-1,0));


	if(GetWaterDrop())
	{
		//vO.y -= 10.0f;
		vO -= BSVECTOR(CharRight.x ,CharRight.y ,CharRight.z ) * 10.0f;
		vO -= BSVECTOR(CharDir.x ,CharDir.y ,CharDir.z ) * 3.0f;
	}

	Diff = vO - m_G.Pos;
	float	fDist;

	fDist = BsVec3Length(&Diff);

	if (fDist > m_G.fRad * fRLimit)
		vO = m_G.Pos + Diff / fDist * fRLimit * m_G.fRad;

	int count = 0;
	for (int iY = m_G.iY1; iY <= m_G.iY2; iY++)
	{
		for (int iX = m_G.iX1; iX <= m_G.iX2; iX++)
		{
			count++;
			int	iN = iX + iY * m_G.iCol;
			int	iN2 = iY * m_iCol + iX;

			BSVECTOR	Vel = m_G.Pos + m_G.vPos[iN] - vO; 
			float	fDist = BsVec3Length(&Vel);
			float	fFactor = fDist / ((1.0f + fRLimit) * m_G.fRad);

			if(GetWaterDrop() )
			{
                Vel.y = 300.0f * fFactor;
				Vel *= fDist*fDist / ((fRLimit + 1.0f) * (fRLimit + 1.0f)* m_G.fRad * m_G.fRad * 10.0f );
			}
			else
			{
				Vel.y = 300.0f * fFactor;
                Vel *= fDist / ((fRLimit + 1.0f) * (fRLimit + 1.0f)* m_G.fRad * m_G.fRad);
			}

			m_pVList[iN2].Vel = Vel * fSpeedMax * (0.9f + (float)(rand() % 40) / 100.0f);

			
			if(GetWaterDrop() )
			{////////////////////////////////////////////////////////////////////////////////////////

				D3DXVECTOR3 vDir = CharDir + CharRight;
				D3DXVec3Normalize(&vDir, &vDir);
				vDir *= 20.0f;

				BSVECTOR pos = m_G.Pos + m_G.vPos[iN];
				int nn = iX>8 ? 8 : iX;

				m_pVList[iN2].Vel.y = ( sinf( D3DX_PI/(float)m_G.iX2 * (float)iX ) - 0.4f ) * (float)(nn+0.2f)*(2.0f) * (float)iY * 0.02f;
				if( m_pVList[iN2].Vel.y <= 0 ) m_pVList[iN2].Vel.y = m_pVList[iN2].Vel.y * 2.0f - 5.0f;

				m_pVList[iN2].Vel.x += vDir.x, m_pVList[iN2].Vel.z += vDir.z;
			}/////////////////////////////////////////////////////////////////////////////////////////

			
		}// for
	}// for

	Ungroup();
}



