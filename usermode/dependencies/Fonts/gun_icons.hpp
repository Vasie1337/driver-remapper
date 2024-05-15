#pragma once
#include <unordered_map>
#include <map>
#include <string>

inline const char* GunIcon(const std::string weapon)
{
	const static std::unordered_map<std::string, const char*> gunIcons = {
        {"knife", "]"},
        {"knife_t", "["},
        {"deagle", "A"},
        {"elite", "B"},
        {"fiveseven", "C"},
        {"glock", "D"},
        {"revolver", "J"},
        {"hkp2000", "E"},
        {"p250", "F"},
        {"usp_silencer", "G"},
        {"tec9", "H"},
        {"cz75a", "I"},
        {"mac10", "K"},
        {"ump45", "L"},
        {"bizon", "M"},
        {"mp7", "N"},
        {"mp9", "O"},
        {"p90", "P"},
        {"galilar", "Q"},
        {"famas", "R"},
        {"m4a1_silencer", "T"},
        {"m4a1", "S"},
        {"aug", "U"},
        {"sg556", "V"},
        {"ak47", "W"},
        {"g3sg1", "X"},
        {"scar20", "Y"},
        {"awp", "Z"},
        {"ssg08", "a"},
        {"xm1014", "b"},
        {"sawedoff", "c"},
        {"mag7", "d"},
        {"nova", "e"},
        {"negev", "f"},
        {"m249", "g"},
        {"taser", "h"},
        {"flashbang", "i"},
        {"hegrenade", "j"},
        {"smokegrenade", "k"},
        {"molotov", "l"},
        {"decoy", "m"},
        {"incgrenade", "n"},
        {"c4", "o"}
	};

    auto it = gunIcons.find(weapon);
    if (it != gunIcons.end()) {
        return it->second;
    }

	return "";
}
