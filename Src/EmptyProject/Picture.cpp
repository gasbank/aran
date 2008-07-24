#include "EmptyProjectPCH.h"
#include "Picture.h"

Picture::Picture(void)
{
	m_d3dxMesh = 0;
	m_d3dTex = 0;
	m_x = m_y = 0;
}

Picture::~Picture(void)
{
	release();
}

void Picture::init( UINT x, UINT y, UINT width, UINT height, const TCHAR* imgFileName, LPDIRECT3DDEVICE9 d3dDev, UINT segments )
{
	UINT faces = segments * segments * 2;
	UINT vertices = (segments+1) * (segments+1);
	if (FAILED(D3DXCreateMeshFVF(faces, vertices, D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_TEX1, d3dDev, &m_d3dxMesh)))
	{
		exit(10);
	}

	Vertex* v = 0;
	m_d3dxMesh->LockVertexBuffer(0, (void**)&v);

	UINT i, j;
	for (i = 0; i < segments+1; ++i)
	{
		for (j = 0; j < segments+1; ++j)
		{
			Vertex* cv = &v[(segments+1) * i + j];
			cv->x = 1.0f/segments * j;
			cv->y = 1.0f/segments * i;
			cv->z = 0;
			cv->u = 1.0f/segments * j;
			cv->v = abs(cv->y - 1);
		}
	}
	

	m_d3dxMesh->UnlockVertexBuffer();

	WORD* ind = 0;
	m_d3dxMesh->LockIndexBuffer(0, (void**)&ind);

	for (i = 0; i < segments; ++i)
	{
		for (j = 0; j < segments; ++j)
		{
			ind[6 * (segments * i + j) + 0] = (WORD)((segments + 1) * i + j);
			ind[6 * (segments * i + j) + 1] = (WORD)((segments + 1) * i + j + 1 + segments);
			ind[6 * (segments * i + j) + 2] = (WORD)((segments + 1) * i + j + 1);

			ind[6 * (segments * i + j) + 3] = (WORD)((segments + 1) * i + j + 1);
			ind[6 * (segments * i + j) + 4] = (WORD)((segments + 1) * i + j + 1 + segments);
			ind[6 * (segments * i + j) + 5] = (WORD)((segments + 1) * i + j + 1 + segments + 1);
		}
	}

	m_d3dxMesh->UnlockIndexBuffer();

	if (FAILED(D3DXCreateTextureFromFile(d3dDev, imgFileName, &m_d3dTex)))
	{
		exit(100);
	}

	m_d3dDev = d3dDev;

	D3DXMatrixIdentity(&m_localXform);
}

void Picture::release()
{
	SAFE_RELEASE(m_d3dxMesh);
	SAFE_RELEASE(m_d3dTex);
}

void Picture::draw()
{
	m_d3dDev->SetTransform(D3DTS_WORLD, &m_localXform);
	m_d3dDev->SetTexture(0, m_d3dTex);
	m_d3dxMesh->DrawSubset(0);
}

void Picture::move( float dx, float dy )
{
	m_x += dx;
	m_y += dy;

	D3DXMatrixTranslation(&m_localXform, m_x, m_y, 0);
}

LRESULT Picture::handleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( hWnd );
	UNREFERENCED_PARAMETER( lParam );

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			// Map this key to a D3DUtil_CameraKeys enum and update the
			// state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
			// only if the key is not down
			PictureInput mappedKey = mapKey( ( UINT )wParam );
			if( mappedKey != PIC_UNKNOWN )
			{
				if( FALSE == IsKeyDown( m_aKeys[mappedKey] ) )
				{
					m_aKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
					++m_cKeysDown;
				}
			}
			break;
		}

	case WM_KEYUP:
		{
			// Map this key to a D3DUtil_CameraKeys enum and update the
			// state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
			PictureInput mappedKey = mapKey( ( UINT )wParam );
			if( mappedKey != PIC_UNKNOWN && ( DWORD )mappedKey < 8 )
			{
				m_aKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
				--m_cKeysDown;
			}
			break;
		}

	}

	return FALSE;
}

PictureInput Picture::mapKey( UINT nKey )
{
	switch( nKey )
	{
	case VK_LEFT:	return PIC_MOVE_LEFT;
	case VK_RIGHT:	return PIC_MOVE_RIGHT;
	case VK_UP:		return PIC_MOVE_UP;
	case VK_DOWN:	return PIC_MOVE_DOWN;
	}

	return PIC_UNKNOWN;
}

void Picture::frameMove( float fElapsedTime )
{
	m_vKeyboardDirection = D3DXVECTOR3( 0, 0, 0 );
	// Update acceleration vector based on keyboard state
	if( IsKeyDown( m_aKeys[PIC_MOVE_UP] ) )
		m_vKeyboardDirection.y += 1.0f;
	if( IsKeyDown( m_aKeys[PIC_MOVE_DOWN] ) )
		m_vKeyboardDirection.y -= 1.0f;
	if( IsKeyDown( m_aKeys[PIC_MOVE_RIGHT] ) )
		m_vKeyboardDirection.x += 1.0f;
	if( IsKeyDown( m_aKeys[PIC_MOVE_LEFT] ) )
		m_vKeyboardDirection.x -= 1.0f;

	// Update velocity
	m_vVelocity = m_vKeyboardDirection;

	// Simple euler method to calculate position delta
	D3DXVECTOR3 vPosDelta = m_vVelocity * fElapsedTime;
	m_vPos += vPosDelta;

	D3DXMATRIX mTrans;
	D3DXMatrixTranslation( &mTrans, m_vPos.x, m_vPos.y, m_vPos.z );
	m_localXform = mTrans;
}