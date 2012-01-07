#pragma once

//#define FREE_CAMERA

class Player
{
public:
	~Player();

	// Singleton
	static Player& Get();

	// processes messages which are relevant for moving
	void MovementMessage(const unsigned int Message, const WPARAM WParam, const LPARAM LParam);

	// computes movment etc.
	void Move();

	//const D3DXMATRIX& GetViewMatrix() { return m_ViewMatrix; }

	D3DXMATRIX		m_ViewMatrix;
	D3DXVECTOR3		m_CameraPosition;
	D3DXVECTOR3		m_CameraDirection;
	float m_Speed;
private:
	Player();

	// Camera rotation
	POINT m_LastMousePos;
	POINT m_MouseDelta;
	static const float m_CameraRotSpeed;
	static const float m_Height;
	
	// Movement
	bool m_Forward;
	bool m_Back;
	static const float m_Accl;
	static const float m_Friction;
	static const float m_BackFactor;
};

