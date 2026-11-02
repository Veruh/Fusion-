#include "menu.h"
#include "fusionGui.h"
#include "../../main.h"
#include "base.h"
#include "configManager/configManager.h"
#include <cstdio>
#include <exception>

namespace {
std::string message;
void Profiles() {
    static std::vector<std::string> files;
    static bool loaded=false;
    static std::string selected;
    static char name[128]="";
    try {
        if(!loaded) { files=ConfigManager::GetConfigList(); loaded=true; }
        FusionGui::Section("SAVED PROFILES");
        if(ImGui::Button("Refresh list")) files=ConfigManager::GetConfigList();
        if(files.empty()) ImGui::TextDisabled("No profiles yet. Save one below.");
        if(ImGui::BeginChild("##profiles",ImVec2(0,150),true)) {
            for(const auto& file:files) if(ImGui::Selectable(file.c_str(),selected==file)) {
                selected=file;
                std::snprintf(name,sizeof(name),"%s",file.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::BeginDisabled(selected.empty());
        if(ImGui::Button("Load selected",ImVec2(-1,34))) message=ConfigManager::LoadConfig(selected.c_str())?"Profile loaded.":"Could not load this profile.";
        ImGui::EndDisabled();
        FusionGui::Section("SAVE CURRENT SETTINGS");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##profileName","Profile name",name,sizeof(name));
        // Keep profile names inside the configuration directory.
        const std::string text(name);
        bool valid=!text.empty() && text!="." && text!=".." && text.find_first_of("<>:\"/\\|?*")==std::string::npos && text.back()!='.' && text.back()!=' ';
        ImGui::BeginDisabled(!valid);
        if(ImGui::Button("Save profile",ImVec2(-1,34))) {
            if(ConfigManager::SaveConfig(name)>=0) { files=ConfigManager::GetConfigList(); selected=name; message="Profile saved."; }
            else message="Could not save profile.";
        }
        ImGui::EndDisabled();
        if(!valid && !text.empty()) ImGui::TextWrapped("Use a name without path separators or special filename characters.");
    } catch(const std::exception&) { message="Invalid profile or filesystem error. Existing settings may have been partially loaded."; }
    if(!message.empty()) { ImGui::Spacing(); ImGui::TextWrapped("%s",message.c_str()); }
}
void Preferences() {
    FusionGui::Section("APPEARANCE");
    FusionGui::Picker("Accent color",settings::Menu_AccentColor);
    FusionGui::Toggle("Move while menu is open",&settings::Menu_GUIMovement);
    FusionGui::Toggle("Watermark",&settings::Menu_Watermark);
    FusionGui::Toggle("Disable overlay rendering",&settings::Menu_DisableAllRendering);
    ImGui::TextDisabled("Menu shortcut: INSERT");
    FusionGui::Section("FRIENDS");
    ImGui::TextWrapped("Ignored by Aim Assist when its friend filter is enabled.");
    static char name[128]="";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##friend","Player name",name,sizeof(name));
    if(ImGui::Button("Add friend",ImVec2(-1,32)) && name[0]) {
        try { if(ConfigManager::AddFriend(name)) { name[0]=0; message="Friend saved."; } else message="Already listed or could not save."; }
        catch(const std::exception&) { message="Could not save friends."; }
    }
    std::string remove;
    for(const auto& friendName:settings::friends) {
        ImGui::PushID(friendName.c_str());
        if(ImGui::SmallButton("Remove")) remove=friendName;
        ImGui::SameLine(); ImGui::TextUnformatted(friendName.c_str());
        ImGui::PopID();
    }
    if(!remove.empty()) {
        try { message=ConfigManager::RemoveFriend(remove)?"Friend removed.":"Could not save friends."; }
        catch(const std::exception&) { message="Could not save friends."; }
    }
    if(!message.empty()) ImGui::TextWrapped("%s",message.c_str());
}
}
void Menu::RenderMenu() {
    FusionGui::Render(Profiles,Preferences,&Menu::Open,&Base::Running);
}
