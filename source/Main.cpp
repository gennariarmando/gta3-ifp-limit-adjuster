#include "plugin.h"
#include "CAnimManager.h"
#include "CAnimBlendHierarchy.h"

class IfpLimitAdjusterIII {
public:
	static inline CAnimationStyleDescriptor animAssocDefinitions[25] = {};
	static inline std::array<CAnimBlendHierarchy, 1024> animations = {};

	static inline const std::unordered_map<std::string, int32_t> flagMap = {
		{ "ASSOC_RUNNING", ASSOC_RUNNING },
		{ "ASSOC_REPEAT", ASSOC_REPEAT },
		{ "ASSOC_DELETEFADEDOUT", ASSOC_DELETEFADEDOUT },
		{ "ASSOC_FADEOUTWHENDONE", ASSOC_FADEOUTWHENDONE },
		{ "ASSOC_PARTIAL", ASSOC_PARTIAL },
		{ "ASSOC_MOVEMENT", ASSOC_MOVEMENT },
		{ "ASSOC_HAS_TRANSLATION", ASSOC_HAS_TRANSLATION },
		{ "ASSOC_WALK", ASSOC_WALK },
		{ "ASSOC_IDLE", ASSOC_IDLE },
		{ "ASSOC_NOWALK", ASSOC_NOWALK },
		{ "ASSOC_BLOCK", ASSOC_BLOCK },
		{ "ASSOC_FRONTAL", ASSOC_FRONTAL },
		{ "ASSOC_HAS_X_TRANSLATION", ASSOC_HAS_X_TRANSLATION }
	};

	struct AnimData {
		std::string name;
		CAnimAssocDesc desc;
	};

	static inline std::vector<AnimData> listOfNewAnims = {};

	static inline bool ReadDataFile() {
		std::ifstream file(PLUGIN_PATH("IfpLimitAdjusterIII.dat"));
		if (!file.is_open()) {
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (line.empty() || line[0] == '#' || line[0] == ' ' || line[0] == ';')
				continue;

			std::stringstream ss(line);
			std::string temp;

			AnimData data;

			if (std::getline(ss, temp, ',')) {
				data.desc.animId = std::stoi(temp);
			}
			else {
				continue;
			}

			if (std::getline(ss, temp, ',')) {
				temp.erase(0, temp.find_first_not_of(' '));
				temp.erase(temp.find_last_not_of(' ') + 1);
				data.name = temp;
			}
			else {
				continue;
			}

			if (std::getline(ss, temp)) {
				temp.erase(std::remove_if(temp.begin(), temp.end(), ::isspace), temp.end());

				std::istringstream flagStream(temp);
				int flags = 0;
				std::string flag;
				while (std::getline(flagStream, flag, '|')) {
					if (flagMap.find(flag) != flagMap.end()) {
						flags |= flagMap.at(flag);
					}
					else {
					}
				}
				data.desc.flags = flags;
			}
			else {
				continue;
			}

			listOfNewAnims.push_back(data);
		}

		file.close();

		return true;
	}

public:
	IfpLimitAdjusterIII() {
		// TODO: free memory ?
		plugin::Events::initRwEvent += []() {
			listOfNewAnims.push_back({ CAnimManager::ms_aAnimAssocDefinitions[0].animNames[0], {} }); // 173
			if (ReadDataFile()) {
				// Anim limit
				plugin::patch::SetPointer(0x4011A1 + 2, &animations);
				plugin::patch::SetPointer(0x4033C7 + 1, &animations);
				plugin::patch::SetPointer(0x403508 + 1, &animations);
				plugin::patch::SetPointer(0x403C76 + 4, &animations);
				plugin::patch::SetPointer(0x40434B + 2, &animations);

				plugin::patch::SetPointer(0x4035B7 + 3, &animAssocDefinitions->groupName);
				plugin::patch::SetPointer(0x403944 + 2, &animAssocDefinitions->modelIndex);
				plugin::patch::SetPointer(0x40396E + 2, &animAssocDefinitions->animsCount);
				plugin::patch::SetPointer(0x403986 + 2, &animAssocDefinitions->blockName);
				plugin::patch::SetPointer(0x4039B8 + 2, &animAssocDefinitions->animDescs);
				plugin::patch::SetPointer(0x403974 + 2, &animAssocDefinitions->animNames);

				const int32_t oldstdAnimsLimit = 173;
				const int32_t newstdAnimsLimit = oldstdAnimsLimit + listOfNewAnims.size();
				int32_t stdAnimsCount = 0;
				animAssocDefinitions[0].animNames = new char* [newstdAnimsLimit];
				animAssocDefinitions[0].animDescs = new CAnimAssocDesc[newstdAnimsLimit];

				// Copy original anim definitions
				for (int32_t i = 0; i < 25; i++) {
					auto old_anim = &CAnimManager::ms_aAnimAssocDefinitions[i];
					auto new_anim = &animAssocDefinitions[i];

					new_anim->groupName = new char[strlen(old_anim->groupName)];
					strcpy(new_anim->groupName, old_anim->groupName);

					new_anim->blockName = new char[strlen(old_anim->blockName)];
					strcpy(new_anim->blockName, old_anim->blockName);

					new_anim->modelIndex = old_anim->modelIndex;
					new_anim->animsCount = old_anim->animsCount;

					if (i == 0)
						stdAnimsCount = new_anim->animsCount;

					int32_t anims = new_anim->animsCount;
					for (int32_t j = 0; j < anims; j++) {
						if (!new_anim->animNames)
							new_anim->animNames = new char* [anims];

						new_anim->animNames[j] = new char[strlen(old_anim->animNames[j])];
						strcpy(new_anim->animNames[j], old_anim->animNames[j]);

						if (!new_anim->animDescs)
							new_anim->animDescs = new CAnimAssocDesc[anims];

						new_anim->animDescs[j] = old_anim->animDescs[j];
					}
				}

				// Add our new anims to std only
				animAssocDefinitions[0].animsCount = newstdAnimsLimit;
				for (int32_t j = stdAnimsCount; j < newstdAnimsLimit; j++) {
					int32_t index = j - stdAnimsCount;

					animAssocDefinitions[0].animNames[j] = new char[strlen(listOfNewAnims[index].name.c_str())];
					strcpy(animAssocDefinitions[0].animNames[j], listOfNewAnims[index].name.c_str());

					animAssocDefinitions[0].animDescs[j] = listOfNewAnims[index].desc;
				}
			}
        };
    }
} ifpLimitAdjusterIII;
