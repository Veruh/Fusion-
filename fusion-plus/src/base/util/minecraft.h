#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct MinecraftUtils
{
    static inline bool IsWeapon(int id)
    {
		return id == 276 || id == 283 || id == 267 || id == 272 || id == 268;
    }

    static inline bool IsWeapon(std::string name)
    {
		return name.find("sword") != std::string::npos;
    }

    static inline bool IsWeapon(CItemStack item)
    {
        if (item.GetInstance() != nullptr) {
            return IsWeapon(item.GetItem().GetID());
        }
		return false;
    }

    static inline bool IsEmpty(CItemStack item)
    {
        return item.GetInstance() == nullptr;
    }
};
