#pragma once
#include "imgui/imgui.h"
#include "configManager/settings.h"
#include <algorithm>

// Shared by the injected menu and the standalone, non-injecting visual preview.
namespace FusionGui {
inline int page = 0;
inline ImVec4 Accent() { return ImVec4(settings::Menu_AccentColor[0], settings::Menu_AccentColor[1], settings::Menu_AccentColor[2], 1); }
inline ImU32 Color(ImVec4 c) { return ImGui::ColorConvertFloat4ToU32(c); }
inline void Theme() {
    auto& s = ImGui::GetStyle();
    s.WindowRounding = 14; s.ChildRounding = 10; s.FrameRounding = 6;
    s.PopupRounding = 8; s.GrabRounding = 6; s.ScrollbarRounding = 6;
    s.WindowBorderSize = 1; s.ChildBorderSize = 1; s.FrameBorderSize = 0;
    s.WindowPadding = ImVec2(18,18); s.FramePadding = ImVec2(10,7);
    s.ItemSpacing = ImVec2(12,10); s.ItemInnerSpacing = ImVec2(8,6);
    s.ScrollbarSize = 9; s.GrabMinSize = 12;
    auto* c = s.Colors;
    c[ImGuiCol_Text] = ImVec4(.91f,.92f,.98f,1);
    c[ImGuiCol_TextDisabled] = ImVec4(.49f,.53f,.66f,1);
    c[ImGuiCol_WindowBg] = ImVec4(.035f,.043f,.07f,.99f);
    c[ImGuiCol_ChildBg] = ImVec4(.05f,.061f,.095f,1);
    c[ImGuiCol_PopupBg] = ImVec4(.065f,.075f,.12f,1);
    c[ImGuiCol_Border] = ImVec4(.15f,.18f,.29f,.8f);
    c[ImGuiCol_FrameBg] = ImVec4(.09f,.105f,.165f,1);
    c[ImGuiCol_FrameBgHovered] = ImVec4(.13f,.14f,.23f,1);
    c[ImGuiCol_FrameBgActive] = ImVec4(.18f,.18f,.31f,1);
    c[ImGuiCol_Button] = ImVec4(.105f,.115f,.19f,1);
    c[ImGuiCol_ButtonHovered] = ImVec4(.20f,.18f,.35f,1);
    c[ImGuiCol_ButtonActive] = Accent();
    c[ImGuiCol_Header] = ImVec4(.18f,.14f,.34f,1);
    c[ImGuiCol_HeaderHovered] = ImVec4(.22f,.18f,.40f,1);
    c[ImGuiCol_HeaderActive] = Accent();
    c[ImGuiCol_SliderGrab] = c[ImGuiCol_SliderGrabActive] = c[ImGuiCol_CheckMark] = Accent();
    c[ImGuiCol_Separator] = ImVec4(.15f,.18f,.29f,.8f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0,0,0,0);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(.22f,.24f,.36f,1);
    c[ImGuiCol_NavHighlight] = Accent();
}
inline void Section(const char* title) {
    ImGui::Spacing(); ImGui::TextColored(ImVec4(.64f,.61f,.89f,1), "%s", title);
    ImGui::Separator(); ImGui::Spacing();
}
inline void Toggle(const char* label, bool* value) {
    ImGui::PushID(label);
    const auto p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    const bool pressed = ImGui::InvisibleButton("##toggle", ImVec2(w,32));
    if (pressed) *value = !*value;
    auto* d = ImGui::GetWindowDrawList();
    if (ImGui::IsItemHovered()) d->AddRectFilled(p,ImVec2(p.x+w,p.y+32),IM_COL32(255,255,255,6),5);
    d->AddText(ImVec2(p.x,p.y+7),ImGui::GetColorU32(ImGuiCol_Text),label);
    const float x = p.x+w-42, y=p.y+5;
    d->AddRectFilled(ImVec2(x,y),ImVec2(x+42,y+22),*value ? Color(Accent()) : IM_COL32(38,43,65,255),11);
    d->AddCircleFilled(ImVec2(x+(*value?31:11),y+11),8, *value?IM_COL32(255,255,255,255):IM_COL32(134,142,167,255),20);
    ImGui::PopID();
}
inline void Float(const char* label,float* v,float lo,float hi,const char* format="%.1f") {
    ImGui::PushID(label); ImGui::TextUnformatted(label);
    ImGui::SetNextItemWidth(-1); ImGui::SliderFloat("##value",v,lo,hi,format,ImGuiSliderFlags_AlwaysClamp); ImGui::PopID();
}
inline void Int(const char* label,int* v,int lo,int hi,const char* format="%d") {
    ImGui::PushID(label); ImGui::TextUnformatted(label);
    ImGui::SetNextItemWidth(-1); ImGui::SliderInt("##value",v,lo,hi,format,ImGuiSliderFlags_AlwaysClamp); ImGui::PopID();
}
inline void Combo(const char* label,int* v,const char* const* options,int count) {
    ImGui::PushID(label); ImGui::TextUnformatted(label); ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##value",v,options,count); ImGui::PopID();
}
inline void Picker(const char* label,float* v) {
    ImGui::PushID(label); ImGui::TextUnformatted(label); ImGui::SetNextItemWidth(-1);
    ImGui::ColorEdit4("##color",v,ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar); ImGui::PopID();
}
inline void Aim() {
    Toggle("Enable Aim Assist", &settings::AA_Enabled);
    Section("GENERAL");
    Float("Field of view",&settings::AA_fov,5,180,"%.0f deg");
    Float("Smoothness",&settings::AA_smooth,1,90);
    Float("Lock distance",&settings::AA_aimDistance,1,8,"%.1f blocks");
    Combo("Target priority",&settings::AA_targetPriority,settings::AA_targetPriorityList,3);
    Section("TARGET FILTERS");
    Toggle("Visibility check",&settings::AA_visibilityCheck);
    Toggle("Ignore invisible players",&settings::AA_invisibleCheck);
    Toggle("Ignore friends",&settings::AA_ignoreFriends);
    Toggle("Weapon only",&settings::AA_weaponOnly);
    Toggle("Only while sprinting",&settings::AA_sprintCheck);
    Toggle("Check block breaking",&settings::AA_blockBreakCheck);
    Toggle("Require mouse movement",&settings::AA_mouseMoveCheck);
    Toggle("Require mouse button",&settings::AA_mousePressCheck);
    Section("ADAPTIVE / RANDOMNESS");
    Toggle("Adapt to strafing",&settings::AA_adaptive);
    Float("Strafing offset",&settings::AA_adaptiveOffset,.1f,15);
    Float("Yaw randomness",&settings::AA_randomYaw,0,10);
    Float("Pitch randomness",&settings::AA_randomPitch,0,1,"%.3f");
    Section("FEEDBACK");
    Toggle("FOV circle",&settings::AA_fovCircle);
    if(settings::AA_fovCircle) Picker("Circle color",settings::AA_fovCircleColor);
    Toggle("Feedback line",&settings::AA_aimAssistFeedback);
    if(settings::AA_aimAssistFeedback) Picker("Line color",settings::AA_aimAssistFeedbackColor);
}
inline void Visuals() {
    Toggle("Enable ESP",&settings::ESP_Enabled);
    Section("PLAYER DISPLAY");
    Toggle("Health bar",&settings::ESP_HealthBar);
    Toggle("Names and distance",&settings::ESP_Text);
    if(settings::ESP_Text) {
        Float("Text size",&settings::ESP_TextSize,1,50);
        Float("Text distance",&settings::ESP_TextUnrenderDistance,0,20,"%.1f blocks");
        Picker("Text color",settings::ESP_TextColor);
        Toggle("Text outline",&settings::ESP_TextOutline);
        if(settings::ESP_TextOutline) Picker("Text outline color",settings::ESP_TextOutlineColor);
    }
    Float("Fade distance",&settings::ESP_FadeDistance,0,10);
    Section("BOX STYLE");
    Combo("Box type",&settings::ESP_BoxType,settings::ESP_BoxTypeList,2);
    if(settings::ESP_BoxType==0) { Toggle("Box edges",&settings::ESP_Box); Picker("Box color",settings::ESP_BoxColor); }
    else { Float("Thickness",&settings::ESP_3DBoxThickness,.5f,5); Picker("3D box color",settings::ESP_3DBoxColor); }
    Toggle("Filled box",&settings::ESP_FilledBox);
    if(settings::ESP_FilledBox) {
        Picker("Fill color",settings::ESP_FilledBoxColor);
        if(settings::ESP_BoxType==0) Picker("Second fill color",settings::ESP_SecondFilledBoxColor);
    }
    Toggle("Outline",&settings::ESP_Outline);
    if(settings::ESP_Outline) Picker("Outline color",settings::ESP_OutlineColor);
    Section("FRIENDS");
    Toggle("Highlight friends",&settings::ESP_HighlightFriends);
    if(settings::ESP_HighlightFriends) {
        Picker("Friend box color",settings::ESP_FriendBoxColor);
        Picker("Friend 3D color",settings::ESP_Friend3DBoxColor);
        Picker("Friend fill color",settings::ESP_FriendFilledBoxColor);
        Picker("Friend second fill",settings::ESP_FriendSecondFilledBoxColor);
        Picker("Friend outline color",settings::ESP_FriendOutlineColor);
    }
}
inline void Velocity() {
    Toggle("Enable Velocity",&settings::Velocity_Enabled);
    Section("JUMP RESET");
    ImGui::TextDisabled("Mode: Jump Reset");
    Float("Reaction time",&settings::Velocity_JRReactionTime,0,1000,"%.1f ms");
    Int("Chance",&settings::Velocity_JRChange,0,100,"%d %%");
}
inline bool Nav(const char* label,int index) {
    const bool active=page==index;
    ImGui::PushStyleColor(ImGuiCol_Button, active?ImVec4(.23f,.16f,.43f,1):ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Text,active?ImVec4(1,1,1,1):ImVec4(.60f,.63f,.75f,1));
    bool clicked=ImGui::Button(label,ImVec2(-1,42));
    if(active) { auto p=ImGui::GetItemRectMin(); ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x,p.y+9),ImVec2(p.x+3,p.y+33),Color(Accent()),2); }
    ImGui::PopStyleColor(2);
    if(clicked) page=index;
    return clicked;
}
inline void Preview() {
    Section("STYLE PREVIEW");
    ImGui::TextDisabled("Illustration / not a live target");
    ImVec2 p=ImGui::GetCursorScreenPos();
    const float width=ImGui::GetContentRegionAvail().x;
    ImGui::Dummy(ImVec2(width,196));
    auto* d=ImGui::GetWindowDrawList();
    d->AddRectFilled(p,ImVec2(p.x+width,p.y+196),IM_COL32(12,17,28,255),8);
    float cx=p.x+width*.5f, cy=p.y+96;
    const auto a=Color(Accent());
    d->AddLine(ImVec2(p.x+15,p.y+171),ImVec2(p.x+width-15,p.y+171),IM_COL32(36,44,63,255));
    d->AddRectFilled(ImVec2(cx-15,cy-45),ImVec2(cx+15,cy-15),IM_COL32(116,109,155,255),3);
    d->AddRectFilled(ImVec2(cx-20,cy-10),ImVec2(cx+20,cy+34),IM_COL32(70,62,111,255),3);
    d->AddRectFilled(ImVec2(cx-20,cy+38),ImVec2(cx-3,cy+68),IM_COL32(56,57,82,255),2);
    d->AddRectFilled(ImVec2(cx+3,cy+38),ImVec2(cx+20,cy+68),IM_COL32(56,57,82,255),2);
    if(page==1) {
        if(settings::ESP_FilledBox) d->AddRectFilled(ImVec2(cx-31,cy-53),ImVec2(cx+31,cy+73),Color(ImVec4(Accent().x,Accent().y,Accent().z,.12f)));
        if(settings::ESP_Box || settings::ESP_BoxType==1) d->AddRect(ImVec2(cx-31,cy-53),ImVec2(cx+31,cy+73),a,3,0,1.5f);
        if(settings::ESP_HealthBar) d->AddLine(ImVec2(cx-38,cy+72),ImVec2(cx-38,cy-52),IM_COL32(59,223,169,255),3);
        if(settings::ESP_Text) d->AddText(ImVec2(cx-34,cy-75),IM_COL32(219,220,240,255),"Player");
    } else {
        d->AddCircle(ImVec2(cx,cy),58,a,64,1.5f);
        d->AddLine(ImVec2(cx-70,cy),ImVec2(cx-48,cy),a,1.5f);
        d->AddLine(ImVec2(cx+48,cy),ImVec2(cx+70,cy),a,1.5f);
        d->AddLine(ImVec2(cx,cy-70),ImVec2(cx,cy-48),a,1.5f);
        d->AddCircleFilled(ImVec2(cx,cy),3,IM_COL32(230,229,250,255));
    }
}
// Callback functions keep filesystem operations out of the presentation layer.
inline void Render(void (*profiles)(),void (*preferences)(),bool* open,bool* running,bool standalone=false) {
    Theme();
    ImGuiIO& io=ImGui::GetIO();
    const bool compact=io.DisplaySize.x<980;
    const float width=(std::min)(compact?740.f:980.f,(std::max)(320.f,io.DisplaySize.x-24));
    const float height=(std::min)(650.f,(std::max)(240.f,io.DisplaySize.y-24));
    ImGui::SetNextWindowSize(ImVec2(width,height));
    static ImVec2 previousDisplay(0,0);
    const bool resized=previousDisplay.x!=io.DisplaySize.x || previousDisplay.y!=io.DisplaySize.y;
    previousDisplay=io.DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*.5f,io.DisplaySize.y*.5f),resized?ImGuiCond_Always:ImGuiCond_Appearing,ImVec2(.5f,.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    bool visible=ImGui::Begin("FUSION+##main",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    if(!visible) { ImGui::End(); return; }
    auto origin=ImGui::GetWindowPos(); auto* draw=ImGui::GetWindowDrawList();
    draw->AddRectFilled(ImVec2(origin.x+1,origin.y+1),ImVec2(origin.x+width-1,origin.y+70),IM_COL32(14,17,29,255),14,ImDrawFlags_RoundCornersTop);
    draw->AddLine(ImVec2(origin.x+18,origin.y+70),ImVec2(origin.x+width-18,origin.y+70),IM_COL32(40,46,70,255));
    draw->AddCircle(ImVec2(origin.x+31,origin.y+33),13,Color(Accent()),6,2);
    draw->AddText(ImVec2(origin.x+56,origin.y+18),IM_COL32(242,241,255,255),"F U S I O N +");
    draw->AddText(ImVec2(origin.x+56,origin.y+41),IM_COL32(137,139,167,255),standalone?"STANDALONE PREVIEW / NO GAME CONNECTED":"YOUR CONFIG. YOUR STYLE.");
    ImGui::SetCursorPos(ImVec2(width-116,23));
    ImGui::TextDisabled("v0.0.4"); ImGui::SameLine();
    if(ImGui::SmallButton(" X ")) *open=false;
    const float sidebar=compact?144.f:174.f;
    ImGui::SetCursorPos(ImVec2(14,86));
    ImGui::BeginChild("##navigation",ImVec2(sidebar-14,height-104),false,ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("  WORKSPACE"); ImGui::Spacing();
    Nav("Aim Assist",0); Nav("Visuals",1); Nav("Velocity",2);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    Nav("Profiles",3); Nav("Settings",4);
    ImGui::SetCursorPosY((std::max)(ImGui::GetCursorPosY()+16,height-214));
    ImGui::Separator(); ImGui::TextDisabled("%s",standalone?" Local preview":" INSERT / menu");
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(.93f,.53f,.58f,1));
    if(ImGui::Button(standalone?"Close preview":"Detach",ImVec2(-1,32))) {
        if(standalone) { *running=false; *open=false; }
        else ImGui::OpenPopup("Detach Fusion?");
    }
    ImGui::PopStyleColor();
    if(ImGui::BeginPopupModal("Detach Fusion?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Unload Fusion from this game session?");
        if(ImGui::Button("Detach now")) { *running=false; *open=false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if(ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::EndChild();
    const char* titles[]={"Aim Assist","Visuals","Velocity","Profiles","Settings"};
    const char* subtitles[]={"Precision, configured your way.","A clearer view of your world.","Movement controls, in one place.","Keep your favorite configurations.","Make Fusion feel like yours."};
    const float contentX=sidebar+20;
    ImGui::SetCursorPos(ImVec2(contentX,87)); ImGui::TextUnformatted(titles[page]);
    ImGui::SetCursorPos(ImVec2(contentX,112)); ImGui::TextDisabled("%s",subtitles[page]);
    const bool rail=!compact && page<3;
    const float railWidth=224;
    const float mainWidth=width-contentX-20-(rail?railWidth+16:0);
    ImGui::SetCursorPos(ImVec2(contentX,148));
    ImGui::BeginChild("##settingsPanel",ImVec2(mainWidth,height-168),true);
    switch(page) { case 0:Aim();break; case 1:Visuals();break; case 2:Velocity();break; case 3:profiles();break; case 4:preferences();break; }
    ImGui::EndChild();
    if(rail) {
        ImGui::SetCursorPos(ImVec2(contentX+mainWidth+16,148));
        ImGui::BeginChild("##context",ImVec2(railWidth,height-168),true);
        Preview(); Section("SESSION");
        int active=int(settings::AA_Enabled)+int(settings::ESP_Enabled)+int(settings::Velocity_Enabled);
        ImGui::TextUnformatted("Modules enabled"); ImGui::TextColored(Accent(),"%d / 3",active);
        ImGui::TextUnformatted("Friends"); ImGui::TextColored(Accent(),"%d",int(settings::friends.size()));
        Section("QUICK TIP");
        ImGui::TextWrapped("%s",standalone?"Controls update this preview only. No game is connected. Save a profile to keep your settings.":"Scroll to see all settings. Changes apply immediately. Save a profile to keep them.");
        ImGui::EndChild();
    }
    ImGui::End();
}
}
