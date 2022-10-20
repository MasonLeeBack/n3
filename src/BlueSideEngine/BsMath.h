#ifdef _XBOX

// vector
#define BSVECTOR					XMVECTOR
#define XMVECTOR(a,b,c)				XMVectorSet(a,b,c,0.f)
#define BsVec3Dot(a,b)				XMVector3Dot(*a,*b).x
#define BsVec3Lerp(a,b,c,d)			*a=XMVectorLerp(*b,*c,d)
#define BsVec3TransformCoord(a,b,c)	*a=XMVector3TransformCoord(*b,*c)
#define BsVec3Normalize(a,b)		*a=XMVector3Normalize(*b)
#define BsVec3Maximize(a,b,c)		*a=XMVectorMax(*b,*c)
#define BsVec3Minimize(a,b,c)		*a=XMVectorMin(*b,*c)
#define BsVec3Length(a)				XMVector3Length(*a).x
#define BsVec3LengthSq(a)				XMVector3LengthSq(*a).x
#define BsVec3Cross(a,b,c)			*a=XMVector3Cross(*b,*c)

// Quaternion
#define BSQUATERNION				XMVECTOR
#define BsQuaternionSlerp(a,b,c,d)	*a=XMQuaternionSlerp(*b,*c,d)

// matrix
#define BSMATRIX									XMMATRIX
#define BsMatrixScaling(a,b,c,d)					*a=XMMatrixScaling(b,c,d)
#define BsMatrixMultiply(a,b,c)						*a=XMMatrixMultiply(*b,*c);
#define BsMatrixRotationYawPitchRoll(a,b,c,d)		*a=XMMatrixRotationRollPitchYaw(c,b,d)
#define BsMatrixTransformation(a,b,c,d,e,f,g)		*a=XMMatrixTransformation(*b,*c,*d,*e,*f,*g)
#define BsMatrixIdentity(a)							*a=XMMatrixIdentity()
#define BsMatrixRotationY(a,b)						*a=XMMatrixRotationY(b)
#define BsMatrixRotationQuaternion(a,b)				*a=XMMatrixRotationQuaternion(*b)
#define BsMatrixInverse(a,b)						*a=XMMatrixInverse( &XMVectorZero() , *b)
#define BsMatrixTranslation(a,b,c,d)				*a=XMMatrixTranslation(b,c,d)
#define BsMatrixTranspose(a,b)						*a=XMMatrixTranspose(*b)

#else		// For PC

// vector
#define BSVECTOR					D3DXVECTOR3
#define D3DXVECTOR3(a,b,c)			D3DXVECTOR3(a,b,c)
#define BsVec3Dot(a,b)				D3DXVec3Dot(a,b)
#define BsVec3Lerp(a,b,c,d)			D3DXVec3Lerp(a,b,c,d)
#define BsQuaternionSlerp(a,b,c,d)	D3DXQuaternionSlerp(a,b,c,d)
#define BsVec3TransformCoord(a,b,c)	D3DXVec3TransformCoord(a,b,c)
#define BsVec3Normalize(a,b)		D3DXVec3Normalize(a,b)
#define BsVec3Maximize(a,b,c)		D3DXVec3Maximize(a,b,c)
#define BsVec3Minimize(a,b,c)		D3DXVec3Minimize(a,b,c)
#define BsVec3Length(a)				D3DXVec3Length(a)
#define BsVec3LengthSq(a)				D3DXVec3LengthSq(a)
#define BsVec3Cross(a,b,c)			D3DXVec3Cross(a,b,c)

// Quaternion
#define BSQUATERNION				D3DXQUATERNION
#define BsQuaternionSlerp(a,b,c,d)	D3DXQuaternionSlerp(a,b,c,d)

// matrix
#define BSMATRIX									D3DXMATRIX
#define BsMatrixScaling(a,b,c,d)					D3DXMatrixScaling(a,b,c,d)
#define BsMatrixMultiply(a,b,c)						D3DXMatrixMultiply(a,b,c)
#define BsMatrixRotationYawPitchRoll(a,b,c,d)		D3DXMatrixRotationYawPitchRoll(a,b,c,d)
#define BsMatrixTransformation(a,b,c,d,e,f,g)		D3DXMatrixTransformation(a,b,c,d,e,f,g)
#define BsMatrixIdentity(a)							D3DXMatrixIdentity(a);
#define BsMatrixRotationY(a,b)						D3DXMatrixRotationY(a,b)
#define BsMatrixRotationQuaternion(a,b)				D3DXMatrixRotationQuaternion(a,b)
#define BsMatrixInverse(a,b)						D3DXMatrixInverse(a,0,b);
#define BsMatrixTranslation(a,b,c,d)				D3DXMatrixTranslation(a,b,c,d)
#define BsMatrixTranspose(a,b)						D3DXMatrixTranspose(a,b)

#endif