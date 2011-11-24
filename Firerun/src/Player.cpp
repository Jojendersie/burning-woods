#include "StdAfx.h"
#include "Player.h"

const float Player::m_CameraRotSpeed = 0.01f;
const float Player::m_MovementSpeed = 3.25f;

// **************************************************************************************************************************************************** //
Player::Player()
{
	ZeroMemory(this, sizeof(Player));
}

// **************************************************************************************************************************************************** //
Player& Player::Get()
{
	static Player TheOneAndOnly;
	return TheOneAndOnly;
}


// **************************************************************************************************************************************************** //
Player::~Player()
{
}

// **************************************************************************************************************************************************** //
void Player::MovementMessage(const unsigned int Message, const WPARAM WParam, const LPARAM LParam)
{
	switch(Message)
	{
	case WM_MOUSEMOVE:
		m_LastMousePos.x = LOWORD(LParam);
		m_LastMousePos.y = HIWORD(LParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
		bool On = Message==WM_KEYDOWN;
		switch(WParam)
		{
		case VK_UP:
			m_Forward = On;
			break;
		case VK_DOWN:
			m_Back = On;
			break;
#ifdef TEXTURE_TEST
		case 0xbb: // +: VK_ADD doesn't react:
			if(Message==WM_KEYUP) g_TexIndex = min(g_TexIndex+1, 7);
			break;
		case 0xbd://VK_SUBTRACT:
			if(Message==WM_KEYUP) g_TexIndex = max(g_TexIndex-1, 0);
			break;
#endif
		}
		break;
	}
}

// **************************************************************************************************************************************************** //
void Player::Move()
{
	float MouseX = m_CameraRotSpeed * m_LastMousePos.x;
	float MouseY = m_CameraRotSpeed * m_LastMousePos.y;
	m_CameraDirection = D3DXVECTOR3(sinf(MouseX) * cosf(MouseY), sinf(MouseY), cosf(MouseX) * cosf(MouseY));
	m_CameraPosition = m_CameraPosition + float(m_Forward - m_Back) * m_CameraDirection * m_MovementSpeed;
	D3DXMatrixLookAtLH(&m_ViewMatrix, &m_CameraPosition, &(m_CameraPosition + m_CameraDirection), &D3DXVECTOR3(0,1,0));
}