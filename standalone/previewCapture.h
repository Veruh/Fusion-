#pragma once
#include <vector>
#include <cmath>
#include <cstdio>
#include <cassert>
namespace StudioCapture {
static float Edge(ImVec2 a,ImVec2 b,float x,float y) { return (x-a.x)*(b.y-a.y)-(y-a.y)*(b.x-a.x); }
static void SaveFrame(const char* path,int w,int h) {
    unsigned char* atlas; int tw,th;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&atlas,&tw,&th);
    std::vector<unsigned char> pixels(w*h*3);
    for(int y=0;y<h;y++) for(int x=0;x<w;x++) {
        int i=(y*w+x)*3;
        pixels[i]=9; pixels[i+1]=12; pixels[i+2]=20;
    }
    auto* data=ImGui::GetDrawData();
    for(int l=0;l<data->CmdListsCount;l++) {
        const auto* list=data->CmdLists[l];
        for(const auto& cmd:list->CmdBuffer) {
            if(cmd.UserCallback) continue;
            for(unsigned int j=0;j+2<cmd.ElemCount;j+=3) {
                const auto &a=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+j]+cmd.VtxOffset];
                const auto &b=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+j+1]+cmd.VtxOffset];
                const auto &c=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+j+2]+cmd.VtxOffset];
                float area=Edge(a.pos,b.pos,c.pos.x,c.pos.y);
                if(std::abs(area)<.00001f) continue;
                int left=(std::max)(0,(int)std::floor((std::max)(cmd.ClipRect.x,(std::min)({a.pos.x,b.pos.x,c.pos.x}))));
                int right=(std::min)(w-1,(int)std::ceil((std::min)(cmd.ClipRect.z,(std::max)({a.pos.x,b.pos.x,c.pos.x}))));
                int top=(std::max)(0,(int)std::floor((std::max)(cmd.ClipRect.y,(std::min)({a.pos.y,b.pos.y,c.pos.y}))));
                int bottom=(std::min)(h-1,(int)std::ceil((std::min)(cmd.ClipRect.w,(std::max)({a.pos.y,b.pos.y,c.pos.y}))));
                for(int y=top;y<=bottom;y++) for(int x=left;x<=right;x++) {
                    float wa=Edge(b.pos,c.pos,x+.5f,y+.5f)/area;
                    float wb=Edge(c.pos,a.pos,x+.5f,y+.5f)/area;
                    float wc=1-wa-wb;
                    if(wa<0 || wb<0 || wc<0) continue;
                    float u=a.uv.x*wa+b.uv.x*wb+c.uv.x*wc;
                    float v=a.uv.y*wa+b.uv.y*wb+c.uv.y*wc;
                    int tx=std::clamp(int(u*tw),0,tw-1),ty=std::clamp(int(v*th),0,th-1);
                    const auto* tex=&atlas[(ty*tw+tx)*4];
                    float alpha=(((a.col>>24)&255)*wa+((b.col>>24)&255)*wb+((c.col>>24)&255)*wc)/255.f*tex[3]/255.f;
                    int pixel=(y*w+x)*3;
                    for(int k=0;k<3;k++) {
                        float col=(((a.col>>(k*8))&255)*wa+((b.col>>(k*8))&255)*wb+((c.col>>(k*8))&255)*wc)*tex[k]/255.f;
                        pixels[pixel+k]=(unsigned char)(col*alpha+pixels[pixel+k]*(1-alpha));
                    }
                }
            }
        }
    }
    FILE* file=std::fopen(path,"wb"); assert(file);
    std::fprintf(file,"P6\n%d %d\n255\n",w,h);
    std::fwrite(pixels.data(),1,pixels.size(),file); std::fclose(file);
}

}
