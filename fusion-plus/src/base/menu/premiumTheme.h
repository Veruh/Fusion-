#pragma once
#include "imgui/imgui.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace FusionGui {
namespace Palette {
inline ImVec4 Rgb(unsigned hex,float alpha=1) {return ImVec4(float((hex>>16)&255)/255,float((hex>>8)&255)/255,float(hex&255)/255,alpha);}
inline const ImVec4 Outer=Rgb(0x0D0C0F),Main=Rgb(0x101012),Surface=Rgb(0x171518);
inline const ImVec4 Elevated=Rgb(0x1B181C),Interactive=Rgb(0x222024),Border=Rgb(0x2B282D);
inline const ImVec4 Emerald=Rgb(0x078B72),Active=Rgb(0x0A9B7E),Deep=Rgb(0x056653);
inline const ImVec4 Text=Rgb(0xF2F2F3),Secondary=Rgb(0x99979D),Muted=Rgb(0x66636B);
}
inline bool ReducedMotion=false;
inline bool ForceReducedMotion=false,SystemMotionEnabled=true;
inline double EntranceStart=-1,PageStart=-1,FilterStart=-1;
inline int LastPage=-1;
inline float IndicatorY=-1,IndicatorTarget=0;
inline float TabLeft=-1,TabRight=-1;
inline float LoadingVisual=0;
inline float Smooth(float value,float target,float dt,float seconds,bool reduced=false) {
    if(reduced || seconds<=0) return target;
    const float factor=1-std::exp(-4.6f*(std::max)(0.f,dt)/seconds);
    float result=value+(target-value)*factor;
    return std::abs(target-result)<.001f?target:result;
}
inline float Ease(float t) {t=std::clamp(t,0.f,1.f);return 1-(1-t)*(1-t)*(1-t);}
inline float Progress(double start,float seconds) {
    return ReducedMotion || start<0?1.f:Ease(float((ImGui::GetTime()-start)/seconds));
}
inline float Animate(const char* id,float target,float seconds,float initial=-1) {
    float* value=ImGui::GetStateStorage()->GetFloatRef(ImGui::GetID(id),initial<0?target:initial);
    *value=Smooth(*value,target,ImGui::GetIO().DeltaTime,seconds,ReducedMotion);return *value;
}
inline ImVec4 Mix(ImVec4 a,ImVec4 b,float t) {return ImVec4(a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t,a.w+(b.w-a.w)*t);}
inline ImVec4 EmeraldAccent(const float* rgb) {
    // Old profiles remain readable, but cannot reintroduce purple/neon into this identity.
    float h,s,v;ImGui::ColorConvertRGBtoHSV(rgb[0],rgb[1],rgb[2],h,s,v);
    if(!std::isfinite(h)||!std::isfinite(s)||!std::isfinite(v)||h<.37f||h>.48f||s<.35f) return Palette::Emerald;
    return ImVec4(std::clamp(rgb[0],0.f,.20f),std::clamp(rgb[1],.25f,.65f),std::clamp(rgb[2],.18f,.55f),1);
}
inline float BrandWidth(ImFont* font,float size,const char* text,float spacing) {
    float width=0;for(const char* p=text;*p;p++) {char glyph[2]={*p,0};width+=font->CalcTextSizeA(size,(std::numeric_limits<float>::max)(),0,glyph).x+spacing;}
    return width-spacing;
}
inline void DrawBrand(ImDrawList* draw,ImFont* font,ImVec2 p,float size,ImU32 ink,float spacing=1.1f) {
    for(const char* c="FUSION";*c;c++) {char glyph[2]={*c,0};draw->AddText(font,size,p,ink,glyph);p.x+=font->CalcTextSizeA(size,(std::numeric_limits<float>::max)(),0,glyph).x+spacing;}
}
}
