#include <Windows.h>
#include <ShlObj.h>
#include <gl/GL.h>
#include <dwmapi.h>
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
#include "previewCapture.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
std::filesystem::path profileRoot;
std::string notice;
std::string selected;
bool open=true,running=true;
bool saveAppearanceOnExit=true;

// Remove only this application's known files; never recurse into directories.
std::vector<std::filesystem::path> LocalDataFiles() {
    const auto root=profileRoot.parent_path().lexically_normal();
    PWSTR local=nullptr;
    if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&local))) throw std::runtime_error("Cannot verify local data folder.");
    const auto expected=(std::filesystem::path(local)/"FusionStudio").lexically_normal();CoTaskMemFree(local);
    if(root!=expected || profileRoot.lexically_normal()!=expected/"profiles") throw std::runtime_error("Unexpected data folder. Nothing was deleted.");
    for(const auto& path:{root,profileRoot}) {
        const DWORD attrs=GetFileAttributesW(path.c_str());
        if(attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_REPARSE_POINT)) throw std::runtime_error("Linked data folders are not supported. Nothing was deleted.");
    }
    std::vector<std::filesystem::path> files;
    auto add=[&](const std::filesystem::path& path) {
        const DWORD attrs=GetFileAttributesW(path.c_str());
        if(attrs==INVALID_FILE_ATTRIBUTES) return;
        if((attrs&FILE_ATTRIBUTE_REPARSE_POINT) || (attrs&FILE_ATTRIBUTE_DIRECTORY)) throw std::runtime_error("Unexpected linked file or directory. Nothing was deleted.");
        files.push_back(path);
    };
    add(root/"appearance.json");
    if(std::filesystem::exists(profileRoot)) for(const auto& entry:std::filesystem::directory_iterator(profileRoot)) {
        const auto stem=entry.path().stem().string();
        if(entry.path().extension()==".json" && stem.rfind("profile-",0)==0 && StudioProfiles::ValidName(stem.substr(8))) add(entry.path());
    }
    return files;
}
void DeleteLocalData() {
    const auto files=LocalDataFiles(); // Validate the complete list before the first deletion.
    saveAppearanceOnExit=false;
    size_t failures=0;
    for(const auto& path:files) {std::error_code ec;std::filesystem::remove(path,ec);if(ec) ++failures;}
    // Empty directories only. Unknown files are preserved.
    for(const auto& path:{profileRoot,profileRoot.parent_path()}) {
        std::error_code ec;
        if(std::filesystem::is_directory(path,ec) && std::filesystem::is_empty(path,ec)) std::filesystem::remove(path,ec);
    }
    if(failures) MessageBoxW(nullptr,L"Some app files could not be deleted. The app will close without saving settings. The EXE was not changed.",L"Cleanup incomplete",MB_ICONWARNING);
    running=false;open=false;
}

// Commit a complete JSON file in the same directory without truncating the old one.
void WriteJson(const std::filesystem::path& path,const nlohmann::json& data) {
    const std::string bytes=data.dump(2);
    auto temporary=path;temporary+=L"."+std::to_wstring(GetCurrentProcessId())+L".tmp";
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE) throw std::runtime_error("Could not create temporary settings file.");
    DWORD written=0;
    const bool complete=WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr) && written==bytes.size() && FlushFileBuffers(file);
    CloseHandle(file);
    if(!complete || !MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary.c_str());throw std::runtime_error("Could not commit settings. Previous file was preserved.");
    }
}
void SaveAppearance() {
    nlohmann::json look;
    const auto accent=FusionGui::Accent();
    look["accent"]={accent.x,accent.y,accent.z,1};look["preview"]=FusionGui::ShowPreview;look["favorites"]=FusionGui::Favorites;
    look["reducedMotion"]=FusionGui::ForceReducedMotion;
    WriteJson(profileRoot.parent_path()/"appearance.json",look);
}
void LoadAppearance() {
    // Appearance is independent of gameplay/profile values and persists on normal exit.
    const auto base=FusionGui::Palette::Emerald;
    settings::Menu_AccentColor[0]=base.x;settings::Menu_AccentColor[1]=base.y;settings::Menu_AccentColor[2]=base.z;
    auto path=profileRoot.parent_path()/"appearance.json";
    if(!std::filesystem::exists(path)) return;
    if(std::filesystem::file_size(path)>4096) throw std::runtime_error("Appearance file exceeds 4 KB.");
    std::ifstream input(path);nlohmann::json look;input>>look;
    const auto& color=look.at("accent");
    if(!color.is_array() || color.size()!=4) throw std::runtime_error("Invalid accent color.");
    auto values=color.get<std::array<float,4>>();
    for(float v:values) if(!std::isfinite(v)||v<0||v>1) throw std::runtime_error("Invalid accent color.");
    const bool preview=look.at("preview").get<bool>();
    std::array<bool,3> favorites{};
    if(look.contains("favorites")) {
        const auto& list=look.at("favorites");
        if(!list.is_array() || list.size()!=3) throw std::runtime_error("Invalid favorites list.");
        favorites=list.get<std::array<bool,3>>();
    }
    const bool reduced=look.value("reducedMotion",false);
    const auto accent=FusionGui::EmeraldAccent(values.data());
    settings::Menu_AccentColor[0]=accent.x;settings::Menu_AccentColor[1]=accent.y;settings::Menu_AccentColor[2]=accent.z;settings::Menu_AccentColor[3]=1;
    FusionGui::ShowPreview=preview;FusionGui::ForceReducedMotion=reduced;
    std::copy(favorites.begin(),favorites.end(),FusionGui::Favorites);
}

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
            WriteJson(path,StudioProfiles::Encode());
            selected=name;Refresh(files);notice="Profile saved.";
        }
        ImGui::EndDisabled();
        ImGui::TextWrapped("Saving an existing name replaces that preview profile.");
    } catch(const std::exception& e) { notice=e.what(); }
    if(!notice.empty()) ImGui::TextWrapped("%s",notice.c_str());
}
void Preferences() {
    FusionGui::Section("ACCENT COLOR");
    const char* names[]={"Emerald","Jade","Forest"};
    const ImVec4 presets[]={FusionGui::Palette::Emerald,FusionGui::Palette::Active,FusionGui::Palette::Deep};
    for(int i=0;i<3;i++) {
        if(i) ImGui::SameLine();
        if(ImGui::ColorButton(names[i],presets[i],ImGuiColorEditFlags_NoTooltip,ImVec2(30,24))) {
            settings::Menu_AccentColor[0]=presets[i].x;settings::Menu_AccentColor[1]=presets[i].y;settings::Menu_AccentColor[2]=presets[i].z;
        }
        if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s",names[i]);
    }
    float intensity=FusionGui::Accent().y/FusionGui::Palette::Emerald.y;
    ImGui::TextUnformatted("Emerald intensity");ImGui::SetNextItemWidth(-1);
    if(ImGui::SliderFloat("##intensity",&intensity,.72f,1.18f,"%.2f",ImGuiSliderFlags_AlwaysClamp)) {
        settings::Menu_AccentColor[0]=FusionGui::Palette::Emerald.x*intensity;
        settings::Menu_AccentColor[1]=FusionGui::Palette::Emerald.y*intensity;
        settings::Menu_AccentColor[2]=FusionGui::Palette::Emerald.z*intensity;
    }
    FusionGui::Toggle("Illustration panel on wide windows",&FusionGui::ShowPreview);
    FusionGui::Toggle("Reduce motion",&FusionGui::ForceReducedMotion);
    FusionGui::ReducedMotion=!FusionGui::SystemMotionEnabled || FusionGui::ForceReducedMotion;
    ImGui::TextDisabled("Inter / emerald edition");
    if(!FusionGui::SystemMotionEnabled) ImGui::TextDisabled("Windows reduced motion is active.");
    if(ImGui::Button("Save appearance",ImVec2(-1,32))) {
        try {SaveAppearance();notice="Appearance saved.";}catch(const std::exception& e) {notice=e.what();}
    }
    ImGui::TextWrapped("Your color is also saved when you close the preview normally.");
    if(!notice.empty()) ImGui::TextWrapped("%s",notice.c_str());
    FusionGui::Section("WORKSPACE");
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
    FusionGui::Section("LOCAL DATA");
    ImGui::TextWrapped("Delete this preview's saved profiles and appearance, then close it. Keeps the EXE and any unrelated files.");
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(.96f,.45f,.49f,1));
    if(ImGui::Button("Delete local data and exit",ImVec2(-1,34))) ImGui::OpenPopup("Delete local data?");
    ImGui::PopStyleColor();
    if(ImGui::BeginPopupModal("Delete local data?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Permanently delete saved preview profiles and appearance?");
        ImGui::TextUnformatted("Keeps FusionStudio.exe. Windows and antivirus records are unchanged.");
        ImGui::Spacing();
        if(ImGui::Button("Delete and exit")) {
            try {DeleteLocalData();ImGui::CloseCurrentPopup();}
            catch(const std::exception& e) {notice=e.what();ImGui::CloseCurrentPopup();}
        }
        ImGui::SameLine();if(ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
void PrepareWorkspace() {
    PWSTR folder=nullptr;
    if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&folder))) throw std::runtime_error("Cannot find local application data.");
    profileRoot=std::filesystem::path(folder)/"FusionStudio"/"profiles";CoTaskMemFree(folder);
    std::filesystem::create_directories(profileRoot);
}
void Check(bool condition,const char* what) { if(!condition) throw std::runtime_error(what); }
void EmptyPanel() { ImGui::TextUnformatted("Test panel"); }
bool embeddedFonts=false;
void LoadFonts() {
    auto& io=ImGui::GetIO();
    ImFontConfig config{};config.FontDataOwnedByAtlas=false;config.OversampleH=3;config.OversampleV=2;
    auto add=[&](int id,float size) -> ImFont* {
        const auto module=GetModuleHandleW(nullptr);
        HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(id),RT_RCDATA);
        if(!resource) throw std::runtime_error("Embedded Inter resource missing.");
        auto data=LoadResource(module,resource);auto length=SizeofResource(module,resource);
        void* bytes=LockResource(data);
        if(!bytes || !length) throw std::runtime_error("Cannot read embedded font.");
        return io.Fonts->AddFontFromMemoryTTF(bytes,int(length),size,&config);
    };
    io.FontDefault=add(101,16);
    FusionGui::HeadingFont=add(102,22);
    FusionGui::BrandFont=add(102,25);
    if(!io.FontDefault || !FusionGui::HeadingFont || !FusionGui::BrandFont) throw std::runtime_error("Cannot prepare Inter font atlas.");
    embeddedFonts=true;
}
void UpdateMotionPreference() {
    BOOL enabled=TRUE;
    if(SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION,0,&enabled,0)) FusionGui::SystemMotionEnabled=enabled!=FALSE;
    FusionGui::ReducedMotion=!FusionGui::SystemMotionEnabled || FusionGui::ForceReducedMotion;
}
int SelfTest() {
    Check(FusionGui::Smooth(0,1,.016f,.2f,true)==1,"Reduced motion did not skip interpolation");
    const float motion=FusionGui::Smooth(0,1,.016f,.2f,false);
    Check(motion>0 && motion<1,"Animation is not bounded");
    float oldPurple[3]={.6f,.4f,1.f};const auto migrated=FusionGui::EmeraldAccent(oldPurple);
    Check(migrated.x==FusionGui::Palette::Emerald.x && migrated.y==FusionGui::Palette::Emerald.y,"Old purple accent was not normalized");
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
    bad=saved;bad["settings"]["Menu_AccentColor"].push_back(.5f);
    rejected=false;try {StudioProfiles::Decode(bad);}catch(const std::exception&) {rejected=true;}
    Check(rejected && StudioProfiles::Encode()==saved,"Oversized color array accepted");
    const auto testFile=std::filesystem::temp_directory_path()/("FusionStudio-selftest-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+".json");
    WriteJson(testFile,original);WriteJson(testFile,saved);
    {std::ifstream input(testFile);nlohmann::json actual;input>>actual;Check(actual==saved,"Atomic profile replacement failed");}
    Check(std::filesystem::remove(testFile),"Could not remove self-test file");
    const auto previousRoot=profileRoot;
    profileRoot=std::filesystem::temp_directory_path()/"NotFusionStudio"/"profiles";
    rejected=false;try {DeleteLocalData();}catch(const std::exception&) {rejected=true;}
    Check(rejected && running && saveAppearanceOnExit,"Cleanup accepted an unrelated folder");
    profileRoot=previousRoot;
    StudioProfiles::Decode(original);
    const auto renderRoot=std::filesystem::temp_directory_path()/("FusionStudio-render-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    std::filesystem::create_directory(renderRoot);profileRoot=renderRoot;
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
    LoadFonts();io.Fonts->Build();io.DeltaTime=1.f/60;settings::AA_Enabled=true;settings::ESP_Enabled=true;
    Check(embeddedFonts,"Inter was not loaded from embedded resources");
    io.DisplaySize=ImVec2(1280,800);
    FusionGui::LoadingVisual=.75f;
    ImGui::NewFrame();FusionGui::Startup(.75f,"Workspace prepared");ImGui::Render();
    StudioCapture::SaveFrame("gui-loading.ppm",1280,800);
    for(auto size:{ImVec2(1280,800),ImVec2(854,480),ImVec2(720,1280)}) {
        io.DisplaySize=size;
        for(int page=0;page<6;page++) {
            FusionGui::page=page;
            for(int frame=0;frame<24;frame++) {
                ImGui::NewFrame();FusionGui::Render(Profiles,Preferences,&open,&running,true);ImGui::Render();
                auto* window=ImGui::FindWindowByName("FUSION+##main");
                Check(window && window->Pos.x>=0 && window->Pos.y>=0,"Window outside viewport");
                Check(window->Pos.x+window->Size.x<=size.x && window->Pos.y+window->Size.y<=size.y,"Window clipped by viewport");
                Check(ImGui::GetDrawData()->TotalVtxCount>0,"UI did not draw");
                if(size.x==1280 && page==5 && frame==23) StudioCapture::SaveFrame("gui-preview.ppm",1280,800);
                if(size.x==1280 && page==4 && frame==23) StudioCapture::SaveFrame("gui-appearance.ppm",1280,800);
            }
        }
    }
    // Run the actual switch with synthetic mouse events; no OS input is injected.
    io.DisplaySize=ImVec2(1280,800);FusionGui::page=0;settings::AA_Enabled=false;
    for(int frame=0;frame<4;frame++) {
        io.AddMousePosEvent(1000,280);
        if(frame==1) io.AddMouseButtonEvent(0,true);
        if(frame==2) io.AddMouseButtonEvent(0,false);
        ImGui::NewFrame();FusionGui::Render(EmptyPanel,EmptyPanel,&open,&running,true);ImGui::Render();
    }
    Check(settings::AA_Enabled,"Toggle interaction failed");
    FusionGui::ModuleFilter=1;
    Check(!FusionGui::ModuleVisible(0) && !FusionGui::ModuleVisible(1),"Empty favorites filter failed");
    FusionGui::Favorites[1]=true;
    Check(FusionGui::ModuleVisible(1) && !FusionGui::ModuleVisible(0),"Favorites filter failed");
    FusionGui::ModuleFilter=0;
    std::snprintf(FusionGui::Search,sizeof(FusionGui::Search),"aIM");
    Check(FusionGui::ModuleVisible(0) && !FusionGui::ModuleVisible(1),"Case-insensitive search failed");
    FusionGui::Search[0]=0;
    FusionGui::ReducedMotion=true;FusionGui::EntranceStart=ImGui::GetTime();FusionGui::SelectPage(5);
    ImGui::NewFrame();FusionGui::Render(EmptyPanel,EmptyPanel,&open,&running,true);ImGui::Render();
    Check(FusionGui::Progress(FusionGui::EntranceStart,.42f)==1,"Reduced motion still fades the interface");
    profileRoot=previousRoot;Check(std::filesystem::remove(renderRoot),"Could not remove rendering test directory");
    ImGui::DestroyContext();std::puts("PASS: profiles, local cleanup boundaries, 18 layouts, filters, search and switch interaction.");return 0;
}
LRESULT WINAPI WindowProc(HWND hwnd,UINT msg,WPARAM w,LPARAM l) {
    if(ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(hwnd,msg,w,l)) return 1;
    if(msg==WM_CLOSE) {running=false;return 0;}
    if(msg==WM_DESTROY) {PostQuitMessage(0);return 0;}
    if(msg==WM_SETTINGCHANGE) UpdateMotionPreference();
    if(msg==WM_GETMINMAXINFO) {auto* info=reinterpret_cast<MINMAXINFO*>(l);info->ptMinTrackSize={640,420};return 0;}
    if(msg==WM_SYSCOMMAND && (w&0xfff0)==SC_KEYMENU) return 0;
    return DefWindowProcW(hwnd,msg,w,l);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR command,int) {
    try {if(std::wstring(command).find(L"--self-test")!=std::wstring::npos) return SelfTest();}
    catch(const std::exception& e) {std::fprintf(stderr,"Self-test failed: %s\n",e.what());return 1;}
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSW wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=WindowProc;wc.hInstance=instance;
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.lpszClassName=L"FusionStudioWindow";
    if(!RegisterClassW(&wc)) return 1;
    HWND hwnd=CreateWindowW(wc.lpszClassName,L"Fusion Studio",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,920,720,nullptr,nullptr,instance,nullptr);
    if(!hwnd) {UnregisterClassW(wc.lpszClassName,instance);return 1;}
    BOOL dark=TRUE;DwmSetWindowAttribute(hwnd,DWMWA_USE_IMMERSIVE_DARK_MODE,&dark,sizeof(dark));
    HDC dc=GetDC(hwnd);PIXELFORMATDESCRIPTOR pfd{};pfd.nSize=sizeof(pfd);pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;pfd.iPixelType=PFD_TYPE_RGBA;pfd.cColorBits=32;pfd.cAlphaBits=8;
    int pixel=ChoosePixelFormat(dc,&pfd);HGLRC gl=nullptr;
    if(pixel && SetPixelFormat(dc,pixel,&pfd)) gl=wglCreateContext(dc);
    if(!gl || !wglMakeCurrent(dc,gl)) {
        MessageBoxW(hwnd,L"Cannot initialize OpenGL.",L"Fusion Studio",MB_ICONERROR);
        if(gl) wglDeleteContext(gl);ReleaseDC(hwnd,dc);DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);return 1;
    }
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
    ImFontConfig fallback{};fallback.SizePixels=16;io.Fonts->AddFontDefault(&fallback);
    ImGui_ImplWin32_Init(hwnd);ImGui_ImplOpenGL2_Init();ShowWindow(hwnd,SW_SHOWDEFAULT);
    UpdateMotionPreference();
    int startupStage=0;bool workspaceReady=false;
    while(running && open) {
        MSG msg;while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {TranslateMessage(&msg);DispatchMessageW(&msg);if(msg.message==WM_QUIT) running=false;}
        if(!running) break;
        if(IsIconic(hwnd)) {Sleep(50);continue;}
        try {
            if(startupStage==1) {
                ImGui_ImplOpenGL2_DestroyFontsTexture();io.Fonts->Clear();io.FontDefault=nullptr;
                FusionGui::HeadingFont=nullptr;FusionGui::BrandFont=nullptr;LoadFonts();io.Fonts->Build();
            }
            if(startupStage==2) {PrepareWorkspace();workspaceReady=true;}
            if(startupStage==3) {
                try {LoadAppearance();}catch(const std::exception& e) {notice=e.what();}
                UpdateMotionPreference();
            }
        } catch(const std::exception& e) {MessageBoxA(hwnd,e.what(),"Startup failed",MB_ICONERROR);running=false;break;}
        ImGui_ImplOpenGL2_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
        if(startupStage<4) {
            const char* completed[]={"Graphics initialized","Typography prepared","Workspace prepared","Preferences ready"};
            FusionGui::Startup(float(startupStage+1)/4,completed[startupStage]);
        } else FusionGui::Render(Profiles,Preferences,&open,&running,true);
        ImGui::Render();
        glViewport(0,0,int(io.DisplaySize.x),int(io.DisplaySize.y));glClearColor(13/255.f,12/255.f,15/255.f,1);glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());SwapBuffers(dc);
        if(startupStage<4) {
            ++startupStage;
            if(startupStage==4) FusionGui::EntranceStart=ImGui::GetTime();
        } else Sleep(16);
    }
    if(workspaceReady && saveAppearanceOnExit) try {SaveAppearance();}catch(const std::exception& e) {MessageBoxA(hwnd,e.what(),"Could not save appearance",MB_ICONWARNING);}
    ImGui_ImplOpenGL2_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(gl);ReleaseDC(hwnd,dc);DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);
    return 0;
}
