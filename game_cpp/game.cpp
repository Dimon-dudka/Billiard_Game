
#include <cassert>
#include <cmath>
#include <array>

#include "../framework/scene.hpp"
#include "../framework/game.hpp"
#include "../framework/engine.hpp"

//-------------------------------------------------------
//	Basic Vector2 class
//-------------------------------------------------------

class Vector2
{
public:
	float x = 0.f;
	float y = 0.f;

	constexpr Vector2() = default;
	constexpr Vector2( float vx, float vy );
	constexpr Vector2( Vector2 const &other ) = default;

	//	Overriding some operators for easy work
	Vector2& operator/=(const float& div) {
		x /= div;
		y /= div;
		return *this;
	}

	Vector2& operator*=(const float& mul) {
		x *= mul;
		y *= mul;
		return *this;
	}

	Vector2& operator+=(const Vector2& vec) {
		x += vec.x;
		y += vec.y;
		return *this;
	}

	Vector2& operator-=(const Vector2& vec) {
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	Vector2 operator /(const float& div) {
		return Vector2{ x / div,y / div };
	}

	Vector2 operator *(const float& mul) {
		return Vector2{ x * mul,y * mul };
	}

	Vector2 operator -(const Vector2& vec) {
		return Vector2{ x - vec.x,y - vec.y };
	}

	Vector2 operator +(const Vector2& vec) {
		return Vector2{ x + vec.x,y + vec.y };
	}
};



constexpr Vector2::Vector2( float vx, float vy ) :
	x( vx ),
	y( vy )
{
}


//-------------------------------------------------------
//	game parameters
//-------------------------------------------------------

namespace Params
{
	namespace System
	{
		constexpr int targetFPS = 60;
	}

	namespace Table
	{
		constexpr float width = 15.f;
		constexpr float height = 8.f;
		constexpr float pocketRadius = 0.4f;

		static constexpr std::array< Vector2, 6 > pocketsPositions =
		{
			Vector2{ -0.5f * width, -0.5f * height },
			Vector2{ 0.f, -0.5f * height },
			Vector2{ 0.5f * width, -0.5f * height },
			Vector2{ -0.5f * width, 0.5f * height },
			Vector2{ 0.f, 0.5f * height },
			Vector2{ 0.5f * width, 0.5f * height }
		};

		static constexpr std::array< Vector2, 7 > ballsPositions =
		{
			// player ball
			Vector2( -0.3f * width, 0.f ),
			// other balls
			Vector2( 0.2f * width, 0.f ),
			Vector2( 0.25f * width, 0.05f * height ),
			Vector2( 0.25f * width, -0.05f * height ),
			Vector2( 0.3f * width, 0.1f * height ),
			Vector2( 0.3f * width, 0.f ),
			Vector2( 0.3f * width, -0.1f * height )
		};
	}

	namespace Ball
	{
		constexpr float radius = 0.3f;
	}

	namespace Shot
	{
		constexpr float chargeTime = 1.f;
	}
}


//-------------------------------------------------------
//	Table logic
//-------------------------------------------------------

class Table
{
public:
	Table() = default;
	Table( Table const& ) = delete;

	void init();
	void deinit();

	Scene::Mesh* getMesh(const size_t &i);
	void eraseMesh(const size_t &i);

private:
	std::array< Scene::Mesh*, 6 > pockets = {};
	std::array< Scene::Mesh*, 7 > balls = {};
	std::array<bool, 7>proof = {};
};


void Table::eraseMesh(const size_t& i) {
	proof.at(i) = false;
}


Scene::Mesh* Table::getMesh(const size_t& i) {
	return balls.at(i);
}


void Table::init()
{
	for ( int i = 0; i < 6; i++ )
	{
		assert( !pockets[ i ] );
		pockets[ i ] = Scene::createPocketMesh( Params::Table::pocketRadius );
		Scene::placeMesh( pockets[ i ], Params::Table::pocketsPositions[ i ].x, Params::Table::pocketsPositions[ i ].y, 0.f );
	}


	for ( int i = 0; i < 7; i++ )
	{
		assert( !balls[ i ] );
		balls[ i ] = Scene::createBallMesh( Params::Ball::radius );
		Scene::placeMesh( balls[ i ], Params::Table::ballsPositions[ i ].x, Params::Table::ballsPositions[ i ].y, 0.f );
		proof.at(i) = 1;
	}
}


void Table::deinit()
{
	
	for ( Scene::Mesh* mesh : pockets )
		Scene::destroyMesh( mesh );

	for (size_t i = 0; i < balls.size(); ++i) {
		if (proof.at(i)) {
			Scene::destroyMesh(getMesh(i));
		}
	}

	pockets = {};
	balls = {};
	proof = {};
}


//-------------------------------------------------------
//	game public interface
//-------------------------------------------------------

namespace Game
{
	Table table;

	bool isChargingShot = false;
	bool flagImpulses = false;

	float shotChargeProgress = 0.f;
	float tableBallMovingKoef = 0.995f;
	float tableEdgeKoef = 0.89f;
	float ballsCollisionKoef = 0.997f;

	Vector2 mouseCoordinates = { 0.f,0.f };

	std::array< Vector2, 7 > dtPositionOfBalls ={};
	std::array<float, 7> impulses = {};
	std::array <bool, 7> proofExisting={};
	std::array<Vector2, 7> dxdyVector = {};

	void init()
	{
		Engine::setTargetFPS( Params::System::targetFPS );
		Scene::setupBackground( Params::Table::width, Params::Table::height );
		table.init();

		for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {
			dtPositionOfBalls.at(i) = Params::Table::ballsPositions.at(i);
			dxdyVector.at(i) = { 0.f,0.f };
			proofExisting.at(i) = true;
			impulses.at(i) = 0.f;
		}

		mouseCoordinates = { 0.f,0.f };

	}


	void deinit()
	{
		table.deinit();

		dtPositionOfBalls = {};
		proofExisting = {};
		impulses = {};
		dxdyVector = {};
	}


	void update( float dt )
	{
		flagImpulses = false;

		//	If some ball is moving
		for (const auto& w : impulses) {
			if (w != 0.f)flagImpulses = true;
		}
		
		//	Update balls positions
		if (flagImpulses) {
		
			//	Balls moving
			for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {

				if (proofExisting.at(i) == false)continue;

				if (i == 0 && dxdyVector.at(i).x == 0.f && dxdyVector.at(i).y == 0.f) {

					dxdyVector.at(0) = { mouseCoordinates.x - dtPositionOfBalls.at(0).x,
						mouseCoordinates.y - dtPositionOfBalls.at(0).y };

					mouseCoordinates = { 0.f,0.f };
					
					float length = sqrtf(powf(dxdyVector.at(0).x,2) + powf(dxdyVector.at(0).y, 2));
					dxdyVector.at(i) /= length;

				}

				if(dxdyVector.at(i).x != 0.f && dxdyVector.at(i).y != 0.f){

					Vector2 V(dxdyVector.at(i).x * impulses.at(i), dxdyVector.at(i).y * impulses.at(i));

					dtPositionOfBalls.at(i) += V;

					//	Lowing speed by friction on the table
					impulses.at(i) *= tableBallMovingKoef;

					if (impulses.at(i) < 0.01f) {
						impulses.at(i) = 0.f;
						dxdyVector.at(i) = { 0.f,0.f };
					}
				}
			}

			//	Balls collisions
			for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {

				if (proofExisting.at(i) == false)continue;

				for (size_t j = i+1; j < Params::Table::ballsPositions.size(); ++j) {

					if (proofExisting.at(j) == false)continue;

					//	Normalized vector n{Dx,Dy}
					Vector2 N{ dtPositionOfBalls.at(i).x - dtPositionOfBalls.at(j).x,
					dtPositionOfBalls.at(i).y - dtPositionOfBalls.at(j).y };

					float distance = sqrtf(N.x * N.x + N.y * N.y);

					if (distance == 0.f) {
						distance = 0.01f;
					}

					//	If there is some collision
					if (distance <= Params::Ball::radius * 2) {
						
						//	Unit vecor un {unx,uny}
						Vector2 uN{ N / distance };

						//	Tangent vector ut{-un(y),un(x)}
						Vector2 uT{ -uN.y ,uN.x };

						//	Project V on the normal and tangent axes
						float vn1{ (uN.x * dxdyVector.at(i).x) + (uN.y * dxdyVector.at(i).y) };
						float vt1{ (uT.x * dxdyVector.at(i).x) + (uT.y * dxdyVector.at(i).y) };

						float vn2{ (uN.x * dxdyVector.at(j).x) + (uN.y * dxdyVector.at(j).y) };
						float vt2{ (uT.x * dxdyVector.at(j).x) + (uT.y * dxdyVector.at(j).y) };

						//	Balls bug collision solution
						float Dt = ((Params::Ball::radius * 2) - distance) / (vn2 - vn1);
						if (Dt > 0.6)
							Dt = 0.6;
						if (Dt < -0.6)
							Dt = -0.6;

						dtPositionOfBalls.at(i) -= {dxdyVector.at(i).x * Dt, dxdyVector.at(i).y * Dt};
						dtPositionOfBalls.at(j) -= {dxdyVector.at(j).x * Dt, dxdyVector.at(j).y * Dt};

						N = dtPositionOfBalls.at(i) - dtPositionOfBalls.at(j);

						distance = sqrtf(N.x * N.x + N.y * N.y);

						if (distance == 0.f) {
							distance = 0.01f;
						}

						//	Unit vecor un {unx,uny}
						uN = N / distance;

						vn2 = (uN.x * dxdyVector.at(j).x) + (uN.y * dxdyVector.at(j).y);
						vt2 = (uT.x * dxdyVector.at(j).x) + (uT.y * dxdyVector.at(j).y);

						//	Find new velocities on normal and tangent axes
						float vn1N{ vn2 }, vt1N{ vt1 }, vn2N{ vn1 }, vt2N{ vt2 };

						//	Convert the scalar normal and tangential velocities into vectors
						Vector2 vnV1{ vn1N * uN.x ,vn1N * uN.y };
						Vector2 vtV1{ vt1N * uT.x,vt1N * uT.y };
						Vector2 vnV2{ vn2N * uN.x ,vn2N * uN.y };
						Vector2 vtV2{ vt2N * uT.x,vt2N * uT.y };

						dxdyVector.at(i) = vnV1 + vtV1;
						dxdyVector.at(j) = vnV2 + vtV2;

						//	Balls bug collision solution
						dtPositionOfBalls.at(i) += dxdyVector.at(i) * Dt;
						dtPositionOfBalls.at(j) += dxdyVector.at(j) * Dt;

						//	Momentum calculate
						if (impulses.at(i) == 0.f)
							impulses.at(i) = impulses.at(j);
						else 
							impulses.at(j) = impulses.at(i);

						impulses.at(i) *= ballsCollisionKoef;
						impulses.at(j) *= ballsCollisionKoef;
					}
				}
			}

			//	Walls collisions
			for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {
				if (impulses.at(i) != 0.f) {
					if (dtPositionOfBalls.at(i).x + Params::Ball::radius >= Params::Table::width / 2) {
						dtPositionOfBalls.at(i).x = (Params::Table::width / 2) - Params::Ball::radius;
						dxdyVector.at(i).x *= -1;
						impulses.at(i) *= tableEdgeKoef;
					}
					else if (dtPositionOfBalls.at(i).x - Params::Ball::radius <= -Params::Table::width / 2) {
						dtPositionOfBalls.at(i).x = -(Params::Table::width / 2) + Params::Ball::radius;
						dxdyVector.at(i).x *= -1;
						impulses.at(i) *= tableEdgeKoef;
					}
					else if (dtPositionOfBalls.at(i).y + Params::Ball::radius >= Params::Table::height / 2) {
						dtPositionOfBalls.at(i).y = (Params::Table::height / 2) - Params::Ball::radius;
						dxdyVector.at(i).y *= -1;
						impulses.at(i) *= tableEdgeKoef;
					}
					else if (dtPositionOfBalls.at(i).y - Params::Ball::radius <= -Params::Table::height / 2) {
						dtPositionOfBalls.at(i).y = -(Params::Table::height / 2) + Params::Ball::radius;
						dxdyVector.at(i).y *= -1;
						impulses.at(i) *= tableEdgeKoef;
					}
				}
			}

			//	Proof that ball is in some pocket
			for (const auto& w : Params::Table::pocketsPositions) {
				for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {

					//	If ball already in pocket
					if (proofExisting.at(i) == false)continue;
					
					float distance{ sqrtf(powf(dtPositionOfBalls.at(i).x - w.x,2)
						+ powf(dtPositionOfBalls.at(i).y - w.y,2)) };

					//	Pocket radius / 1.5 - because ball may not be in pocket only if he touches it
					if (distance < Params::Ball::radius + Params::Table::pocketRadius / 1.5) {

						//	If in the pocket is player ball then restart the game;
						if (i == 0) {
							deinit();
							init();
							return;
						}
						else {
							if (proofExisting.at(i))
								Scene::destroyMesh(table.getMesh(i));

							table.eraseMesh(i);
							proofExisting.at(i) = false;
							impulses.at(i) = 0.f;
							dtPositionOfBalls.at(i) = { 0.f,0.f };
							dxdyVector.at(i) = { 0.f,0.f };
						}
					}
				}
			}

			//	Place Balls
			for (size_t i = 0; i < Params::Table::ballsPositions.size(); ++i) {
				if (proofExisting.at(i)) {
					Scene::placeMesh(table.getMesh(i), dtPositionOfBalls.at(i).x, dtPositionOfBalls.at(i).y, 0.f);
				}
			}			
		}

		//	If mouse button is pressed and none of balls is moving
		if (isChargingShot && !flagImpulses) {
			shotChargeProgress = std::min(shotChargeProgress + dt / Params::Shot::chargeTime, 1.f);
		}
		Scene::updateProgressBar( shotChargeProgress );

	}

	
	void mouseButtonPressed( float x, float y )
	{
		isChargingShot = true;
	}


	void mouseButtonReleased( float x, float y )
	{
		//	If some ball is moving we cannot shot the player ball
		if (!flagImpulses) {
			
			//	Becoming an impulse on player ball and mouse coordinates
			if (dtPositionOfBalls.at(0).x != x && dtPositionOfBalls.at(0).y != y) {
				impulses.at(0) = shotChargeProgress / 2;
				mouseCoordinates = { x,y };
			}
		}

		isChargingShot = false;
		shotChargeProgress = 0.f;
	}
}
