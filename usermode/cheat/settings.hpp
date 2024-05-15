#pragma once


inline bool ShowMenu = false;


inline bool EnableAimbot = true;
inline float FOV = 120.f;
inline float Smooth = 3.f;
inline int AimBone = 0;
inline int aimKey = VK_RBUTTON;
enum class AimBones{Head = 0, Neck = 7, Chest = 8, RLeg = 4, LLeg = 3, Closest = 999};
inline const char* AimBonesList[]{ "Head", "Neck", "Chest", "RLeg", "LLeg", "Closest" };

inline bool EnableEsp = true;
inline bool OnlyPlayer = false;
inline bool WeaponName = false;
inline bool HealthBar = false;
inline bool DrawSkeleton = true;
inline bool DrawBox = true;
inline bool Filled = true;
inline ImColor BoxColor = ImColor(30, 30, 30, 100);
inline ImColor SkelColor = ImColor(100, 100, 100, 200);
