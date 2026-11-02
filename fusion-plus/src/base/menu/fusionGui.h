#pragma once
#include "imgui/imgui.h"
#include "configManager/settings.h"
#include <algorithm>
#include <cctype>

// Shared by the injected menu and the standalone, non-injecting visual preview.
namespace FusionGui {
inline int page = 5;
inline bool Favorites[3]={false,false,false};
inline int ModuleFilter=0;
inline char Search[64]="";
inline ImFont* HeadingFont=nullptr;
inline ImFont* BrandFont=nullptr;
inline bool ShowPreview=false;
inline ImVec4 Accent() { return ImVec4(settings::Menu_AccentColor[0], settings::Menu_AccentColor[1], settings::Menu_AccentColor[2], 1); }
inline ImU32 Color(ImVec4 c) { return ImGui::ColorConvertFloat4ToU32(c); }
inline void Theme() {
    auto& s = ImGui::GetStyle();
    s.WindowRounding = 10; s.ChildRounding = 7; s.FrameRounding = 4;
    s.PopupRounding = 8; s.GrabRounding = 6; s.ScrollbarRounding = 6;
    s.WindowBorderSize = 1; s.ChildBorderSize = 1; s.FrameBorderSize = 0;
    s.WindowPadding = ImVec2(20,18); s.FramePadding = ImVec2(10,5);
    s.ItemSpacing = ImVec2(12,10); s.ItemInnerSpacing = ImVec2(8,6);
    s.ScrollbarSize = 9; s.GrabMinSize = 12;
    auto* c = s.Colors;
    c[ImGuiCol_Text] = ImVec4(.88f,.89f,.90f,1);
    c[ImGuiCol_TextDisabled] = ImVec4(.47f,.49f,.50f,1);
    c[ImGuiCol_WindowBg] = ImVec4(.055f,.052f,.062f,1);
    c[ImGuiCol_ChildBg] = ImVec4(.068f,.064f,.076f,1);
    c[ImGuiCol_PopupBg] = ImVec4(.095f,.10f,.105f,1);
    c[ImGuiCol_Border] = ImVec4(.17f,.18f,.19f,.8f);
    c[ImGuiCol_FrameBg] = ImVec4(.125f,.133f,.14f,1);
    c[ImGuiCol_FrameBgHovered] = ImVec4(.16f,.17f,.18f,1);
    c[ImGuiCol_FrameBgActive] = ImVec4(.19f,.20f,.21f,1);
    c[ImGuiCol_Button] = ImVec4(.13f,.14f,.145f,1);
    c[ImGuiCol_ButtonHovered] = ImVec4(.18f,.19f,.20f,1);
    c[ImGuiCol_ButtonActive] = Accent();
    c[ImGuiCol_Header] = ImVec4(Accent().x,Accent().y,Accent().z,.12f);
    c[ImGuiCol_HeaderHovered] = ImVec4(Accent().x,Accent().y,Accent().z,.2f);
    c[ImGuiCol_HeaderActive] = Accent();
    c[ImGuiCol_SliderGrab] = c[ImGuiCol_SliderGrabActive] = c[ImGuiCol_CheckMark] = Accent();
    c[ImGuiCol_Separator] = ImVec4(.17f,.18f,.19f,.8f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0,0,0,0);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(.25f,.27f,.28f,1);
    c[ImGuiCol_NavHighlight] = Accent();
    c[ImGuiCol_Tab]=ImVec4(.085f,.08f,.095f,1);
    c[ImGuiCol_TabActive]=ImVec4(.16f,.15f,.18f,1);
    c[ImGuiCol_TabHovered]=ImVec4(Accent().x,Accent().y,Accent().z,.23f);
}
inline void Section(const char* title) {
    ImGui::Spacing(); ImGui::TextColored(ImVec4(.53f,.56f,.58f,1), "%s", title);
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
    const float x = p.x+w-34, y=p.y+7;
    float* t=ImGui::GetStateStorage()->GetFloatRef(ImGui::GetID("##animation"),*value?1.f:0.f);
    *t+=((*value?1.f:0.f)-*t)*(std::min)(1.f,ImGui::GetIO().DeltaTime*18.f);
    d->AddRectFilled(ImVec2(x,y),ImVec2(x+34,y+18),*value ? Color(Accent()) : IM_COL32(51,55,58,255),9);
    d->AddCircleFilled(ImVec2(x+9+*t*16,y+9),6.5f, *value?IM_COL32(248,250,249,255):IM_COL32(157,163,165,255),20);
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
inline void Icon(ImDrawList* d,ImVec2 p,int index,ImU32 color) {
    const float x=p.x,y=p.y;
    if(index==0) {
        d->AddCircle(p,6,color,24,1.3f);
        d->AddLine(ImVec2(x-10,y),ImVec2(x-3,y),color,1.3f);d->AddLine(ImVec2(x+3,y),ImVec2(x+10,y),color,1.3f);
        d->AddLine(ImVec2(x,y-10),ImVec2(x,y-3),color,1.3f);d->AddLine(ImVec2(x,y+3),ImVec2(x,y+10),color,1.3f);
    } else if(index==1) {
        d->AddLine(ImVec2(x-10,y),ImVec2(x,y-6),color,1.3f);d->AddLine(ImVec2(x,y-6),ImVec2(x+10,y),color,1.3f);
        d->AddLine(ImVec2(x+10,y),ImVec2(x,y+6),color,1.3f);d->AddLine(ImVec2(x,y+6),ImVec2(x-10,y),color,1.3f);d->AddCircle(p,2.5f,color,16,1.3f);
    } else if(index==2) {
        d->AddLine(ImVec2(x-9,y+6),ImVec2(x+7,y-6),color,1.5f);d->AddLine(ImVec2(x,y-6),ImVec2(x+7,y-6),color,1.5f);d->AddLine(ImVec2(x+7,y-6),ImVec2(x+7,y+1),color,1.5f);
    } else if(index==3) {
        d->AddRect(ImVec2(x-8,y-8),ImVec2(x+8,y+8),color,2,0,1.3f);
        d->AddLine(ImVec2(x-4,y-3),ImVec2(x+4,y-3),color);d->AddLine(ImVec2(x-4,y+2),ImVec2(x+4,y+2),color);
    } else if(index==5) {
        for(int row=0;row<2;row++) for(int col=0;col<2;col++)
            d->AddRectFilled(ImVec2(x-7+col*9,y-7+row*9),ImVec2(x-1+col*9,y-1+row*9),color,1.5f);
    } else {
        d->AddCircle(p,6,color,20,1.3f);d->AddCircle(p,2,color,12,1.3f);
        for(int i=0;i<4;i++) {
            const float dx=(i==0?1.f:i==1?-1.f:0.f),dy=(i==2?1.f:i==3?-1.f:0.f);
            d->AddLine(ImVec2(x+dx*6,y+dy*6),ImVec2(x+dx*10,y+dy*10),color,2);
        }
    }
}
inline bool Nav(const char* label,int index) {
    ImGui::PushID(index);
    const auto p=ImGui::GetCursorScreenPos();const float w=ImGui::GetContentRegionAvail().x;
    const bool active=page==index;
    bool clicked=ImGui::InvisibleButton("##navigation",ImVec2(w,44));
    auto* d=ImGui::GetWindowDrawList();
    if(active || ImGui::IsItemHovered()) d->AddRectFilled(p,ImVec2(p.x+w,p.y+44),active?IM_COL32(39,43,45,255):IM_COL32(31,34,36,255),5);
    const auto color=active?Color(Accent()):IM_COL32(142,150,153,255);
    Icon(d,ImVec2(p.x+18,p.y+22),index,color);
    d->AddText(ImVec2(p.x+40,p.y+13),active?IM_COL32(241,244,243,255):IM_COL32(148,155,158,255),label);
    const float arrow=p.x+w-15;
    d->AddLine(ImVec2(arrow,p.y+19),ImVec2(arrow+3,p.y+22),color);d->AddLine(ImVec2(arrow+3,p.y+22),ImVec2(arrow,p.y+25),color);
    if(active) d->AddRectFilled(ImVec2(p.x,p.y+13),ImVec2(p.x+2,p.y+31),Color(Accent()),1);
    ImGui::PopID();if(clicked) page=index;return clicked;
}
inline const char* ModuleName(int i) {const char* names[]={"Aim Assist","ESP","Velocity"};return names[i];}
inline bool ModuleVisible(int i) {
    if(ModuleFilter==1 && !Favorites[i]) return false;
    if(ModuleFilter>=2 && i!=ModuleFilter-2) return false;
    std::string query(Search),name(ModuleName(i));
    auto lower=[](unsigned char c){return char(std::tolower(c));};
    std::transform(query.begin(),query.end(),query.begin(),lower);
    std::transform(name.begin(),name.end(),name.begin(),lower);
    return name.find(query)!=std::string::npos;
}
inline void ModuleCard(int index,ImVec2 size) {
    bool* states[]={&settings::AA_Enabled,&settings::ESP_Enabled,&settings::Velocity_Enabled};
    bool* enabled=states[index];const auto a=Accent();
    ImGui::PushID(index);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,*enabled?ImVec4(a.x*.24f+.02f,a.y*.24f+.02f,a.z*.24f+.02f,1):ImVec4(.085f,.08f,.095f,1));
    ImGui::PushStyleColor(ImGuiCol_Border,*enabled?ImVec4(a.x,a.y,a.z,.45f):ImVec4(.16f,.15f,.18f,.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    ImGui::BeginChild("##card",size,true,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    const auto p=ImGui::GetWindowPos();auto* d=ImGui::GetWindowDrawList();
    Icon(d,ImVec2(p.x+24,p.y+25),index,*enabled?Color(a):IM_COL32(158,155,169,255));
    ImGui::SetCursorPos(ImVec2(size.x-49,16));
    if(ImGui::InvisibleButton("##enabled",ImVec2(33,18))) *enabled=!*enabled;
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s %s",*enabled?"Disable":"Enable",ModuleName(index));
    d->AddRectFilled(ImVec2(p.x+size.x-49,p.y+16),ImVec2(p.x+size.x-16,p.y+34),*enabled?Color(a):IM_COL32(57,54,64,255),9);
    d->AddCircleFilled(ImVec2(p.x+size.x-40+(*enabled?15:0),p.y+25),6,IM_COL32(238,241,244,255),20);
    ImGui::SetCursorPos(ImVec2(14,48));
    if(ImGui::InvisibleButton("##open",ImVec2(size.x-28,54))) page=index;
    if(ImGui::IsItemHovered()) {
        d->AddRect(ImVec2(p.x+1,p.y+1),ImVec2(p.x+size.x-1,p.y+size.y-1),Color(ImVec4(a.x,a.y,a.z,.6f)),7);
        ImGui::SetTooltip("Open %s settings",ModuleName(index));
    }
    ImFont* font=HeadingFont?HeadingFont:ImGui::GetFont();
    d->AddText(font,18,ImVec2(p.x+16,p.y+52),IM_COL32(236,234,242,255),ModuleName(index));
    const char* descriptions[]={"Targeting and precision","Player display and colors","Jump reset controls"};
    d->AddText(ImVec2(p.x+16,p.y+80),IM_COL32(145,143,157,255),descriptions[index]);
    ImGui::SetCursorPos(ImVec2(16,size.y-29));
    if(ImGui::SmallButton(Favorites[index]?"Saved":"+ Favorite")) Favorites[index]=!Favorites[index];
    ImGui::SetCursorPos(ImVec2(size.x-57,size.y-27));
    ImGui::TextColored(*enabled?a:ImVec4(.45f,.44f,.49f,1),"%s",*enabled?"On":"Off");
    ImGui::EndChild();ImGui::PopStyleVar();ImGui::PopStyleColor(2);ImGui::PopID();
}
inline void Dashboard() {
    const char* filters[]={"All","Favorites","Combat","Render","Movement"};
    if(ImGui::BeginTabBar("##moduleFilters")) {
        for(int i=0;i<5;i++) {
            if(ImGui::BeginTabItem(filters[i])) {
                ModuleFilter=i;ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::Spacing();ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##searchModules","Search modules...",Search,sizeof(Search));
    ImGui::Spacing();
    const float available=ImGui::GetContentRegionAvail().x;
    const int columns=available>=540?2:1;
    const float cardWidth=(available-(columns-1)*12)/columns;
    int count=0;
    for(int i=0;i<3;i++) if(ModuleVisible(i)) {
        if(count%columns) ImGui::SameLine(0,12);
        ModuleCard(i,ImVec2(cardWidth,146));++count;
    }
    if(!count) {
        ImGui::Spacing();ImGui::TextDisabled("No modules match this view.");
        if(ModuleFilter==1) ImGui::TextWrapped("Add a module to Favorites from the All tab.");
    }
    ImGui::Spacing();ImGui::TextDisabled("Click a card to configure. Switches only affect this preview.");
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
    const bool rail=ShowPreview && io.DisplaySize.x>=1050 && page<3;
    const float width=(std::min)(rail?1040.f:840.f,(std::max)(320.f,io.DisplaySize.x-24));
    const float height=(std::min)(610.f,(std::max)(240.f,io.DisplaySize.y-24));
    ImGui::SetNextWindowSize(ImVec2(width,height));
    static ImVec2 previousDisplay(0,0);
    const bool resized=previousDisplay.x!=io.DisplaySize.x || previousDisplay.y!=io.DisplaySize.y;
    previousDisplay=io.DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*.5f,io.DisplaySize.y*.5f),resized?ImGuiCond_Always:ImGuiCond_Appearing,ImVec2(.5f,.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    bool visible=ImGui::Begin("FUSION+##main",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    if(!visible) {ImGui::End();return;}
    auto o=ImGui::GetWindowPos();auto* d=ImGui::GetWindowDrawList();
    const float sidebar=180,contentX=200;
    d->AddRectFilled(ImVec2(o.x+1,o.y+1),ImVec2(o.x+sidebar,o.y+height-1),IM_COL32(20,22,23,255),10,ImDrawFlags_RoundCornersLeft);
    d->AddLine(ImVec2(o.x+sidebar,o.y+1),ImVec2(o.x+sidebar,o.y+height-1),IM_COL32(43,46,48,255));
    d->AddLine(ImVec2(o.x+1,o.y+65),ImVec2(o.x+width-1,o.y+65),IM_COL32(43,46,48,255));
    ImFont* brand=BrandFont?BrandFont:ImGui::GetFont();
    d->AddText(brand,25,ImVec2(o.x+20,o.y+18),IM_COL32(240,243,242,255),"FUSION");
    d->AddText(ImVec2(o.x+119,o.y+28),Color(Accent()),"studio");
    ImGui::SetCursorPos(ImVec2(width-86,22));
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
    if(ImGui::SmallButton("Style")) page=4;
    ImGui::SameLine();if(ImGui::SmallButton("x")) *open=false;
    ImGui::PopStyleColor();
    ImGui::SetCursorPos(ImVec2(12,82));
    ImGui::BeginChild("##navigation",ImVec2(sidebar-24,height-98),false,ImGuiWindowFlags_NoScrollbar);
    Nav("Modules",5);ImGui::Spacing();Nav("Combat",0);Nav("Render",1);Nav("Movement",2);
    ImGui::Spacing();ImGui::Separator();ImGui::Spacing();Nav("Profiles",3);Nav("Appearance",4);
    ImGui::SetCursorPosY((std::max)(ImGui::GetCursorPosY()+16,height-194));
    ImGui::Separator();ImGui::TextDisabled("%s",standalone?"Preview only":"INSERT / menu");
    if(ImGui::Button(standalone?"Close preview":"Detach",ImVec2(-1,30))) {
        if(standalone) {*running=false;*open=false;}else ImGui::OpenPopup("Detach Fusion?");
    }
    if(ImGui::BeginPopupModal("Detach Fusion?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Unload Fusion from this game session?");
        if(ImGui::Button("Detach now")) {*running=false;*open=false;ImGui::CloseCurrentPopup();}
        ImGui::SameLine();if(ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();ImGui::EndPopup();
    }
    ImGui::EndChild();
    const char* titles[]={"Aim Assist","Visuals","Velocity","Profiles","Appearance","Modules"};
    const char* descriptions[]={"COMBAT / AIM ASSIST","RENDER / PLAYER DISPLAY","MOVEMENT / JUMP RESET","WORKSPACE / PROFILES","WORKSPACE / PERSONALIZATION","WORKSPACE / MODULE LIBRARY"};
    ImGui::SetCursorPos(ImVec2(contentX,83));ImGui::TextDisabled("%s",descriptions[page]);
    ImGui::SetCursorPos(ImVec2(contentX,106));
    if(HeadingFont) ImGui::PushFont(HeadingFont);
    ImGui::TextUnformatted(titles[page]);if(HeadingFont) ImGui::PopFont();
    const float mainWidth=width-contentX-20-(rail?240.f:0);
    ImGui::SetCursorPos(ImVec2(contentX,146));
    ImGui::BeginChild("##settingsPanel",ImVec2(mainWidth,height-166),true);
    switch(page) {case 0:Aim();break;case 1:Visuals();break;case 2:Velocity();break;case 3:profiles();break;case 4:preferences();break;case 5:Dashboard();break;}
    ImGui::EndChild();
    if(rail) {
        ImGui::SetCursorPos(ImVec2(contentX+mainWidth+16,146));
        ImGui::BeginChild("##context",ImVec2(224,height-166),true);
        Preview();Section("WORKSPACE");
        int active=int(settings::AA_Enabled)+int(settings::ESP_Enabled)+int(settings::Velocity_Enabled);
        ImGui::Text("Enabled: %d / 3",active);ImGui::Text("Friends: %d",int(settings::friends.size()));
        ImGui::TextWrapped("%s",standalone?"Local preview. No game is connected.":"Save a profile to keep your settings.");ImGui::EndChild();
    }
    ImGui::End();
}
}
