#pragma once
#include "imgui/imgui.h"
#include "configManager/settings.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include "premiumTheme.h"

// Shared by the injected menu and the standalone, non-injecting visual preview.
namespace FusionGui {
inline int page = 5;
inline void SelectPage(int next) {if(next!=page) {page=next;PageStart=ImGui::GetTime();}}
inline bool Favorites[3]={false,false,false};
inline int ModuleFilter=0;
inline char Search[64]="";
inline ImFont* HeadingFont=nullptr;
inline ImFont* BrandFont=nullptr;
inline bool ShowPreview=false;
inline ImVec4 Accent() { return EmeraldAccent(settings::Menu_AccentColor); }
inline ImU32 Color(ImVec4 c) { c.w*=ImGui::GetStyle().Alpha;return ImGui::ColorConvertFloat4ToU32(c); }
inline void Theme() {
    auto& s = ImGui::GetStyle();
    s.WindowRounding = 9; s.ChildRounding = 6; s.FrameRounding = 4;
    s.PopupRounding = 8; s.GrabRounding = 6; s.ScrollbarRounding = 6;
    s.WindowBorderSize = 1; s.ChildBorderSize = 1; s.FrameBorderSize = 0;
    s.WindowPadding = ImVec2(20,18); s.FramePadding = ImVec2(10,5);
    s.ItemSpacing = ImVec2(12,10); s.ItemInnerSpacing = ImVec2(8,6);
    s.ScrollbarSize = 9; s.GrabMinSize = 12;
    auto* c = s.Colors;
    c[ImGuiCol_Text] = Palette::Text;
    c[ImGuiCol_TextDisabled] = Palette::Secondary;
    c[ImGuiCol_WindowBg] = Palette::Main;
    c[ImGuiCol_ChildBg] = Palette::Surface;
    c[ImGuiCol_PopupBg] = Palette::Elevated;
    c[ImGuiCol_Border] = Palette::Rgb(0x2B282D,.72f);
    c[ImGuiCol_FrameBg] = Palette::Interactive;
    c[ImGuiCol_FrameBgHovered] = Palette::Rgb(0x2A272D);
    c[ImGuiCol_FrameBgActive] = Palette::Rgb(0x302C33);
    c[ImGuiCol_Button] = Palette::Interactive;
    c[ImGuiCol_ButtonHovered] = Palette::Rgb(0x2A272D);
    c[ImGuiCol_ButtonActive] = Accent();
    c[ImGuiCol_Header] = ImVec4(Accent().x,Accent().y,Accent().z,.12f);
    c[ImGuiCol_HeaderHovered] = ImVec4(Accent().x,Accent().y,Accent().z,.2f);
    c[ImGuiCol_HeaderActive] = Accent();
    c[ImGuiCol_SliderGrab] = c[ImGuiCol_SliderGrabActive] = c[ImGuiCol_CheckMark] = Accent();
    c[ImGuiCol_Separator] = Palette::Rgb(0x2B282D,.7f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0,0,0,0);
    c[ImGuiCol_ScrollbarGrab] = Palette::Rgb(0x363139);
    c[ImGuiCol_NavHighlight] = Accent();
    c[ImGuiCol_Tab]=Palette::Surface;
    c[ImGuiCol_TabActive]=Palette::Interactive;
    c[ImGuiCol_TabHovered]=ImVec4(Accent().x,Accent().y,Accent().z,.23f);
}
inline void Section(const char* title) {
    ImGui::Spacing(); ImGui::TextColored(Palette::Secondary, "%s", title);
    ImGui::Separator(); ImGui::Spacing();
}
inline void Toggle(const char* label, bool* value) {
    ImGui::PushID(label);
    const auto p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    const bool pressed = ImGui::InvisibleButton("##toggle", ImVec2(w,32));
    if (pressed) *value = !*value;
    auto* d = ImGui::GetWindowDrawList();
    const float hover=Animate("##hover",ImGui::IsItemHovered()?1.f:0.f,.14f);
    if(hover>0) d->AddRectFilled(p,ImVec2(p.x+w,p.y+32),Color(Palette::Rgb(0xFFFFFF,.025f*hover)),4);
    d->AddText(ImVec2(p.x,p.y+7),ImGui::GetColorU32(ImGuiCol_Text),label);
    const float x = p.x+w-34, y=p.y+7;
    const float t=Animate("##animation",*value?1.f:0.f,.21f);
    d->AddRectFilled(ImVec2(x,y),ImVec2(x+34,y+18),Color(Mix(Palette::Rgb(0x363239),Accent(),t)),9);
    d->AddCircleFilled(ImVec2(x+9+t*16,y+9),6.5f,Color(Mix(Palette::Rgb(0xA5A1AA),Palette::Text,t)),20);
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
    const auto p=ImGui::GetCursorScreenPos();const float localY=ImGui::GetCursorPosY();
    const float w=ImGui::GetContentRegionAvail().x;const bool active=page==index;
    bool clicked=ImGui::InvisibleButton("##navigation",ImVec2(w,42));
    const float hover=Animate("##hover",ImGui::IsItemHovered()?1.f:0.f,.15f);
    const float selected=Animate("##selected",active?1.f:0.f,.23f);
    auto* d=ImGui::GetWindowDrawList();
    d->AddRectFilled(p,ImVec2(p.x+w,p.y+42),Color(Mix(Palette::Main,Palette::Interactive,.3f*hover+.32f*selected)),4);
    const auto ink=Color(Mix(Palette::Secondary,Accent(),selected));
    Icon(d,ImVec2(p.x+18,p.y+21),index,ink);
    d->AddText(ImVec2(p.x+40,p.y+12),Color(Mix(Palette::Secondary,Palette::Text,selected)),label);
    const float arrow=p.x+w-15;
    d->AddLine(ImVec2(arrow,p.y+18),ImVec2(arrow+3,p.y+21),ink);d->AddLine(ImVec2(arrow+3,p.y+21),ImVec2(arrow,p.y+24),ink);
    if(active) IndicatorTarget=localY+12;
    ImGui::PopID();if(clicked) SelectPage(index);return clicked;
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
    const float on=Animate("##card-on",*enabled?1.f:0.f,.21f);
    const float reveal=ReducedMotion?1.f:Progress(FilterStart,.20f+index*.02f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha,ImGui::GetStyle().Alpha*(.5f+.5f*reveal));
    const auto cardOrigin=ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(cardOrigin.x,cardOrigin.y+3),ImVec2(cardOrigin.x+size.x,cardOrigin.y+size.y+3),Color(Palette::Rgb(0x000000,.14f)),6);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,Palette::Elevated);
    ImGui::PushStyleColor(ImGuiCol_Border,Mix(Palette::Rgb(0x2B282D,.6f),ImVec4(a.x,a.y,a.z,.26f),on));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    ImGui::BeginChild("##card",size,true,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    const auto p=ImGui::GetWindowPos();auto* d=ImGui::GetWindowDrawList();
    Icon(d,ImVec2(p.x+24,p.y+25),index,Color(Mix(Palette::Secondary,a,on*.8f)));
    ImGui::SetCursorPos(ImVec2(size.x-49,16));
    if(ImGui::InvisibleButton("##enabled",ImVec2(33,18))) *enabled=!*enabled;
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s %s",*enabled?"Disable":"Enable",ModuleName(index));
    d->AddRectFilled(ImVec2(p.x+size.x-49,p.y+16),ImVec2(p.x+size.x-16,p.y+34),Color(Mix(Palette::Rgb(0x39353D),a,on)),9);
    d->AddCircleFilled(ImVec2(p.x+size.x-40+on*15,p.y+25),6,Color(Mix(Palette::Rgb(0xA5A1AA),Palette::Text,on)),20);
    ImGui::SetCursorPos(ImVec2(14,48));
    if(ImGui::InvisibleButton("##open",ImVec2(size.x-28,54))) SelectPage(index);
    const float hover=Animate("##card-hover",ImGui::IsItemHovered()?1.f:0.f,.15f);
    if(hover>.01f) d->AddRect(ImVec2(p.x+1,p.y+1),ImVec2(p.x+size.x-1,p.y+size.y-1),Color(Palette::Rgb(0xFFFFFF,.07f*hover)),6);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("Open %s settings",ModuleName(index));
    ImFont* font=HeadingFont?HeadingFont:ImGui::GetFont();
    d->AddText(font,17,ImVec2(p.x+16,p.y+52),Color(Palette::Text),ModuleName(index));
    const char* descriptions[]={"Targeting and precision","Player display and colors","Jump reset controls"};
    d->AddText(ImVec2(p.x+16,p.y+79),Color(Palette::Secondary),descriptions[index]);
    ImGui::SetCursorPos(ImVec2(16,size.y-30));
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Text,Favorites[index]?a:Palette::Muted);
    if(ImGui::SmallButton(Favorites[index]?"Favorited":"+ Favorite")) Favorites[index]=!Favorites[index];
    ImGui::PopStyleColor(2);
    d->AddCircleFilled(ImVec2(p.x+size.x-49,p.y+size.y-20),2.5f,Color(Mix(Palette::Muted,a,on)),12);
    d->AddText(ImVec2(p.x+size.x-40,p.y+size.y-27),Color(Mix(Palette::Muted,a,on)),*enabled?"On":"Off");
    ImGui::EndChild();ImGui::PopStyleVar();ImGui::PopStyleColor(2);ImGui::PopStyleVar();ImGui::PopID();
}
inline void Dashboard() {
    const char* filters[]={"Todos","Favoritos","Combate","Renderizado","Movimiento"};
    float activeLeft=0,activeRight=0;const float underlineY=ImGui::GetCursorScreenPos().y+32;
    for(int i=0;i<5;i++) {
        if(i) ImGui::SameLine(0,4);
        ImGui::PushID(i);const auto p=ImGui::GetCursorScreenPos();
        const float w=ImGui::CalcTextSize(filters[i]).x+20;
        if(ImGui::InvisibleButton("##tab",ImVec2(w,34))) {ModuleFilter=i;FilterStart=ImGui::GetTime();}
        const float hover=Animate("##tab-hover",ImGui::IsItemHovered()?1.f:0.f,.14f);
        auto* draw=ImGui::GetWindowDrawList();
        if(hover>.01f) draw->AddRectFilled(p,ImVec2(p.x+w,p.y+30),Color(Palette::Rgb(0xFFFFFF,.025f*hover)),4);
        draw->AddText(ImVec2(p.x+10,p.y+7),Color(ModuleFilter==i?Palette::Text:Palette::Secondary),filters[i]);
        if(ModuleFilter==i) {activeLeft=p.x+10;activeRight=p.x+w-10;}
        ImGui::PopID();
    }
    if(TabLeft<0 || ReducedMotion) {TabLeft=activeLeft;TabRight=activeRight;}
    TabLeft=Smooth(TabLeft,activeLeft,ImGui::GetIO().DeltaTime,.23f,ReducedMotion);
    TabRight=Smooth(TabRight,activeRight,ImGui::GetIO().DeltaTime,.23f,ReducedMotion);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(TabLeft,underlineY),ImVec2(TabRight,underlineY),Color(Accent()),2);
    ImGui::Spacing();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::InputTextWithHint("##searchModules","Buscar modulos...",Search,sizeof(Search))) FilterStart=ImGui::GetTime();
    const float focus=Animate("##search-focus",ImGui::IsItemActive()?1.f:0.f,.16f);
    if(focus>.01f) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(),ImGui::GetItemRectMax(),Color(ImVec4(Accent().x,Accent().y,Accent().z,.45f*focus)),4);
    ImGui::Spacing();
    const float available=ImGui::GetContentRegionAvail().x;
    const int columns=available>=540?2:1;
    const float cardWidth=(available-(columns-1)*22)/columns;
    int count=0;
    for(int i=0;i<3;i++) if(ModuleVisible(i)) {
        if(count%columns) ImGui::SameLine(0,22);
        ModuleCard(i,ImVec2(cardWidth,182));++count;
    }
    if(!count) {
        ImGui::Spacing();ImGui::TextDisabled("No modules match this view.");
        if(ModuleFilter==1) ImGui::TextWrapped("Add a module to Favorites from the All tab.");
    }
    ImGui::Spacing();ImGui::TextDisabled("Open a card to configure. Local preview only.");
}
inline void Startup(float completed,const char* status,float alpha=1) {
    Theme();auto* d=ImGui::GetBackgroundDrawList();const auto display=ImGui::GetIO().DisplaySize;
    auto ink=[&](unsigned hex,float a=1){return Color(Palette::Rgb(hex,a*alpha));};
    const float cx=display.x*.5f,cy=display.y*.5f;
    // Very low contrast planes give the loading surface depth without neon effects.
    d->AddTriangleFilled(ImVec2(cx-390,cy-230),ImVec2(cx+140,cy-260),ImVec2(cx-60,cy+230),ink(0x141216,.7f));
    d->AddTriangleFilled(ImVec2(cx+380,cy-180),ImVec2(cx+320,cy+230),ImVec2(cx-140,cy+220),ink(0x19161B,.45f));
    const float w=(std::min)(460.f,display.x-40),h=208;
    ImVec2 p(cx-w*.5f,cy-h*.5f);
    d->AddRectFilled(ImVec2(p.x-3,p.y+7),ImVec2(p.x+w+3,p.y+h+9),ink(0x000000,.25f),10);
    d->AddRectFilled(p,ImVec2(p.x+w,p.y+h),ink(0x101012),8);
    d->AddRect(p,ImVec2(p.x+w,p.y+h),ink(0x2B282D,.7f),8);
    auto* font=BrandFont?BrandFont:ImGui::GetFont();
    const float brandWidth=BrandWidth(font,29,"FUSION",1.3f);
    DrawBrand(d,font,ImVec2(cx-brandWidth*.5f,p.y+34),29,ink(0xF2F2F3),1.3f);
    const char* subtitle="STUDIO";const float subWidth=ImGui::CalcTextSize(subtitle).x;
    d->AddText(ImVec2(cx-subWidth*.5f,p.y+75),ink(0x99979D),subtitle);
    const float completedProgress=std::clamp(completed,0.f,1.f),barWidth=w-64;
    LoadingVisual=Smooth(LoadingVisual,completedProgress,ImGui::GetIO().DeltaTime,.20f,ReducedMotion);
    const float progress=(std::min)(LoadingVisual,completedProgress);
    d->AddRectFilled(ImVec2(p.x+32,p.y+121),ImVec2(p.x+w-32,p.y+126),ink(0x222024),2.5f);
    if(progress>0) {
        d->AddRectFilled(ImVec2(p.x+31,p.y+119),ImVec2(p.x+33+barWidth*progress,p.y+128),ink(0x078B72,.065f),4);
        d->AddRectFilled(ImVec2(p.x+32,p.y+121),ImVec2(p.x+32+barWidth*progress,p.y+126),ink(0x078B72),2.5f);
    }
    d->AddText(ImVec2(p.x+32,p.y+146),ink(0x99979D),status);
    char percentage[12];std::snprintf(percentage,sizeof(percentage),"%d%%",int(completedProgress*100));
    const float numberWidth=ImGui::CalcTextSize(percentage).x;
    d->AddText(ImVec2(p.x+w-32-numberWidth,p.y+146),ink(0x66636B),percentage);
}
inline void Preview() {
    Section("STYLE PREVIEW");
    ImGui::TextDisabled("Illustration / not a live target");
    ImVec2 p=ImGui::GetCursorScreenPos();
    const float width=ImGui::GetContentRegionAvail().x;
    ImGui::Dummy(ImVec2(width,196));
    auto* d=ImGui::GetWindowDrawList();
    d->AddRectFilled(p,ImVec2(p.x+width,p.y+196),Color(Palette::Main),8);
    float cx=p.x+width*.5f, cy=p.y+96;
    const auto a=Color(Accent());
    d->AddLine(ImVec2(p.x+15,p.y+171),ImVec2(p.x+width-15,p.y+171),Color(Palette::Border));
    d->AddRectFilled(ImVec2(cx-15,cy-45),ImVec2(cx+15,cy-15),Color(Palette::Rgb(0x625D66)),3);
    d->AddRectFilled(ImVec2(cx-20,cy-10),ImVec2(cx+20,cy+34),Color(Palette::Rgb(0x454049)),3);
    d->AddRectFilled(ImVec2(cx-20,cy+38),ImVec2(cx-3,cy+68),Color(Palette::Rgb(0x363239)),2);
    d->AddRectFilled(ImVec2(cx+3,cy+38),ImVec2(cx+20,cy+68),Color(Palette::Rgb(0x363239)),2);
    if(page==1) {
        if(settings::ESP_FilledBox) d->AddRectFilled(ImVec2(cx-31,cy-53),ImVec2(cx+31,cy+73),Color(ImVec4(Accent().x,Accent().y,Accent().z,.12f)));
        if(settings::ESP_Box || settings::ESP_BoxType==1) d->AddRect(ImVec2(cx-31,cy-53),ImVec2(cx+31,cy+73),a,3,0,1.5f);
        if(settings::ESP_HealthBar) d->AddLine(ImVec2(cx-38,cy+72),ImVec2(cx-38,cy-52),Color(ImVec4(59/255.f,223/255.f,169/255.f,255/255.f)),3);
        if(settings::ESP_Text) d->AddText(ImVec2(cx-34,cy-75),Color(Palette::Text),"Player");
    } else {
        d->AddCircle(ImVec2(cx,cy),58,a,64,1.5f);
        d->AddLine(ImVec2(cx-70,cy),ImVec2(cx-48,cy),a,1.5f);
        d->AddLine(ImVec2(cx+48,cy),ImVec2(cx+70,cy),a,1.5f);
        d->AddLine(ImVec2(cx,cy-70),ImVec2(cx,cy-48),a,1.5f);
        d->AddCircleFilled(ImVec2(cx,cy),3,Color(Palette::Text));
    }
}
// Callback functions keep filesystem operations out of the presentation layer.
inline void Render(void (*profiles)(),void (*preferences)(),bool* open,bool* running,bool standalone=false) {
    Theme();
    if(LastPage!=page) {LastPage=page;PageStart=ImGui::GetTime();}
    const float entrance=Progress(EntranceStart,.42f);
    if(entrance<1) Startup(1,"Workspace ready",1-entrance);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha,ImGui::GetStyle().Alpha*entrance);
    ImGuiIO& io=ImGui::GetIO();
    const bool rail=ShowPreview && io.DisplaySize.x>=1050 && page<3;
    const float width=(std::min)(rail?1340.f:1120.f,(std::max)(320.f,io.DisplaySize.x-24));
    const float height=(std::min)(780.f,(std::max)(240.f,io.DisplaySize.y-24));
    ImGui::SetNextWindowSize(ImVec2(width,height));
    static ImVec2 previousDisplay(0,0);
    const bool resized=previousDisplay.x!=io.DisplaySize.x || previousDisplay.y!=io.DisplaySize.y;
    previousDisplay=io.DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*.5f,io.DisplaySize.y*.5f),resized?ImGuiCond_Always:ImGuiCond_Appearing,ImVec2(.5f,.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    bool visible=ImGui::Begin("FUSION+##main",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    if(!visible) {ImGui::End();ImGui::PopStyleVar();return;}
    auto o=ImGui::GetWindowPos();auto* d=ImGui::GetWindowDrawList();
    const float sidebar=260,contentX=292;
    d->AddRectFilled(ImVec2(o.x+1,o.y+1),ImVec2(o.x+sidebar,o.y+height-1),Color(Palette::Main),9,ImDrawFlags_RoundCornersLeft);
    d->AddLine(ImVec2(o.x+sidebar,o.y+1),ImVec2(o.x+sidebar,o.y+height-1),Color(Palette::Border));
    d->AddLine(ImVec2(o.x+1,o.y+65),ImVec2(o.x+width-1,o.y+65),Color(Palette::Border));
    ImFont* brand=BrandFont?BrandFont:ImGui::GetFont();
    DrawBrand(d,brand,ImVec2(o.x+24,o.y+18),24,Color(Palette::Text),.7f);
    const ImVec2 studioMin(o.x+174,o.y+21),studioMax(o.x+235,o.y+49);
    d->AddRectFilled(studioMin,studioMax,Color(Palette::Rgb(0x063326,.9f)),12);
    d->AddRect(studioMin,studioMax,Color(Palette::Rgb(0x00E99A,.9f)),12,0,1.5f);
    d->AddText(ImVec2(studioMin.x+9,studioMin.y+6),Color(Palette::Rgb(0x00E99A)),"Studio");
    ImGui::SetCursorPos(ImVec2(width-86,22));
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
    if(ImGui::SmallButton("Style")) SelectPage(4);
    ImGui::SameLine();if(ImGui::SmallButton("x")) *open=false;
    ImGui::PopStyleColor();
    ImGui::SetCursorPos(ImVec2(12,82));
    ImGui::BeginChild("##navigation",ImVec2(sidebar-24,height-98),false,ImGuiWindowFlags_NoScrollbar);
    Nav("Modulos",5);ImGui::Spacing();Nav("Combate",0);Nav("Renderizado",1);Nav("Movimiento",2);
    ImGui::Spacing();ImGui::Separator();ImGui::Spacing();Nav("Perfiles",3);Nav("Apariencia",4);
    if(IndicatorY<0) IndicatorY=IndicatorTarget;
    IndicatorY=Smooth(IndicatorY,IndicatorTarget,io.DeltaTime,.24f,ReducedMotion);
    const auto navOrigin=ImGui::GetWindowPos();
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(navOrigin.x,navOrigin.y+IndicatorY),ImVec2(navOrigin.x+2,navOrigin.y+IndicatorY+18),Color(Accent()),1);
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
    const char* titles[]={"Combate","Renderizado","Movimiento","Perfiles","Apariencia","Modulos"};
    const char* descriptions[]={"ESPACIO DE TRABAJO / COMBATE","ESPACIO DE TRABAJO / RENDERIZADO","ESPACIO DE TRABAJO / MOVIMIENTO","ESPACIO DE TRABAJO / PERFILES","ESPACIO DE TRABAJO / APARIENCIA","ESPACIO DE TRABAJO / BIBLIOTECA DE MODULOS"};
    ImGui::SetCursorPos(ImVec2(contentX,83));ImGui::TextDisabled("%s",descriptions[page]);
    ImGui::SetCursorPos(ImVec2(contentX,106));
    if(HeadingFont) ImGui::PushFont(HeadingFont);
    ImGui::TextUnformatted(titles[page]);if(HeadingFont) ImGui::PopFont();
    const float mainWidth=width-contentX-20-(rail?240.f:0);
    d->AddLine(ImVec2(o.x+contentX,o.y+140),ImVec2(o.x+contentX+130,o.y+140),Color(Palette::Rgb(0x00E99A,.8f)),2);
    ImGui::SetCursorPos(ImVec2(contentX,158));
    const float scene=Progress(PageStart,.26f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha,ImGui::GetStyle().Alpha*(.40f+.60f*scene));
    ImGui::SetCursorPosY(158+(ReducedMotion?0:5*(1-scene)));
    ImGui::BeginChild("##settingsPanel",ImVec2(mainWidth,height-178-(ReducedMotion?0:5*(1-scene))),true);
    switch(page) {case 0:Aim();break;case 1:Visuals();break;case 2:Velocity();break;case 3:profiles();break;case 4:preferences();break;case 5:Dashboard();break;}
    ImGui::EndChild();
    ImGui::PopStyleVar();
    if(rail) {
        ImGui::SetCursorPos(ImVec2(contentX+mainWidth+16,146));
        ImGui::BeginChild("##context",ImVec2(224,height-166),true);
        Preview();Section("WORKSPACE");
        int active=int(settings::AA_Enabled)+int(settings::ESP_Enabled)+int(settings::Velocity_Enabled);
        ImGui::Text("Enabled: %d / 3",active);ImGui::Text("Friends: %d",int(settings::friends.size()));
        ImGui::TextWrapped("%s",standalone?"Local preview. No game is connected.":"Save a profile to keep your settings.");ImGui::EndChild();
    }
    // Ambient edge light and understated layered highlights.
    for(int layer=5;layer>=1;--layer) {
        const float inset=float(layer)*1.7f;
        d->AddRect(ImVec2(o.x+inset,o.y+inset),ImVec2(o.x+width-inset,o.y+height-inset),
            Color(Palette::Rgb(0x00E99A,.012f*(6-layer))),11);
    }
    d->AddRect(ImVec2(o.x+1,o.y+1),ImVec2(o.x+width-1,o.y+height-1),Color(Palette::Rgb(0x00E99A,.22f)),10);
    ImGui::End();ImGui::PopStyleVar();
}
}
