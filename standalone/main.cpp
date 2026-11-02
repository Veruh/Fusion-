#include <Windows.h>
#include <ShlObj.h>
#include <gl/GL.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"
#include "menu/fusionGui.h"
#include "profiles.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
std::filesystem::path profileRoot;
std::string notice;
std::string selected;
bool open=true,running=true;

void Refresh(std::vector<std::string>& files) {
    files.clear();
    for(const auto& f:std::filesystem::directory_iterator(profileRoot)) {
        const auto name=f.path().stem().string();
        if(f.is_regular_file() && f.path().extension()==".json" && name.rfind("profile-",0)==0)
            files.push_back(name.substr(8));
    }
    std::sort(files.begin(),files.end());
}
void Profiles() {
    static std::vector<std::string> files;
    static bool first=true;
    static char name[49]="";
    try {
        if(first) { Refresh(files); first=false; }
        FusionGui::Section("LOCAL PREVIEW PROFILES");
        ImGui::TextWrapped("Separate from Minecraft configs. Stores every preview control and the friend list.");
        if(ImGui::Button("Refresh")) Refresh(files);
        if(ImGui::BeginChild("##saved",ImVec2(0,120),true)) {
            if(files.empty()) ImGui::TextDisabled("No saved profiles yet.");
            for(const auto& file:files) if(ImGui::Selectable(file.c_str(),selected==file)) {
                selected=file; std::snprintf(name,sizeof(name),"%s",file.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::BeginDisabled(!StudioProfiles::ValidName(selected));
        if(ImGui::Button("Load selected",ImVec2(-1,34))) {
            const auto path=profileRoot/("profile-"+selected+".json");
            if(std::filesystem::file_size(path)>256*1024) throw std::runtime_error("Profile exceeds 256 KB.");
            std::ifstream input(path);
            if(!input) throw std::runtime_error("Could not open profile.");
            nlohmann::json data; input>>data; StudioProfiles::Decode(data);
            notice="Profile loaded.";
        }
        ImGui::EndDisabled();
        FusionGui::Section("SAVE CURRENT PREVIEW");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##name","Name: letters, numbers, spaces, - or _",name,sizeof(name));
        ImGui::BeginDisabled(!StudioProfiles::ValidName(name));
        if(ImGui::Button("Save profile",ImVec2(-1,34))) {
            const auto path=profileRoot/("profile-"+std::string(name)+".json");
            std::ofstream output(path,std::ios::trunc);
            if(!output) throw std::runtime_error("Could not create profile.");
            output<<StudioProfiles::Encode().dump(2); output.close();
            if(!output) throw std::runtime_error("Could not finish writing profile.");
            selected=name;Refresh(files);notice="Profile saved.";
        }
        ImGui::EndDisabled();
        ImGui::TextWrapped("Saving an existing name replaces that preview profile.");
    } catch(const std::exception& e) { notice=e.what(); }
    if(!notice.empty()) ImGui::TextWrapped("%s",notice.c_str());
}
void Preferences() {
    FusionGui::Section("PREVIEW APPEARANCE");
    FusionGui::Picker("Accent color",settings::Menu_AccentColor);
    ImGui::TextWrapped("Module switches only update local values. This executable does not connect to Minecraft.");
    FusionGui::Section("FRIENDS");
    static char friendName[49]="";
    ImGui::SetNextItemWidth(-1);ImGui::InputTextWithHint("##friend","Player name",friendName,sizeof(friendName));
    if(ImGui::Button("Add friend",ImVec2(-1,32)) && friendName[0]) {
        if(settings::friends.size()<128 && std::find(settings::friends.begin(),settings::friends.end(),friendName)==settings::friends.end())
            settings::friends.emplace_back(friendName);
        friendName[0]=0;
    }
    int remove=-1;
    for(int i=0;i<int(settings::friends.size());i++) {
        ImGui::PushID(i);if(ImGui::SmallButton("Remove")) remove=i;
        ImGui::SameLine();ImGui::TextUnformatted(settings::friends[i].c_str());ImGui::PopID();
    }
    if(remove>=0) settings::friends.erase(settings::friends.begin()+remove);
    ImGui::TextWrapped("Save a profile to persist the list. Profiles are stored in LocalAppData/FusionStudio/profiles.");
}
void Check(bool condition,const char* what) { if(!condition) throw std::runtime_error(what); }
void EmptyPanel() { ImGui::TextUnformatted("Test panel"); }
int SelfTest() {
    const auto original=StudioProfiles::Encode();
    Check(!StudioProfiles::ValidName("../escape"),"Path traversal accepted");
    Check(!StudioProfiles::ValidName(""),"Empty name accepted");
    Check(StudioProfiles::ValidName("My profile_01"),"Valid name rejected");
    settings::AA_smooth=42;settings::friends={"Example"};
    const auto saved=StudioProfiles::Encode();
    settings::AA_smooth=7;settings::friends.clear();StudioProfiles::Decode(saved);
    Check(settings::AA_smooth==42 && settings::friends.size()==1,"Profile roundtrip failed");
    auto bad=saved;bad["settings"]["AA_fov"]="wrong type";
    bool rejected=false;try {StudioProfiles::Decode(bad);}catch(const std::exception&) {rejected=true;}
    Check(rejected && StudioProfiles::Encode()==saved,"Malformed profile partially modified settings");
    StudioProfiles::Decode(original);
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
    io.Fonts->AddFontDefault();io.Fonts->Build();io.DeltaTime=1.f/60;
    for(auto size:{ImVec2(1280,800),ImVec2(854,480),ImVec2(720,1280)}) {
        io.DisplaySize=size;
        for(int page=0;page<5;page++) {
            FusionGui::page=page;
            for(int frame=0;frame<2;frame++) {
                ImGui::NewFrame();FusionGui::Render(EmptyPanel,EmptyPanel,&open,&running,true);ImGui::Render();
                auto* window=ImGui::FindWindowByName("FUSION+##main");
                Check(window && window->Pos.x>=0 && window->Pos.y>=0,"Window outside viewport");
                Check(window->Pos.x+window->Size.x<=size.x && window->Pos.y+window->Size.y<=size.y,"Window clipped by viewport");
                Check(ImGui::GetDrawData()->TotalVtxCount>0,"UI did not draw");
            }
        }
    }
    // Run the actual switch with synthetic mouse events; no OS input is injected.
    io.DisplaySize=ImVec2(1280,800);FusionGui::page=0;settings::AA_Enabled=false;
    for(int frame=0;frame<4;frame++) {
        io.AddMousePosEvent(840,257);
        if(frame==1) io.AddMouseButtonEvent(0,true);
        if(frame==2) io.AddMouseButtonEvent(0,false);
        ImGui::NewFrame();FusionGui::Render(EmptyPanel,EmptyPanel,&open,&running,true);ImGui::Render();
    }
    Check(settings::AA_Enabled,"Toggle interaction failed");
    ImGui::DestroyContext();std::puts("PASS: profiles, atomic validation, 15 UI layouts and switch interaction.");return 0;
}
LRESULT WINAPI WindowProc(HWND hwnd,UINT msg,WPARAM w,LPARAM l) {
    if(ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(hwnd,msg,w,l)) return 1;
    if(msg==WM_CLOSE) {running=false;return 0;}
    if(msg==WM_DESTROY) {PostQuitMessage(0);return 0;}
    if(msg==WM_GETMINMAXINFO) {auto* info=reinterpret_cast<MINMAXINFO*>(l);info->ptMinTrackSize={640,420};return 0;}
    if(msg==WM_SYSCOMMAND && (w&0xfff0)==SC_KEYMENU) return 0;
    return DefWindowProcW(hwnd,msg,w,l);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR command,int) {
    try {if(std::wstring(command).find(L"--self-test")!=std::wstring::npos) return SelfTest();}
    catch(const std::exception& e) {std::fprintf(stderr,"Self-test failed: %s\n",e.what());return 1;}
    PWSTR folder=nullptr;
    if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&folder))) return 1;
    profileRoot=std::filesystem::path(folder)/"FusionStudio"/"profiles";CoTaskMemFree(folder);
    try {std::filesystem::create_directories(profileRoot);}catch(...) {MessageBoxW(nullptr,L"Cannot create preview profile folder.",L"Fusion Studio",MB_ICONERROR);return 1;}
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSW wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=WindowProc;wc.hInstance=instance;
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.lpszClassName=L"FusionStudioWindow";
    if(!RegisterClassW(&wc)) return 1;
    HWND hwnd=CreateWindowW(wc.lpszClassName,L"Fusion Studio - standalone GUI preview",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1100,800,nullptr,nullptr,instance,nullptr);
    if(!hwnd) {UnregisterClassW(wc.lpszClassName,instance);return 1;}
    HDC dc=GetDC(hwnd);PIXELFORMATDESCRIPTOR pfd{};pfd.nSize=sizeof(pfd);pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;pfd.iPixelType=PFD_TYPE_RGBA;pfd.cColorBits=32;pfd.cAlphaBits=8;
    int pixel=ChoosePixelFormat(dc,&pfd);HGLRC gl=nullptr;
    if(pixel && SetPixelFormat(dc,pixel,&pfd)) gl=wglCreateContext(dc);
    if(!gl || !wglMakeCurrent(dc,gl)) {
        MessageBoxW(hwnd,L"Cannot initialize OpenGL.",L"Fusion Studio",MB_ICONERROR);
        if(gl) wglDeleteContext(gl);ReleaseDC(hwnd,dc);DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);return 1;
    }
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
    ImFontConfig config{};config.SizePixels=16;io.Fonts->AddFontDefault(&config);
    ImGui_ImplWin32_Init(hwnd);ImGui_ImplOpenGL2_Init();ShowWindow(hwnd,SW_SHOWDEFAULT);
    while(running && open) {
        MSG msg;while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {TranslateMessage(&msg);DispatchMessageW(&msg);if(msg.message==WM_QUIT) running=false;}
        if(!running) break;
        if(IsIconic(hwnd)) {Sleep(50);continue;}
        ImGui_ImplOpenGL2_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
        FusionGui::Render(Profiles,Preferences,&open,&running,true);ImGui::Render();
        glViewport(0,0,int(io.DisplaySize.x),int(io.DisplaySize.y));glClearColor(.025f,.03f,.05f,1);glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());SwapBuffers(dc);
        Sleep(16);
    }
    ImGui_ImplOpenGL2_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(gl);ReleaseDC(hwnd,dc);DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);
    return 0;
}
