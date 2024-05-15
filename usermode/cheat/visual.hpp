#pragma once

ImDrawList* DL;

void DrawName(std::uint64_t Ped, Vector2 minPos, Vector2 maxPos)
{
	const std::string Name = "unamed player";
	ImVec2 textPosition = ImVec2(
		(minPos.x + maxPos.x - ImGui::CalcTextSize(Name.c_str()).x) / 2.f,
		minPos.y - 20.f
	);

	ImGui::PushFontShadow(0xFF000000);
	DL->AddText(textPosition, BoxColor, Name.c_str());
	ImGui::PopFontShadow();
}

inline const char* GetWeaopnName(std::uint64_t hash)
{
	//removed xoring, cba to do properly.
	const char* dagger = ("Dagger");
	const char* bat = ("Bat");
	const char* bottle = ("Bottle");
	const char* crowbar = ("Crow Bar");
	const char* unarmed = ("None");
	const char* flashlight = ("Flash Light");
	const char* golfclub = ("Golf club");
	const char* hammer = ("Hammer");
	const char* hatchet = ("Hatchet");
	const char* knuckle = ("Knuckle");
	const char* knife = ("Knife");
	const char* machete = ("Machete");
	const char* switchblade = ("Switch Blade");
	const char* nightstick = ("Night Stick");
	const char* wrench = ("Wrench");
	const char* battleaxe = ("Battle Axe");
	const char* poolcue = ("Pool Cue");
	const char* pistol = ("Pistol");
	const char* pistolmk2 = ("Pistol MK2");
	const char* combatpistol = ("Combat Pistol");
	const char* appistol = ("AP Pistol");
	const char* stungun = ("Stungun");
	const char* pistol50 = ("Pistol 50");
	const char* snspistol = ("SNS PISTOL");
	const char* snspistolmk2 = ("SNS Pistol MK2");
	const char* heavypistol = ("Heavy Pistol");
	const char* vintagepistol = ("Vintage Pisol");
	const char* flaregun = ("Flare Gun");
	const char* marksmanpistol = ("marksmanpistol");
	const char* revolver = ("Revolver");
	const char* revolvermk2 = ("Revolver MK2");
	static auto doubleaction = ("Double Action");
	static auto microsmg = ("Micro Smg");
	static auto smg = ("Smg");
	static auto smgmk2 = ("Smg MK2");
	static auto assaultsmg = ("Assault Smg");
	static auto combatpdw = ("Combat PDW");
	static auto machinepistol = ("Machine Pistol");
	static auto minismg = ("Mini Smg");
	static auto pumpshotgun = ("Pump Shotgun");
	static auto pumpshotgun_mk2 = ("Pump Shotgun MK2");
	static auto sawnoffshotgun = ("Sawnoff Shotgun");
	static auto assaultshotgun = ("Sssault Shotgun");
	static auto bullpupshotgun = ("Bullpup Shotgun");
	static auto musket = ("Musket");
	static auto heavyshotgun = ("Heavy Shotgun");
	static auto dbshotgun = ("DB Shotgun");
	static auto autoshotgun = ("Auto Shotgun");
	static auto assaultrifle = ("Assault Rifle");
	static auto assaultrifle_mk2 = ("Assault Rifle MK2");
	static auto carbinerifle = ("Carbine Rifle");
	static auto carbinerifle_mk2 = ("Carbine Rifle MK2");
	static auto advancedrifle = ("Advanced Rifle");
	static auto specialcarbine = ("Special Carbine");
	static auto specialcarbine_mk2 = ("Special Carbine MK2");
	static auto bullpuprifle = ("Bullpup Rifle");
	static auto bullpuprifle_mk2 = ("Bullpup Rifle MK2");
	static auto compactrifle = ("Compact Rifle");
	static auto machine_gun = ("Machine Gun");
	static auto combatmg = ("Combat MG");
	static auto combatmg_mk2 = ("Combat MG MK2");
	static auto gusenberg = ("GUSENBERG");
	static auto sniperrifle = ("Sniper Rifle");
	static auto heavysniper = ("AWP");
	static auto heavysniper_mk2 = ("AWP MK2");
	static auto marksmanrifle = ("Marksman Rifle");
	static auto marksmanrifle_mk2 = ("Marksman Rifle MK2");
	static auto rpg = ("RPG");
	static auto grenadelauncher = ("Grenade Launcher");
	static auto grenadelauncher_smoke = ("Grenade Launcher Smoke");
	static auto minigun = ("MiniGun");
	static auto firework = ("FireWork");
	static auto railgun = ("RailGun");
	static auto hominglauncher = ("Homing Launcher");
	static auto compactlauncher = ("Compact Launcher");
	static auto grenade = ("Grenade");
	static auto bzgas = ("BZGAS");
	static auto smokegrenade = ("Smoke Grenade");
	static auto flare = ("Flare");
	static auto molotov = ("Molotov");
	static auto stickybomb = ("Sticky BOMB");
	static auto proxmine = ("Prox Mine");
	static auto snowball = ("SnowBall");
	static auto pipebomb = ("Pipe Bomb");
	static auto ball = ("Ball");
	static auto petrolcan = ("Petrol Can");
	static auto fireextinguisher = ("Fire Extinguisher");
	static auto parachute = ("Parachute");

	static auto weapon_militaryrifle = ("Military Rifle");
	static auto weapon_heavyrifle = ("Heavy Rifle");
	static auto weapon_tacticalrifle = ("Tactical Rifle");

	static auto weapon_precisionrifle = ("Precision Rifle");

	static auto weapon_emplauncher = ("Compact EMP Launcher");
	static auto weapon_rayminigun = ("RayMiniGun");

	switch (hash)
	{
	case 0x92A27487:
		return dagger; break;
	case 0x958A4A8F:
		return bat; break;
	case 0xF9E6AA4B:
		return bottle; break;
	case 0x84BD7BFD:
		return crowbar; break;
	case 0xA2719263:
		return unarmed; break;
	case 0x8BB05FD7:
		return flashlight; break;
	case 0x440E4788:
		return golfclub; break;
	case 0x4E875F73:
		return hammer; break;
	case 0xF9DCBF2D:
		return hatchet; break;
	case 0xD8DF3C3C:
		return knuckle; break;
	case 0x99B507EA:
		return knife; break;
	case 0xDD5DF8D9:
		return machete; break;
	case 0xDFE37640:
		return switchblade; break;
	case 0x678B81B1:
		return nightstick; break;
	case 0x19044EE0:
		return wrench; break;
	case 0xCD274149:
		return battleaxe; break;
	case 0x94117305:
		return poolcue; break;
	case 0x1B06D571:
		return pistol; break;
	case 0xBFE256D4:
		return pistolmk2; break;
	case 0x5EF9FEC4:
		return combatpistol; break;
	case 0x22D8FE39:
		return appistol; break;
	case 0x3656C8C1:
		return stungun; break;
	case 0x99AEEB3B:
		return pistol50; break;
	case 0xBFD21232:
		return snspistol; break;
	case 0x88374054:
		return snspistolmk2; break;
	case 0xD205520E:
		return heavypistol; break;
	case 0x83839C4:
		return vintagepistol; break;
	case 0x47757124:
		return flaregun; break;
	case 0xDC4DB296:
		return marksmanpistol; break;
	case 0xC1B3C3D1:
		return revolver; break;
	case 0xCB96392F:
		return revolvermk2; break;
	case 0x97EA20B8:
		return doubleaction; break;
	case 0x13532244:
		return microsmg; break;
	case 0x2BE6766B:
		return smg; break;
	case 0x78A97CD0:
		return smgmk2; break;
	case 0xEFE7E2DF:
		return assaultsmg; break;
	case 0xA3D4D34:
		return combatpdw; break;
	case 0xDB1AA450:
		return machinepistol; break;
	case 0xBD248B55:
		return minismg; break;
	case 0x1D073A89:
		return pumpshotgun; break;
	case 0x555AF99A:
		return pumpshotgun_mk2; break;
	case 0x7846A318:
		return sawnoffshotgun; break;
	case 0xE284C527:
		return assaultshotgun; break;
	case 0x9D61E50F:
		return bullpupshotgun; break;
	case 0xA89CB99E:
		return musket; break;
	case 0x3AABBBAA:
		return heavyshotgun; break;
	case 0xEF951FBB:
		return dbshotgun; break;
	case 0x12E82D3D:
		return autoshotgun; break;
	case 0xBFEFFF6D:
		return assaultrifle; break;
	case 0x394F415C:
		return assaultrifle_mk2; break;
	case 0x83BF0278:
		return carbinerifle; break;
	case 0xFAD1F1C9:
		return carbinerifle_mk2; break;
	case 0xAF113F99:
		return advancedrifle; break;
	case 0xC0A3098D:
		return specialcarbine; break;
	case 0x969C3D67:
		return specialcarbine_mk2; break;
	case 0x7F229F94:
		return bullpuprifle; break;
	case 0x84D6FAFD:
		return bullpuprifle_mk2; break;
	case 0x624FE830:
		return compactrifle; break;
	case 0x9D07F764:
		return machine_gun; break;
	case 0x7FD62962:
		return combatmg; break;
	case 0xDBBD7280:
		return combatmg_mk2; break;
	case 0x61012683:
		return gusenberg; break;
	case 0x5FC3C11:
		return sniperrifle; break;
	case 0xC472FE2:
		return heavysniper; break;
	case 0xA914799:
		return heavysniper_mk2; break;
	case 0xC734385A:
		return marksmanrifle; break;
	case 0x6A6C02E0:
		return marksmanrifle_mk2; break;
	case 0xB1CA77B1:
		return rpg; break;
	case 0xA284510B:
		return grenadelauncher; break;
	case 0x4DD2DC56:
		return grenadelauncher_smoke; break;
	case 0x42BF8A85:
		return minigun; break;
	case 0x7F7497E5:
		return firework; break;
	case 0x6D544C99:
		return railgun; break;
	case 0x63AB0442:
		return hominglauncher; break;
	case 0x781FE4A:
		return compactlauncher; break;
	case 0x93E220BD:
		return grenade; break;
	case 0xA0973D5E:
		return bzgas; break;
	case 0xFDBC8A50:
		return smokegrenade; break;
	case 0x497FACC3:
		return flare; break;
	case 0x24B17070:
		return molotov; break;
	case 0x2C3731D9:
		return stickybomb; break;
	case 0xAB564B93:
		return proxmine; break;
	case 0x787F0BB:
		return snowball; break;
	case 0xBA45E8B8:
		return pipebomb; break;
	case 0x23C9F95C:
		return ball; break;
	case 0x34A67B97:
		return petrolcan; break;
	case 0x60EC506:
		return fireextinguisher; break;
	case 0xFBAB5776:
		return parachute; break;
	case 0x9D1F17E6:
		return weapon_militaryrifle; break;
	case 0xC78D71B4:
		return weapon_heavyrifle; break;
	case 0xD1D5F52B:
		return weapon_tacticalrifle; break;
	case 0x6E7DDDEC:
		return weapon_precisionrifle; break;
	case 0xDB26713A:
		return weapon_emplauncher; break;
	case 0xB62D1F67:
		return weapon_rayminigun; break;
	default:
		return unarmed; break;
	}
}

void DrawWeapon(std::uint64_t Ped, Vector2 minPos, Vector2 maxPos)
{
	const auto WeaponManager = device_t.read<std::uint64_t>(Ped + 0x10D8);
	const auto WeaponInfo = device_t.read<std::uint64_t>(WeaponManager + 0x20);
	const auto WeaponHash = device_t.read<std::uint64_t>(WeaponInfo + 0x10);

	const std::string WeaponName = GetWeaopnName(WeaponHash);
	ImVec2 textPosition = ImVec2(
		(minPos.x + maxPos.x - ImGui::CalcTextSize(WeaponName.c_str()).x) / 2.f,
		maxPos.y + 7.f);

	ImGui::PushFontShadow(0xFF000000);
	DL->AddText(textPosition, ImColor(255,255,255), WeaponName.c_str());
	ImGui::PopFontShadow();
}

ImColor GetHealthBarColor(float health)
{
	if (health > 100) {

		float factor = (health - 100) / 100;
		int greenValue = static_cast<int>(255 * factor);
		return ImColor(255 - greenValue, 255, 0, 255);
	}
	else {
		float factor = health / 100;
		int redValue = static_cast<int>(255 * factor);
		return ImColor(255, redValue, 0, 255);
	}
}

void DrawHealthBar(std::uint64_t Ped, Vector2 minPos, Vector2 maxPos)
{
	const auto Health = device_t.read<float>(Ped + 0x280);

	float filledHeight = (maxPos.y - minPos.y) * (Health / 200.f);
	ImColor healthBarColor = GetHealthBarColor(Health);

	DL->AddRect(
		ImVec2(minPos.x - 6, minPos.y - 1.f),
		ImVec2(minPos.x - 3, maxPos.y + 1.f),
		ImColor(0, 0, 0, 255)
	);

	DL->AddRectFilled(
		ImVec2(minPos.x - 5, maxPos.y - filledHeight),
		ImVec2(minPos.x - 4, maxPos.y),
		healthBarColor
	);
}

void Box(std::uint64_t Ped, std::uint64_t ViewPort, bool Filled)
{
	const auto PedModelInfo = device_t.read<std::uint64_t>(Ped + 0x20);
	auto Origin = device_t.read<Vector3>(Ped + 0x90);

	auto min = device_t.read<Vector3>(PedModelInfo + 0x30);
	auto max = device_t.read<Vector3>(PedModelInfo + 0x40);

	min = min + Origin;
	max = max + Origin;

	Vector3 points[] = {
		Vector3(min.x, min.y, min.z),
		Vector3(min.x, max.y, min.z),
		Vector3(max.x, max.y, min.z),
		Vector3(max.x, min.y, min.z),
		Vector3(max.x, max.y, max.z),
		Vector3(min.x, max.y, max.z),
		Vector3(min.x, min.y, max.z),
		Vector3(max.x, min.y, max.z)
	};

	Vector2 blb = WorldToScreen(points[0], ViewPort);
	Vector2 brb = WorldToScreen(points[1], ViewPort);
	Vector2 frb = WorldToScreen(points[2], ViewPort);
	Vector2 flb = WorldToScreen(points[3], ViewPort);
	Vector2 frt = WorldToScreen(points[4], ViewPort);
	Vector2 brt = WorldToScreen(points[5], ViewPort);
	Vector2 blt = WorldToScreen(points[6], ViewPort);
	Vector2 flt = WorldToScreen(points[7], ViewPort);

	Vector2 arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	if (blb.IsZero() ||
		brb.IsZero() ||
		frb.IsZero() ||
		flb.IsZero() ||
		frt.IsZero() ||
		brt.IsZero() ||
		blt.IsZero() ||
		flt.IsZero()
		) {
		return;
	}

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (top < arr[i].y)
			top = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (bottom > arr[i].y)
			bottom = arr[i].y;
	}

	const Vector2 Min2d = Vector2(left, bottom);
	const Vector2 Max2d = Vector2(right, top);

	if (Min2d.IsZero() || Max2d.IsZero())
		return;

	constexpr float Thickness = 1.f;
	constexpr float ShadowThickness = Thickness + 1.f;

	if (Filled)
	{
		DL->AddRect(Min2d.ToImVec2(), Max2d.ToImVec2(), ImColor(0.f, 0.f, 0.f, BoxColor.Value.w), 0.0f, 0, ShadowThickness);
		DL->AddRect(Min2d.ToImVec2(), Max2d.ToImVec2(), BoxColor, 0.0f, 0, Thickness);

		DL->AddRectFilled(Min2d.ToImVec2(), Max2d.ToImVec2(), BoxColor);
	}
	else
	{
		DL->AddRect(Min2d.ToImVec2(), Max2d.ToImVec2(), ImColor(0.f, 0.f, 0.f, BoxColor.Value.w), 0.0f, 0, ShadowThickness);
		DL->AddRect(Min2d.ToImVec2(), Max2d.ToImVec2(), BoxColor, 0.0f, 0, Thickness);
	}

	//DrawName(Ped, Min2d, Max2d);
	if (WeaponName) DrawWeapon(Ped, Min2d, Max2d);
	if (HealthBar) DrawHealthBar(Ped, Min2d, Max2d);
}

void Skeleton(std::uint64_t Ped, std::uint64_t ViewPort)
{
	std::vector<std::pair<int, int>> bonePairs = {
		{ 0, 7 },
		{ 7, 6 },
		{ 7, 5 },
		{ 7, 8 },
		{ 8, 3 },
		{ 8, 4 }
	};

	constexpr float skeletonThickness = 1.0f;

	std::unordered_map<int, Vector2> bonePositions;

	for (const auto& pair : bonePairs)
	{
		Vector2 startPos, endPos;

		if (bonePositions.find(pair.first) == bonePositions.end())
		{
			startPos = GetBonePos2D(Ped, pair.first, ViewPort);
			bonePositions[pair.first] = startPos;
		}
		else
		{
			startPos = bonePositions[pair.first];
		}

		if (bonePositions.find(pair.second) == bonePositions.end())
		{
			endPos = GetBonePos2D(Ped, pair.second, ViewPort);
			bonePositions[pair.second] = endPos;
		}
		else
		{
			endPos = bonePositions[pair.second];
		}

		DL->AddLine(
			ImVec2(startPos.x, startPos.y),
			ImVec2(endPos.x, endPos.y),
			SkelColor,
			skeletonThickness
		);
	}
}