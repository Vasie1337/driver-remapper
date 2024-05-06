#include <dependencies/includes.hpp>

ViewMatrix Matrix;

int Width;
int Height;
Vector3 LocalPos;

#define FOV_MAX 50.f

struct Player {
	Vector2 ScreenPos;
	Vector3 WorldPos;
	float ScreenDistance;
	float WorldDistance;
};

Vector2 w2s(Vector3 position)
{
	Vector3 transform(Matrix.matrix[0][3], Matrix.matrix[1][3], Matrix.matrix[2][3]);
	Vector3 right(Matrix.matrix[0][0], Matrix.matrix[1][0], Matrix.matrix[2][0]);
	Vector3 up(Matrix.matrix[0][1], Matrix.matrix[1][1], Matrix.matrix[2][1]);

	float w = Vector3::Dot(transform, position) + Matrix.matrix[3][3];

	if (w < 1.f)
		w = 1.0f;

	float x = Vector3::Dot(right, position) + Matrix.matrix[3][0];
	float y = Vector3::Dot(up, position) + Matrix.matrix[3][1];

	return Vector2((Width / 2) * (1.f + x / w), (Height / 2) * (1.f - y / w));
}

void get_camera()
{
	const auto MainCamera = device_t.read<std::uint64_t>(device_t.m_base + 0x42E80D8);

	const auto StaticField = device_t.read<std::uint64_t>(MainCamera + 0xB8);

	Width = device_t.read<int>(StaticField + 0x8C);
	Height = device_t.read<int>(StaticField + 0x90);

	Matrix = device_t.read<ViewMatrix>(StaticField + 0xA8);
}

void aim_at(Vector2 ClosestPos)
{
	float TargetX = 0;
	float TargetY = 0;

	Vector2 screen_center = { Width / 2.f, Height / 2.f };

	if (ClosestPos.x != 0) {
		if (ClosestPos.x > screen_center.x) {
			TargetX = -(screen_center.x - ClosestPos.x);
			if (TargetX + screen_center.x > screen_center.x * 2)
				TargetX = 0;
		}

		if (ClosestPos.x < screen_center.x) {
			TargetX = ClosestPos.x - screen_center.x;
			if (TargetX + screen_center.x < 0)
				TargetX = 0;
		}
	}

	if (ClosestPos.y != 0) {
		if (ClosestPos.y > screen_center.y) {
			TargetY = -(screen_center.y - ClosestPos.y);
			if (TargetY + screen_center.y > screen_center.y * 2)
				TargetY = 0;
		}

		if (ClosestPos.y < screen_center.y) {
			TargetY = ClosestPos.y - screen_center.y;
			if (TargetY + screen_center.y < 0)
				TargetY = 0;
		}
	}

	TargetX /= 3.0f;
	TargetY /= 3.0f;

	mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(TargetX), static_cast<int>(TargetY), 0, 0);
}

void draw_player(Player player, float ClosestDistance)
{
	auto Color = ImColor(255, 255, 255, 100);

	if (player.WorldDistance == ClosestDistance)
		Color = ImColor(255, 100, 100, 200);

	ImGui::GetBackgroundDrawList()->AddCircleFilled(
		{ player.ScreenPos.x, player.ScreenPos.y },
		3.f,
		Color
	);
	ImGui::GetBackgroundDrawList()->AddLine(
		{ Width / 2.f, 0.f },
		{ player.ScreenPos.x, player.ScreenPos.y },
		Color
	);
}

int main( )
{
	if (!device_t.start_service()) {
		m_log("[-] Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("BattleBit.exe");
	if (!device_t.m_pid || !device_t.is_mapped(device_t.m_pid)) {
		m_log("[-] Target not mapped\n");
		return 0;
	}

	if (!device_t.resolve_dtb()) {
		m_log("[-] failed to get dtb\n");
		return FALSE;
	}

	//device_t.m_base = device_t.get_module_base(0);
	device_t.m_base = 0x1ce210b0000;
	if (!device_t.m_base) {
		m_log("[-] Failed to get GameAssembly\n");
		return 0;
	}
	m_log("[+] GameAssembly: %p\n", reinterpret_cast<void*>(device_t.m_base));
	
	Overlay::game_window = FindWindowA("UnityWndClass", NULL);
	Overlay::shouldRun = true;
	Overlay::RenderMenu = false;

	Overlay::CreateOverlay();
	Overlay::CreateDevice();
	Overlay::CreateImGui();

	Overlay::SetForeground(Overlay::game_window);

	while (Overlay::shouldRun)
	{
		Overlay::StartRender();

		Overlay::Render();

		get_camera();

		ImGui::GetBackgroundDrawList()->AddCircle({ Width / 2.f, Height / 2.f }, FOV_MAX, ImColor(255, 255, 255, 100));

		const auto PlayerNetwork = device_t.read<std::uint64_t>(device_t.m_base + 0x42E7B88);

		const auto StaticField = device_t.read<std::uint64_t>(PlayerNetwork + 0xB8);

		const auto FastList = device_t.read<std::uint64_t>(StaticField + 0x48);

		const auto FastListBuffer = device_t.read<std::uint64_t>(FastList + 0x10);
		const auto FastListSize = device_t.read<std::uint32_t>(FastList + 0x18);

		float ClosestDistance = FLT_MAX;
		float ClosestDistance3D = FLT_MAX;
		Vector2 ClosestPos = Vector2();

		std::vector<Player> Players;

		for (int i = 0; i < FastListSize; i++)
		{
			const auto PlayerAddress = device_t.read<std::uint64_t>(FastListBuffer + (0x20 + (i * 0x8)));
			
			const auto PlayerNetworkState = device_t.read<std::uint64_t>(PlayerAddress + 0x30);

			const auto Health = device_t.read<float>(PlayerNetworkState + 0x14);
			if (Health <= 0.f) continue;

			const auto Position = device_t.read<Vector3>(PlayerNetworkState + 0x128);

			const auto LocalPlayer = device_t.read<bool>(PlayerAddress + 0x1C);
			if (LocalPlayer)
			{
				LocalPos = Position;
				continue;
			}
			
			const auto IsFiendly = device_t.read<bool>(PlayerNetworkState + 0xC8);
			if (IsFiendly) continue;

			Player player;

			player.ScreenPos = w2s(Position);

			player.WorldDistance = (int)Vector3::Distance(LocalPos, Position);
			player.ScreenDistance = Vector2::Distance({Width / 2.f, Height / 2.f }, player.ScreenPos);

			Players.push_back(player);

			if (player.ScreenDistance < ClosestDistance && player.ScreenDistance < FOV_MAX)
			{
				ClosestDistance = player.ScreenDistance;
				ClosestPos = player.ScreenPos;
			}

			if (player.WorldDistance < ClosestDistance3D)
			{
				ClosestDistance3D = player.WorldDistance;
			}
		}

		if (GetAsyncKeyState(VK_RBUTTON))
			aim_at(ClosestPos);

		for (auto& player: Players)
		{
			draw_player(player, ClosestDistance3D);
		}

		Overlay::EndRender();
	}

	Overlay::DestroyImGui();
	Overlay::DestroyDevice();
	Overlay::DestroyOverlay();
}