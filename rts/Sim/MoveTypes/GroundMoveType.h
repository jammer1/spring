/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef GROUNDMOVETYPE_H
#define GROUNDMOVETYPE_H

#include <array>

#include "MoveType.h"
#include "Sim/Path/IPathController.hpp"
#include "System/Sync/SyncedFloat3.h"

struct UnitDef;
struct MoveDef;
class CSolidObject;

class CGroundMoveType : public AMoveType
{
	CR_DECLARE_DERIVED(CGroundMoveType)

public:
	CGroundMoveType(CUnit* owner);
	~CGroundMoveType();

	struct MemberData {
		std::array<std::pair<unsigned int,  bool*>, 3>  bools;
		std::array<std::pair<unsigned int, short*>, 1> shorts;
		std::array<std::pair<unsigned int, float*>, 9> floats;
	};

	void PostLoad();

	bool Update() override;
	void SlowUpdate() override;

	void StartMovingRaw(const float3 moveGoalPos, float moveGoalRadius) override;
	void StartMoving(float3 pos, float moveGoalRadius) override;
	void StartMoving(float3 pos, float moveGoalRadius, float speed) override { StartMoving(pos, moveGoalRadius); }
	void StopMoving(bool callScript = false, bool hardStop = false, bool cancelRaw = false) override;
	bool IsMovingTowards(const float3& pos, float radius, bool checkProgress) const override {
		return (goalPos == pos * XZVector && goalRadius == radius && (!checkProgress || progressState == Active));
	}

	void KeepPointingTo(float3 pos, float distance, bool aggressive) override;
	void KeepPointingTo(CUnit* unit, float distance, bool aggressive) override;

	void TestNewTerrainSquare();
	bool CanApplyImpulse(const float3&) override;
	void LeaveTransport() override;

	void InitMemberPtrs(MemberData* memberData);
	bool SetMemberValue(unsigned int memberHash, void* memberValue) override;

	bool OnSlope(float minSlideTolerance);
	bool IsReversing() const override { return reversing; }
	bool IsPushResistant() const override { return pushResistant; }
	bool WantToStop() const { return (pathID == 0 && !useRawMovement); }


	float GetTurnRate() const { return turnRate; }
	float GetTurnSpeed() const { return turnSpeed; }
	float GetTurnAccel() const { return turnAccel; }

	float GetAccRate() const { return accRate; }
	float GetDecRate() const { return decRate; }
	float GetMyGravity() const { return myGravity; }

	float GetMaxReverseSpeed() const { return maxReverseSpeed; }
	float GetWantedSpeed() const { return wantedSpeed; }
	float GetCurrentSpeed() const { return currentSpeed; }
	float GetDeltaSpeed() const { return deltaSpeed; }

	float GetCurrWayPointDist() const { return currWayPointDist; }
	float GetPrevWayPointDist() const { return prevWayPointDist; }
	float GetGoalRadius(float s = 0.0f) const override { return (goalRadius + extraRadius * s); }

	unsigned int GetPathID() const { return pathID; }

	const SyncedFloat3& GetCurrWayPoint() const { return currWayPoint; }
	const SyncedFloat3& GetNextWayPoint() const { return nextWayPoint; }

private:
	float3 GetObstacleAvoidanceDir(const float3& desiredDir);
	float3 GetNewSpeedVector(const float hAcc, const float vAcc) const;

	#define SQUARE(x) ((x) * (x))
	bool StartSkidding(const float3& vel, const float3& dir) const { return ((SQUARE(vel.dot(dir)) + 0.01f) < (vel.SqLength() * sqSkidSpeedMult)); }
	bool StopSkidding(const float3& vel, const float3& dir) const { return ((SQUARE(vel.dot(dir)) + 0.01f) >= (vel.SqLength() * sqSkidSpeedMult)); }
	bool StartFlying(const float3& vel, const float3& dir) const { return (vel.dot(dir) > 0.2f); }
	bool StopFlying(const float3& vel, const float3& dir) const { return (vel.dot(dir) <= 0.2f); }
	#undef SQUARE

	float Distance2D(CSolidObject* object1, CSolidObject* object2, float marginal = 0.0f);

	unsigned int GetNewPath();

	void GetNextWayPoint();
	bool CanGetNextWayPoint();
	void ReRequestPath(bool forceRequest);

	float3 Here();

	void StartEngine(bool callScript);
	void StopEngine(bool callScript, bool hardStop = false);

	void Arrived(bool callScript);
	void Fail(bool callScript);

	void HandleObjectCollisions();
	void HandleStaticObjectCollision(
		CUnit* collider,
		CSolidObject* collidee,
		const MoveDef* colliderMD,
		const float colliderRadius,
		const float collideeRadius,
		const float3& separationVector,
		bool canRequestPath,
		bool checkYardMap,
		bool checkTerrain);

	void HandleUnitCollisions(
		CUnit* collider,
		const float colliderSpeed,
		const float colliderRadius,
		const UnitDef* colliderUD,
		const MoveDef* colliderMD);
	void HandleFeatureCollisions(
		CUnit* collider,
		const float colliderSpeed,
		const float colliderRadius,
		const UnitDef* colliderUD,
		const MoveDef* colliderMD);

	void SetMainHeading();
	void ChangeSpeed(float, bool, bool = false);
	void ChangeHeading(short newHeading);

	void UpdateSkid();
	void UpdateControlledDrop();
	void CheckCollisionSkid();
	void CalcSkidRot();

	const float3& GetGroundNormal(const float3&) const;
	float GetGroundHeight(const float3&) const;
	void AdjustPosToWaterLine();
	bool UpdateDirectControl();
	void UpdateOwnerSpeedAndHeading();
	void UpdateOwnerPos(const float3&, const float3&);
	bool OwnerMoved(const short, const float3&, const float3&);
	bool FollowPath();
	bool WantReverse(const float3& wpDir, const float3& ffDir) const;

private:
	GMTDefaultPathController pathController;

	SyncedFloat3 currWayPoint;
	SyncedFloat3 nextWayPoint;

	float3 waypointDir;
	float3 flatFrontDir;
	float3 lastAvoidanceDir;
	float3 mainHeadingPos;
	float3 skidRotVector;               /// vector orthogonal to skidDir

	float turnRate;                     /// maximum angular speed (angular units/frame)
	float turnSpeed;                    /// current angular speed (angular units/frame)
	float turnAccel;                    /// angular acceleration (angular units/frame^2)

	float accRate;
	float decRate;
	float myGravity;

	float maxReverseDist;
	float minReverseAngle;
	float maxReverseSpeed;
	float sqSkidSpeedMult;

	float wantedSpeed;
	float currentSpeed;
	float deltaSpeed;

	bool atGoal;
	bool atEndOfPath;
	bool wantRepath;

	float currWayPointDist;
	float prevWayPointDist;

	float goalRadius;                   /// original radius passed to StartMoving*
	float extraRadius;                  /// owner MoveDef footprint radius

	bool reversing;
	bool idling;
	bool pushResistant;
	bool canReverse;
	bool useMainHeading;                /// if true, turn toward mainHeadingPos until weapons[0] can TryTarget() it
	bool useRawMovement;                /// if true, move towards goal without invoking PFS

	float skidRotSpeed;                 /// rotational speed when skidding (radians / (GAME_SPEED frames))
	float skidRotAccel;                 /// rotational acceleration when skidding (radians / (GAME_SPEED frames^2))

	unsigned int pathID;
	unsigned int nextObstacleAvoidanceFrame;

	unsigned int numIdlingUpdates;      /// {in, de}creased every Update if idling is true/false and pathId != 0
	unsigned int numIdlingSlowUpdates;  /// {in, de}creased every SlowUpdate if idling is true/false and pathId != 0

	short wantedHeading;
	short minScriptChangeHeading;       /// minimum required turn-angle before script->ChangeHeading is called
};

#endif // GROUNDMOVETYPE_H

