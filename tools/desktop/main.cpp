// Native audition: actual shared Engine, PerformanceSurface and RGB565 strip renderer.
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "mhac_desktop_audio.hpp"
#include "mhac_performance.hpp"
#include "mhac_visual_audio.hpp"
#include <array>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
namespace {
void SDLCALL callback(void* userdata, Uint8* stream, int len) {
    if(len>0) static_cast<DesktopAudio*>(userdata)->render(stream,std::size_t(len));
}
struct Resources {
    SDL_Window* window=nullptr;
    SDL_Renderer* renderer=nullptr;
    SDL_Texture* texture=nullptr;
    SDL_AudioDeviceID device=0;
    ~Resources() {
        if(device) SDL_CloseAudioDevice(device);
        if(texture) SDL_DestroyTexture(texture);
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};
struct UI {
    PerformanceSurface surface;
    InkField ink;
    Trace trace;
    bool held=false, captured=false, running=true;
    int x=0,y=0;
    uint32_t lastPoll=0;
    void mute(uint32_t now) {
        surface.forceMute();
        surface.sample(false,0,0,now);
        held=false; captured=false;
        SDL_CaptureMouse(SDL_FALSE);
    }
    void event(const SDL_Event& e,uint32_t now) {
        if(e.type==SDL_QUIT) { mute(now); running=false; }
        else if(e.type==SDL_KEYDOWN && !e.key.repeat) {
            if(e.key.keysym.sym==SDLK_ESCAPE) mute(now);
            if(e.key.keysym.sym==SDLK_q) { mute(now); running=false; }
        } else if(e.type==SDL_WINDOWEVENT &&
                  (e.window.event==SDL_WINDOWEVENT_FOCUS_LOST ||
                   e.window.event==SDL_WINDOWEVENT_MINIMIZED ||
                   (e.window.event==SDL_WINDOWEVENT_LEAVE && held && !captured))) {
            mute(now);
        } else if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT && !held) {
            x=e.button.x; y=e.button.y;
            if(x<0 || x>=width || y<0 || y>=height) return;
            held=true;
            captured=SDL_CaptureMouse(SDL_TRUE)==0;
            surface.sample(true,x,y,now); lastPoll=now;
        } else if(e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
            held=false; captured=false;
            SDL_CaptureMouse(SDL_FALSE);
            surface.sample(false,0,0,now); lastPoll=now;
        } else if(e.type==SDL_MOUSEMOTION && held) {
            x=e.motion.x; y=e.motion.y;
        }
    }
    Control tick(DesktopAudio& audio,uint32_t now) {
        surface.expire(now);
        surface.observeWatchdog(audio.exchange.watchdog_trips.load(std::memory_order_relaxed));
        if(uint32_t(now-lastPoll)>=8) {
            surface.sample(held,x,y,now); lastPoll=now;
        }
        const auto c=surface.effective(now);
        publishControl(audio.exchange,c);
        if(!c.gate || surface.mode()!=5) ink.disconnect();
        return c;
    }
    Frame frame(DesktopAudio& audio,Control c,uint32_t now) {
        mhac::AudioFeatures latest{};
        if(audio.exchange.features.read(latest)) trace.observe(latest,now);
        Frame f{c,surface.mode(),audio.exchange.snapshot(),now/25,surface.watchdogLatched(),
                &ink,surface.scale(),&trace,trace.fresh(now),surface.loopState(),surface.factoryIndex(),surface.factoryAudition()};
        if(f.mode==5) ink.advance(now,c.x,c.y,c.gate&&!c.mute,f.analysis&4095u);
        return f;
    }
};
bool ppm(const char* path,const std::array<uint16_t,width*height>& pixels) {
    std::ofstream out(path,std::ios::binary);
    out<<"P6\n"<<width<<" "<<height<<"\n255\n";
    for(auto p:pixels) {
        const unsigned char rgb[3]={static_cast<unsigned char>(((p>>11)&31u)*255/31),
            static_cast<unsigned char>(((p>>5)&63u)*255/63),
            static_cast<unsigned char>((p&31u)*255/31)};
        out.write(reinterpret_cast<const char*>(rgb),3);
    }
    return bool(out);
}
struct SmokeEvent { uint32_t ms; Uint32 kind; int x,y; };
constexpr SmokeEvent script[]={
    {30,SDL_MOUSEBUTTONDOWN,310,10},{60,SDL_MOUSEBUTTONUP,310,10},
    {90,SDL_MOUSEBUTTONDOWN,160,10},{120,SDL_MOUSEBUTTONUP,160,10},
    {150,SDL_MOUSEBUTTONDOWN,70,160},{230,SDL_MOUSEMOTION,160,100},
    {310,SDL_MOUSEMOTION,250,45},{390,SDL_MOUSEBUTTONUP,250,45},
    {900,SDL_WINDOWEVENT,0,0}
};
void inject(UI& ui,const SmokeEvent& s,uint32_t now) {
    SDL_Event e{}; e.type=s.kind;
    if(s.kind==SDL_MOUSEMOTION) { e.motion.x=s.x; e.motion.y=s.y; }
    else if(s.kind==SDL_WINDOWEVENT) e.window.event=SDL_WINDOWEVENT_FOCUS_LOST;
    else { e.button.button=SDL_BUTTON_LEFT; e.button.x=s.x; e.button.y=s.y; }
    ui.event(e,now);
}
int error(const char* where) {
    std::fprintf(stderr,"%s: %s\n",where,SDL_GetError()); return 1;
}
}
int main(int argc,char** argv) {
    const bool smoke=argc==3 && std::string(argv[1])=="--smoke";
    if(argc!=1 && !smoke) { std::fprintf(stderr,"Usage: mhac-desktop [--smoke capture.ppm]\n"); return 2; }
    SDL_SetMainReady();
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)!=0) return error("SDL_Init");
    DesktopAudio audio; UI ui; Resources res;
    res.window=SDL_CreateWindow("CYD-MHAC | Esc: mute | Q: quit",SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,width*2,height*2,smoke?SDL_WINDOW_HIDDEN:SDL_WINDOW_SHOWN);
    if(!res.window) return error("window");
    res.renderer=SDL_CreateRenderer(res.window,-1,SDL_RENDERER_SOFTWARE);
    if(!res.renderer) return error("renderer");
    if(SDL_RenderSetLogicalSize(res.renderer,width,height)!=0) return error("logical size");
    res.texture=SDL_CreateTexture(res.renderer,SDL_PIXELFORMAT_RGB565,SDL_TEXTUREACCESS_STREAMING,width,height);
    if(!res.texture) return error("RGB565 texture");
    SDL_AudioSpec want{},got{};
    want.freq=mhac::rate; want.format=AUDIO_S16SYS; want.channels=1; want.samples=256;
    want.callback=callback; want.userdata=&audio;
    res.device=SDL_OpenAudioDevice(nullptr,0,&want,&got,0);
    if(!res.device) return error("audio open");
    if(got.freq!=int(mhac::rate) || got.format!=AUDIO_S16SYS || got.channels!=1) {
        std::fprintf(stderr,"Unexpected application audio format; refusing playback\n"); return 1;
    }
    std::array<uint16_t,width*height> pixels{};
    SDL_PauseAudioDevice(res.device,0);
    const uint32_t start=SDL_GetTicks();
    uint32_t lastFrame=start-33;
    unsigned next=0,frames=0,lastFactory=factory_count; bool loopSeen=false,saved=false;
    while(ui.running) {
        const uint32_t now=SDL_GetTicks(), elapsed=now-start;
        SDL_Event e{};
        while(SDL_PollEvent(&e)) {
            // Dummy-driver window events are not part of the explicit smoke script.
            if(!smoke || e.type==SDL_QUIT) ui.event(e,now);
        }
        if(smoke) {
            while(next<sizeof(script)/sizeof(script[0]) && elapsed>=script[next].ms) inject(ui,script[next++],now);
        }
        const auto c=ui.tick(audio,now);
        if(ui.surface.factoryIndex()!=lastFactory) {
            lastFactory=ui.surface.factoryIndex();
            const std::string title=std::string("CYD-MHAC | ")+mhac::factory_presets[lastFactory].name+" | Esc: mute | Q: quit";
            SDL_SetWindowTitle(res.window,title.c_str());
        }
        if(uint32_t(now-lastFrame)>=33) {
            lastFrame=now;
            const auto f=ui.frame(audio,c,now);
            for(int row=0;row<height;row+=strip) renderStrip(f,row,1,pixels.data()+row*width);
            if(SDL_UpdateTexture(res.texture,nullptr,pixels.data(),width*int(sizeof(uint16_t)))!=0 ||
               SDL_RenderClear(res.renderer)!=0 || SDL_RenderCopy(res.renderer,res.texture,nullptr,nullptr)!=0)
                return error("present");
            SDL_RenderPresent(res.renderer); ++frames;
            if(f.loop_state==GestureLoop::State::Playing) {
                loopSeen=true;
                if(smoke && c.gate && !saved) {
                    saved=ppm(argv[2],pixels); if(!saved) return 1;
                }
            }
        }
        if(smoke && elapsed>=1500) {
            const auto calls=audio.callbacks.load(), active=audio.active_callbacks.load();
            const bool pass=loopSeen && saved && calls>20 && active>5 && frames>10 &&
                ui.surface.loopState()==GestureLoop::State::Idle && c.mute && !c.gate &&
                audio.last_peak.load()==0 && audio.exchange.watchdog_trips.load()==0;
            std::printf("desktop smoke %s: callbacks=%u active=%u frames=%u loop=%d capture=%d peak=%u\n",
                        pass?"PASS":"FAIL",unsigned(calls),unsigned(active),frames,int(loopSeen),int(saved),
                        unsigned(audio.last_peak.load()));
            return pass?0:1;
        }
        SDL_Delay(1);
    }
    ui.mute(SDL_GetTicks()); publishControl(audio.exchange,ui.surface.control());
    return 0;
}
