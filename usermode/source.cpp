#include <dependencies/includes.hpp>

#include "cheat/settings.hpp"
#include "cheat/sdk.hpp"
#include "cheat/visual.hpp"
#include "cheat/aim.hpp"

Vector2 GetClosestBone(std::uint64_t Ped, std::uint64_t ViewPort)
{
	float ClosestDistance = FLT_MAX;
	Vector2 ClosestPos;

	for (int BoneIndex = static_cast<int>(AimBones::Head);
		BoneIndex <= static_cast<int>(AimBones::LLeg);
		++BoneIndex) {
		int CurrentBone = static_cast<int>(BoneIndex);

		Vector2 Pos = GetBonePos2D(Ped, CurrentBone, ViewPort);
		float Distance = Vector2::Distance(Pos, ScreenCenter);

		if (Distance < ClosestDistance) {
			ClosestDistance = Distance;
			ClosestPos = Pos;
		}
	}
	return ClosestPos;
}

int main( )
{
	if (!device_t.start_service()) {
		m_log("[-] Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("FiveM_b2699_GTAProcess.exe");
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
	
	const std::uint64_t World = device_t.read<std::uint64_t>(device_t.m_base + offsets::b2699::world_ptr);
	m_log("[+] World: %p\n", reinterpret_cast<void*>(World));

	const std::uint64_t ViewPort = device_t.read<std::uint64_t>(device_t.m_base + offsets::b2699::viewport_ptr);
	m_log("[+] ViewPort: %p\n", reinterpret_cast<void*>(ViewPort));

	const std::uint64_t ReplayInterface = device_t.read<std::uint64_t>(device_t.m_base + offsets::b2699::ReplayInterface_ptr);
	m_log("[+] ReplayInterface: %p\n", reinterpret_cast<void*>(ReplayInterface));

	const std::uint64_t Camera = device_t.read<std::uint64_t>(device_t.m_base + offsets::b2699::Camera_ptr);
	m_log("[+] Camera: %p\n", reinterpret_cast<void*>(Camera));

	const std::uint64_t PedInterface = device_t.read<std::uint64_t>(ReplayInterface + 0x18);
	m_log("[+] PedInterface: %p\n", reinterpret_cast<void*>(PedInterface));

	const std::uint64_t LocalPlayer = device_t.read<std::uint64_t>(World + 0x8);
	m_log("[+] LocalPlayer: %p\n", reinterpret_cast<void*>(LocalPlayer));

	const std::uint64_t PedList = device_t.read<std::uint64_t>(PedInterface + 0x100);
	m_log("[+] PedList: %p\n", reinterpret_cast<void*>(PedList));

	const Vector3 LocalPos = device_t.read<Vector3>(LocalPlayer + 0x90);
		
	Overlay::game_window = FindWindow("grcWindow", 0);
	Overlay::shouldRun = true;

	Overlay::CreateOverlay();
	Overlay::CreateDevice();
	Overlay::CreateImGui();

	Overlay::SetForeground(Overlay::game_window);

	while (Overlay::shouldRun)
	{
		Overlay::StartRender();
		DL = ImGui::GetBackgroundDrawList();

		float ClosestDistance = FLT_MAX;
		Vector2 ClosestPosition2D = Vector2();
		Vector3 ClosestPosition = Vector3();
		
		for (int i = 0; i < 256; i++)
		{
			auto Ped = device_t.read<std::uint64_t>(PedList + (i * 0x10));
			if (!Ped || Ped == LocalPlayer) continue;

			if (device_t.read<float>(Ped + 0x280) <= 0.f) continue;

			auto Type = device_t.read<int>(Ped + 0x10b8) << 11 >> 25;
			if (Type != 2 && OnlyPlayer) continue;


			if (DrawBox) Box(Ped, ViewPort, Filled);
			if (DrawSkeleton) Skeleton(Ped, ViewPort);

			AimBones Bone = AimBones::Head;

			switch (AimBone)
			{
			case 0: Bone = AimBones::Head; break;
			case 1: Bone = AimBones::Neck; break;
			case 2: Bone = AimBones::Chest; break;
			case 3: Bone = AimBones::LLeg; break;
			case 4: Bone = AimBones::RLeg; break;
			case 5: Bone = AimBones::Closest; break;
			}

			Vector2 Pos2D{};
			Vector3 Pos{};

			if (Bone != AimBones::Closest)
			{
				Pos2D = GetBonePos2D(Ped, (int)Bone, ViewPort);
				Pos = GetBonePos(Ped, (int)Bone);
			}
			else
			{
				Pos2D = GetClosestBone(Ped, ViewPort);
				Pos = GetBonePos(Ped, (int)Bone);
			}

			const auto Distance2D = Vector2::Distance(ScreenCenter, Pos2D);
			const auto Distance = Vector3::Distance(LocalPos, Pos);

			if (Distance2D < ClosestDistance && Distance2D < FOV) {
				ClosestDistance = Distance2D;
				ClosestPosition2D = Pos2D;
				ClosestPosition = Pos;
			}
		}

		if (EnableAimbot && !ShowMenu && GetAsyncKeyState(aimKey))
		{
			AimAt(ClosestPosition2D);
			//MemAim(ClosestPosition, Camera);
		}

		if (EnableAimbot && !ShowMenu)
			ImGui::GetBackgroundDrawList()->AddCircle(ScreenCenter.ToImVec2(), FOV, ImColor(30, 30, 30, 200), 60, 2.f);

		if (ShowMenu) Overlay::Render();

		Overlay::EndRender();
	}

	Overlay::DestroyImGui();
	Overlay::DestroyDevice();
	Overlay::DestroyOverlay();

}