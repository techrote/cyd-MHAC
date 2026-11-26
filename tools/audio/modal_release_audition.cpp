#include "mhac_audio.hpp"
#include <array>
#include <cstdio>
#include <cstring>
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    const bool mute=std::strcmp(argv[1],"mute")==0;
    if(!mute && std::strcmp(argv[1],"natural")!=0) return 2;
    mhac::Engine engine; mhac::Exchange io; std::array<int16_t,mhac::block> pcm{};
    for(unsigned k=0;k<750;++k) {
        bool gate=(k<8 || (k>=30 && k<38) || (k>=64 && k<72));
        unsigned x=k<30?0u:k<64?1365u:2730u;
        io.publish_modal(x,400,gate,mute && k==96);
        engine.render(io,pcm.data(),pcm.size());
        if(std::fwrite(pcm.data(),sizeof(int16_t),pcm.size(),stdout)!=pcm.size()) return 1;
    }
}
