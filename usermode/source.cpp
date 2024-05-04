#include <dependencies/includes.hpp>

#define GWORLD 0x43795D8

namespace offsets 
{
	// Engine.World
	uintptr_t OwningGameInstance = 0x0188;
	uintptr_t GameState = 0x0130;
	uintptr_t PersistentLevel = 0x0030;

	// Engine.GameInstance
	uintptr_t LocalPlayers = 0x0038;

	// Engine.Player
	uintptr_t PlayerController = 0x0030;

	// Engine.Controller
	uintptr_t PlayerState = 0x0228;
	uintptr_t Character = 0x0260;
	uintptr_t TransformComponent = 0x0268;

	// Engine.PlayerController
	uintptr_t PlayerCameraManager = 0x02B8;
	uintptr_t TargetViewRotation = 0x02CC;

	// Engine.PlayerCameraManager
	uintptr_t CameraCachePrivate = 0x1A60;

	// Engine.CameraCacheEntry
	uintptr_t POV = 0x0010;

	// Engine.GameStateBase
	uintptr_t PlayerArray = 0x0238;

	// Engine.PlayerState
	uintptr_t PawnPrivate = 0x0280;

	// Engine.Pawn
	uintptr_t Controller = 0x0258;

	// Engine.SceneComponent
	uintptr_t RelativeLocation = 0x011C;
	uintptr_t RelativeRotation = 0x0128;
	uintptr_t RelativeScale3D = 0x0134;

	// Engine.Character
	uintptr_t Mesh = 0x0280;

	// Engine.SkeletalMeshComponent
	uintptr_t CachedBoneSpaceTransforms = 0x06D0;
	uintptr_t CachedComponentSpaceTransforms = 0x06E0;

}

struct TArray
{
	std::uintptr_t Array;
	std::uint32_t Count;
	std::uint32_t MaxCount;

	template<typename T> T Get(std::uint32_t Index)
	{
		return device_t.read<T>(Array + (Index * 0x8));
	}
};

struct FMinimalViewInfo final
{
public:
	struct FVector                                Location;                                          // 0x0000(0x000C)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FRotator                               Rotation;                                          // 0x000C(0x000C)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	float                                         FOV;                                               // 0x0018(0x0004)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FTransform final
{
public:
	struct FQuat                                  Rotation;                                          // 0x0000(0x0010)(Edit, BlueprintVisible, SaveGame, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	struct FVector                                Translation;                                       // 0x0010(0x000C)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8                                         Pad_16[0x4];                                       // 0x001C(0x0004)(Fixing Size After Last Property [ Dumper-7 ])
	struct FVector                                Scale3D;                                           // 0x0020(0x000C)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8                                         Pad_17[0x4];                                       // 0x002C(0x0004)(Fixing Struct Size After Last Property [ Dumper-7 ])
};

int main( )
{
	if (!device_t.start_service()) {
		m_log("Driver not loaded\n");
		return 0;
	}

	device_t.m_pid = device_t.get_process_id("HarshDoorstop-Win64-Shipping.exe");
	if (!device_t.m_pid || !device_t.is_mapped(device_t.m_pid)) {
		m_log("Target not mapped\n");
		return 0;
	}

	device_t.m_base = device_t.get_module_base(0);
	if (!device_t.m_base) {
		m_log("[-] Failed to get image_base\n");
		return 0;
	}

	if (!device_t.resolve_dtb()) {
		m_log("[-] failed to get dtb\n");
		return FALSE;
	}
	
	m_log("[+] Image base: %p\n", reinterpret_cast<void*>(device_t.m_base));
	
	const auto gworld = device_t.read<uintptr_t>(device_t.m_base + GWORLD);
	if (!gworld) {
		m_log("[-] Failed to get gworld\n");
		return 0;
	}
	
	m_log("[+] Gworld: %p\n", reinterpret_cast<void*>(gworld));
	
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	const auto game_instance = device_t.read<uintptr_t>(gworld + offsets::OwningGameInstance);
	//m_log("[+] game_instance: %p\n", reinterpret_cast<void*>(game_instance));

	const auto game_state = device_t.read<uintptr_t>(gworld + offsets::GameState);
	//m_log("[+] game_state: %p\n", reinterpret_cast<void*>(game_state));

	const auto persistentlevel = device_t.read<uintptr_t>(gworld + offsets::PersistentLevel);
	//m_log("[+] persistentlevel: %p\n", reinterpret_cast<void*>(persistentlevel));

	const auto local_players = device_t.read<uintptr_t>(game_instance + offsets::LocalPlayers);
	//m_log("[+] local_players: %p\n", reinterpret_cast<void*>(local_players));

	const auto local_player = device_t.read<uintptr_t>(local_players);
	//m_log("[+] local_player: %p\n", reinterpret_cast<void*>(local_player));

	const auto local_controller = device_t.read<uintptr_t>(local_player + offsets::PlayerController);
	//m_log("[+] local_controller: %p\n", reinterpret_cast<void*>(local_controller));

	const auto local_controller_state = device_t.read<uintptr_t>(local_controller + offsets::PlayerState);
	//m_log("[+] local_controller_state: %p\n", reinterpret_cast<void*>(local_controller_state));

	const auto local_transform_component = device_t.read<uintptr_t>(local_controller + offsets::TransformComponent);
	//m_log("[+] local_transform_component: %p\n", reinterpret_cast<void*>(local_transform_component));

	const auto local_location = device_t.read<FVector>(local_transform_component + offsets::RelativeLocation);
	//m_log("[+] local_location: %f, %f, %f\n", local_location.X, local_location.Y, local_location.Z);

	const auto local_camera_manager = device_t.read<uintptr_t>(local_controller + offsets::PlayerCameraManager);
	//m_log("[+] local_camera_manager: %p\n", reinterpret_cast<void*>(local_camera_manager));

	const auto local_camera_pov = device_t.read<FMinimalViewInfo>(local_camera_manager + offsets::CameraCachePrivate + offsets::POV);
	//m_log("[+] local_camera_pov: %f, %f, %f\n", local_camera_pov.Location.X, local_camera_pov.Location.Y, local_camera_pov.Location.Z);

	auto player_array = device_t.read<TArray>(game_state + offsets::PlayerArray);
	//m_log("[+] player_array: %p\n", reinterpret_cast<void*>(player_array.Array));
	//m_log("[+] player_array_count: %i\n", player_array.Count);
	
	for (int i = 0; i < player_array.Count; i++) {
		if (!player_array.MaxCount) {
			continue;
		}
	
		const auto current_controller_state = player_array.Get<std::uintptr_t>(i);
	
		const auto current_pawn = device_t.read<uintptr_t>(current_controller_state + offsets::PawnPrivate);
	
		const auto current_controller = device_t.read<uintptr_t>(current_pawn + offsets::Controller);
	
		const auto current_character = device_t.read<uintptr_t>(current_controller + offsets::Character);
		m_log("[+] current_character: %p\n", reinterpret_cast<void*>(current_character));

		const auto current_mesh = device_t.read<uintptr_t>(current_character + offsets::Mesh);
		m_log("[+] current_mesh: %p\n", reinterpret_cast<void*>(current_mesh));

		auto bone_array = device_t.read<TArray>(current_mesh + offsets::CachedBoneSpaceTransforms);
		for (int i = 0; i < bone_array.Count; i++) {
			if (!bone_array.MaxCount) {
				continue;
			}

			const auto current_bone = bone_array.Get<FTransform>(i);
			m_log("[+] current_bone: %f, %f, %f\n", 
				current_bone.Translation.X,
				current_bone.Translation.Y,
				current_bone.Translation.Z
			);

		}
	}

}