#include <dependencies/includes.hpp>
#include <d3d9types.h>

inline static const float M_PI = 3.1415926535898f;
inline static const float M_RAD = float(M_PI) / 180.f;

inline static Vector2 Screen = {
		static_cast<float>(GetSystemMetrics(SM_CXSCREEN)),
		static_cast<float>(GetSystemMetrics(SM_CYSCREEN))
};
inline static Vector2 ScreenCenter = {
	Screen.x / 2.f,
	Screen.y / 2.f
};

struct FQuat { double x, y, z, w; };
struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];
	D3DMATRIX to_matrix_with_scale()
	{
		D3DMATRIX m{};
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;
		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;
		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;
		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;
		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;
		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;
		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;
		return m;
	}
};

D3DMATRIX matrix_multiplication(D3DMATRIX pm1, D3DMATRIX pm2)
{
	D3DMATRIX pout{};
	pout._11 = pm1._11 * pm2._11 + pm1._12 * pm2._21 + pm1._13 * pm2._31 + pm1._14 * pm2._41;
	pout._12 = pm1._11 * pm2._12 + pm1._12 * pm2._22 + pm1._13 * pm2._32 + pm1._14 * pm2._42;
	pout._13 = pm1._11 * pm2._13 + pm1._12 * pm2._23 + pm1._13 * pm2._33 + pm1._14 * pm2._43;
	pout._14 = pm1._11 * pm2._14 + pm1._12 * pm2._24 + pm1._13 * pm2._34 + pm1._14 * pm2._44;
	pout._21 = pm1._21 * pm2._11 + pm1._22 * pm2._21 + pm1._23 * pm2._31 + pm1._24 * pm2._41;
	pout._22 = pm1._21 * pm2._12 + pm1._22 * pm2._22 + pm1._23 * pm2._32 + pm1._24 * pm2._42;
	pout._23 = pm1._21 * pm2._13 + pm1._22 * pm2._23 + pm1._23 * pm2._33 + pm1._24 * pm2._43;
	pout._24 = pm1._21 * pm2._14 + pm1._22 * pm2._24 + pm1._23 * pm2._34 + pm1._24 * pm2._44;
	pout._31 = pm1._31 * pm2._11 + pm1._32 * pm2._21 + pm1._33 * pm2._31 + pm1._34 * pm2._41;
	pout._32 = pm1._31 * pm2._12 + pm1._32 * pm2._22 + pm1._33 * pm2._32 + pm1._34 * pm2._42;
	pout._33 = pm1._31 * pm2._13 + pm1._32 * pm2._23 + pm1._33 * pm2._33 + pm1._34 * pm2._43;
	pout._34 = pm1._31 * pm2._14 + pm1._32 * pm2._24 + pm1._33 * pm2._34 + pm1._34 * pm2._44;
	pout._41 = pm1._41 * pm2._11 + pm1._42 * pm2._21 + pm1._43 * pm2._31 + pm1._44 * pm2._41;
	pout._42 = pm1._41 * pm2._12 + pm1._42 * pm2._22 + pm1._43 * pm2._32 + pm1._44 * pm2._42;
	pout._43 = pm1._41 * pm2._13 + pm1._42 * pm2._23 + pm1._43 * pm2._33 + pm1._44 * pm2._43;
	pout._44 = pm1._41 * pm2._14 + pm1._42 * pm2._24 + pm1._43 * pm2._34 + pm1._44 * pm2._44;
	return pout;
}

Vector3 GetBonePosition(std::uint64_t Mesh, int ID)
{
	uintptr_t bone_array = device_t.read<uintptr_t>(Mesh + 0x5B0);
	if (bone_array == 0) 
		bone_array = device_t.read<uintptr_t>(Mesh + 0x5B0 + 0x10);

	FTransform bone = device_t.read<FTransform>(bone_array + (ID * 0x60));

	FTransform component_to_world = device_t.read<FTransform>(Mesh + 0x1c0);

	D3DMATRIX matrix = matrix_multiplication(bone.to_matrix_with_scale(), component_to_world.to_matrix_with_scale());
	
	return Vector3(matrix._41, matrix._42, matrix._43);
}

struct Camera
{
	Vector3 Location;
	Vector3 Rotation;
	double FOV;
};


D3DMATRIX Matrix(Vector3 rot)
{
	float Pitch = rot.x * M_RAD;
	float Yaw = rot.y * M_RAD;
	float Roll = rot.z * M_RAD;

	float SP = sinf(Pitch);
	float CP = cosf(Pitch);
	float SY = sinf(Yaw);
	float CY = cosf(Yaw);
	float SR = sinf(Roll);
	float CR = cosf(Roll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = 0.f;
	matrix.m[3][1] = 0.f;
	matrix.m[3][2] = 0.f;
	matrix.m[3][3] = 1.f;

	return matrix;
}

bool WorldToScreen(Vector3 WorldLocation, Camera LocalCamera, Vector2* OutPos)
{
	D3DMATRIX tempMatrix = Matrix(LocalCamera.Rotation);

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - LocalCamera.Location;
	Vector3 vTransformed = Vector3(Vector3::Dot(vDelta, vAxisY), Vector3::Dot(vDelta, vAxisZ), Vector3::Dot(vDelta, vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	OutPos->x = ScreenCenter.x + vTransformed.x * (ScreenCenter.x / tanf(LocalCamera.FOV * (float)M_PI / 360.f)) / vTransformed.z;
	OutPos->y = ScreenCenter.y - vTransformed.y * (ScreenCenter.x / tanf(LocalCamera.FOV * (float)M_PI / 360.f)) / vTransformed.z;

	return true;
}


int main( )
{
	if (!device_t.start_service()) {
		m_log("[-] Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("FortniteClient-Win64-Shipping.exe");
	if (!device_t.m_pid || !device_t.is_mapped(device_t.m_pid)) {
		m_log("[-] Target not mapped\n");
		return 0;
	}

	if (!device_t.resolve_dtb()) {
		m_log("[-] failed to get dtb\n");
		return FALSE;
	}

	device_t.m_base = device_t.get_module_base(0);
	if (!device_t.m_base) {
		m_log("[-] Failed to get base\n");
		return 0;
	}
	m_log("[+] Base: %p\n", reinterpret_cast<void*>(device_t.m_base));
	
	static int aim_bone = 110;

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(3));

		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			aim_bone+=1;
			std::cout << aim_bone << std::endl;
		}

		const auto Uworld = device_t.read<std::uint64_t>(device_t.m_base + 0x126CF528);

		const auto LocationPtr = device_t.read<std::uint64_t>(Uworld + 0x110);

		const auto GameInstance = device_t.read<std::uint64_t>(Uworld + 0x1D8);

		const auto PersistentLevel = device_t.read<std::uint64_t>(Uworld + 0x30);

		const auto GameState = device_t.read<std::uint64_t>(Uworld + 0x160);

		const auto LocalPlayers = device_t.read<std::uint64_t>(GameInstance + 0x38);

		const auto LocalPlayer = device_t.read<std::uint64_t>(LocalPlayers);

		const auto PlayerController = device_t.read<std::uint64_t>(LocalPlayer + 0x30);

		const auto PlayerCameraManager = device_t.read<std::uint64_t>(PlayerController + 0x348);

		const auto AcknowledgedPawn = device_t.read<std::uint64_t>(PlayerController + 0x338);

		const auto LocalPlayerState = device_t.read<std::uint64_t>(AcknowledgedPawn + 0x2b0);

		const auto LocalTeamIndex = device_t.read<char>(LocalPlayerState + 0x1211);

		const auto LocalPosition = device_t.read<Vector3>(LocationPtr);

		const auto CurrentVehicle = device_t.read<std::uint64_t>(AcknowledgedPawn + 0x29a0); 
		
		const auto CurrentWeapon = device_t.read<std::uint64_t>(AcknowledgedPawn + 0xa68);

		//if (CurrentVehicle && GetAsyncKeyState(VK_SHIFT))
		//{
		//	device_t.write<bool>(CurrentVehicle + 0x7c2, false); // bUseGravity : 1	
		//}
		//else {
		//	device_t.write<bool>(CurrentVehicle + 0x7c2, true); // bUseGravity : 1	
		//}

		//device_t.write<float>(PlayerController + 0x68, -1);
		//device_t.write<float>(CurrentWeapon + 0x68, FLT_MAX);

		const auto PlayerArray = device_t.read<std::uint64_t>(GameState + 0x2A8);

		const auto PlayerCount = device_t.read<std::int32_t>(GameState + 0x2A8 + sizeof(std::uint64_t));

		Vector3 rotation;

		auto rotation_pointer = device_t.read<uintptr_t>(Uworld + 0x120);

		struct FNRotation
		{
			double a; //0x0000
			char pad_0008[24]; //0x0008
			double b; //0x0020
			char pad_0028[424]; //0x0028
			double c; //0x01D0
		}tpmrotation;

		tpmrotation.a = device_t.read<double>(rotation_pointer);
		tpmrotation.b = device_t.read<double>(rotation_pointer + 0x20);
		tpmrotation.c = device_t.read<double>(rotation_pointer + 0x1d0);

		rotation.x = asin(tpmrotation.c) * (180.0 / 3.14159265358979323846);
		rotation.y = ((atan2(tpmrotation.a * -1, tpmrotation.b) * (180.0 / 3.14159265358979323846)) * -1) * -1;
		rotation.z = 0;

		Camera LocalCam;
		LocalCam.FOV = device_t.read<float>(PlayerController + 0x394) * 90.f;
		LocalCam.Location = device_t.read<Vector3>(LocationPtr);
		LocalCam.Rotation = rotation;

		float ClosestDistance = FLT_MAX;
		Vector3 ClosestPos = Vector3();

		for (auto i = 0; i < PlayerCount; i++)
		{
			const auto PlayerState = device_t.read<std::uint64_t>(PlayerArray + (i * sizeof(std::uint64_t)));
			if (!PlayerState) continue;

			const auto TeamIndex = device_t.read<char>(PlayerState + 0x1211);
			if (TeamIndex == LocalTeamIndex) continue;

			const auto PawnPrivate = device_t.read<std::uint64_t>(PlayerState + 0x308);
			if (AcknowledgedPawn == PawnPrivate) continue;

			const auto RootComponent = device_t.read<std::uint64_t>(PawnPrivate + 0x198);
			if (!RootComponent) continue;

			const auto Mesh = device_t.read<std::uint64_t>(PawnPrivate + 0x318);
			if (!Mesh) continue;

			const auto Position = GetBonePosition(Mesh, aim_bone);

			Vector2 ScreenPos{};
			WorldToScreen(Position, LocalCam, &ScreenPos);

			const auto Distance = Vector2::Distance(ScreenPos, ScreenCenter);

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestPos = Position;
			}
		}

		struct viewInfo {
			float pitchMin, pitchMax;
			float yawMin, yawMax;
		} v;

		v.pitchMin = -89.9999f;
		v.pitchMax = 89.9999f;
		v.yawMin = 0.0000f;
		v.yawMax = 359.9999f;

		device_t.write<viewInfo>(PlayerCameraManager + 0x248c, v);

		if (GetAsyncKeyState(VK_RBUTTON) && ClosestDistance != FLT_MAX)
		{
			Vector3 VectorPos = ClosestPos - LocalPosition;

			float distance = sqrtf(VectorPos.x * VectorPos.x + VectorPos.y * VectorPos.y + VectorPos.z * VectorPos.z);

			v.pitchMin = v.pitchMax = -((acosf(VectorPos.z / distance) * (float)(180.0f / M_PI)) - 90.f);
			v.yawMin = v.yawMax = atan2f(VectorPos.y, VectorPos.x) * (float)(180.0f / M_PI);

			device_t.write<viewInfo>(PlayerCameraManager + 0x248c, v);
		}
	}
}