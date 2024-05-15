#pragma once

namespace offsets {
	namespace b2802 {
		DWORD ReplayInterface_ptr = 0x1D76838;
		DWORD world_ptr = 0x254D448;
		DWORD viewport_ptr = 0x1FBC100;
	}
	namespace b2699 {
		DWORD ReplayInterface_ptr = 0x20304C8;
		DWORD world_ptr = 0x26684D8;
		DWORD viewport_ptr = 0x20D8C90;
		DWORD Camera_ptr = 0x20D9868;
	}
	namespace b2612 {
		DWORD ReplayInterface_ptr = 0x1F77EF0;
		DWORD world_ptr = 0x2567DB0;
		DWORD viewport_ptr = 0x1FD8570;
	}
	namespace b2545 {
		DWORD ReplayInterface_ptr = 0x1F2E7A8;
		DWORD world_ptr = 0x25667E8;
		DWORD viewport_ptr = 0x1FD6F70;
	}
	namespace b2372 {
		DWORD ReplayInterface_ptr = 0x1F05208;
		DWORD world_ptr = 0x252DCD8;
		DWORD viewport_ptr = 0x1F9E9F0;
		DWORD i_camera_ptr = 0x1FD8570;
	}
	namespace b2189 {
		DWORD ReplayInterface_ptr = 0x1EE18A8;
		DWORD world_ptr = 0x24E6D90;
		DWORD viewport_ptr = 0x1F888C0;
	}
	namespace b2060 {
		DWORD ReplayInterface_ptr = 0x1EC3828;
		DWORD world_ptr = 0x24C8858;
		DWORD viewport_ptr = 0x1F6A7E0;
	}
	namespace b1604 {
		DWORD ReplayInterface_ptr = 0x1EFD4C8;
		DWORD world_ptr = 0x247F840;
		DWORD viewport_ptr = 0x2087780;
	}
	namespace b3095 {
		DWORD ReplayInterface_ptr = 0x1F58B58;
		DWORD world_ptr = 0x2593320;
		DWORD viewport_ptr = 0x20019E0;
	}
}

inline static const Vector2 Screen = {
		static_cast<float>(GetSystemMetrics(SM_CXSCREEN)),
		static_cast<float>(GetSystemMetrics(SM_CYSCREEN))
};

inline static const Vector2 ScreenCenter = {
	Screen.x / 2.f,
	Screen.y / 2.f
};

Vector2 WorldToScreen(Vector3 WorldPos, std::uint64_t ViewPort)
{
	D3DXMATRIX matrix = device_t.read<D3DXMATRIX>(ViewPort + 0x24C);

	D3DXVECTOR3 screenPosition;

	D3DXMatrixTranspose(&matrix, &matrix);
	D3DXVECTOR4 vectorX = D3DXVECTOR4(matrix._21, matrix._22, matrix._23, matrix._24);
	D3DXVECTOR4 vectorY = D3DXVECTOR4(matrix._31, matrix._32, matrix._33, matrix._34);
	D3DXVECTOR4 vectorZ = D3DXVECTOR4(matrix._41, matrix._42, matrix._43, matrix._44);

	screenPosition.x = (vectorX.x * WorldPos.x) + (vectorX.y * WorldPos.y) + (vectorX.z * WorldPos.z) + vectorX.w;
	screenPosition.y = (vectorY.x * WorldPos.x) + (vectorY.y * WorldPos.y) + (vectorY.z * WorldPos.z) + vectorY.w;
	screenPosition.z = (vectorZ.x * WorldPos.x) + (vectorZ.y * WorldPos.y) + (vectorZ.z * WorldPos.z) + vectorZ.w;
	if (screenPosition.z <= 0.01f)
		return Vector2(0, 0);

	screenPosition.z = 1.0f / screenPosition.z;
	screenPosition.x *= screenPosition.z;
	screenPosition.y *= screenPosition.z;

	float xTmp = ScreenCenter.x;
	float yTmp = ScreenCenter.y;

	screenPosition.x += xTmp + (int)(0.5f * screenPosition.x * Screen.x + 0.5f);
	screenPosition.y = yTmp - (int)(0.5f * screenPosition.y * Screen.y + 0.5f);

	return Vector2(screenPosition.x, screenPosition.y);
}

Vector3 GetBonePos(std::uint64_t Ped, int BoneID) {
	D3DXMATRIX Matrix = device_t.read<D3DXMATRIX>(Ped + 0x60);
	D3DXVECTOR3 BonePos = device_t.read<D3DXVECTOR3>(Ped + 0x430 + (BoneID * 0x10));

	D3DXVECTOR4 Transformed{};
	D3DXVec3Transform(&Transformed, &BonePos, &Matrix);

	return Vector3(Transformed.x, Transformed.y, Transformed.z);
}

Vector2 GetBonePos2D(std::uint64_t Ped, int BoneID, std::uint64_t ViewPort) {
	auto Pos = GetBonePos(Ped, BoneID);
	auto Pos2D = WorldToScreen(Pos, ViewPort);

	return Pos2D;
}
