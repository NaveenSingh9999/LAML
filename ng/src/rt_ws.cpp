#include "rt_ws.h"
#include <vector>
#include <cstring>

// ---- compact SHA-1 (public domain style, RFC3174 logic) ----
namespace {
struct Sha1 {
    uint32_t h[5] = {0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
    uint64_t len = 0;
    uint8_t buf[64]{}; size_t buflen = 0;
    static uint32_t rol(uint32_t v,int n){ return (v<<n)|(v>>(32-n)); }
    void block(const uint8_t* p) {
        uint32_t w[80];
        for (int i=0;i<16;i++) w[i]=(uint32_t(p[i*4])<<24)|(uint32_t(p[i*4+1])<<16)|(uint32_t(p[i*4+2])<<8)|p[i*4+3];
        for (int i=16;i<80;i++) w[i]=rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4];
        for (int i=0;i<80;i++) {
            uint32_t f,k;
            if (i<20){ f=(b&c)|(~b&d); k=0x5A827999; }
            else if (i<40){ f=b^c^d; k=0x6ED9EBA1; }
            else if (i<60){ f=(b&c)|(b&d)|(c&d); k=0x8F1BBCDC; }
            else { f=b^c^d; k=0xCA62C1D6; }
            uint32_t t=rol(a,5)+f+e+k+w[i];
            e=d; d=c; c=rol(b,30); b=a; a=t;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e;
    }
public:
    void update(const uint8_t* p,size_t n){ len+=n; while(n){ size_t t=64-buflen; if(t>n)t=n; memcpy(buf+buflen,p,t); buflen+=t; p+=t; n-=t; if(buflen==64){block(buf);buflen=0;} } }
};
std::string sha1raw(const std::string& s){
    Sha1 c; c.update((const uint8_t*)s.data(),s.size());
    // manual finalize to avoid double-pad bug above: redo cleanly
    uint64_t bits=(uint64_t)s.size()*8;
    std::string m=s; m.push_back((char)0x80);
    while(m.size()%64!=56) m.push_back('\0');
    for(int i=7;i>=0;--i) m.push_back(char((bits>>(i*8))&0xFF));
    // process blocks
    Sha1 d;
    for(size_t i=0;i<m.size();i+=64) d.block((const uint8_t*)(m.data()+i));
    std::string o(20,'\0');
    for(int i=0;i<5;i++){ o[i*4]=char(d.h[i]>>24); o[i*4+1]=char(d.h[i]>>16); o[i*4+2]=char(d.h[i]>>8); o[i*4+3]=char(d.h[i]); }
    return o;
}
std::string b64(const std::string& in){
    static const char* A="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string o; o.reserve(((in.size()+2)/3)*4);
    for(size_t i=0;i<in.size();i+=3){
        unsigned v=(unsigned char)in[i]<<16;
        if(i+1<in.size()) v|=(unsigned char)in[i+1]<<8;
        if(i+2<in.size()) v|=(unsigned char)in[i+2];
        o+=A[(v>>18)&63]; o+=A[(v>>12)&63];
        o+=(i+1<in.size())?A[(v>>6)&63]:'=';
        o+=(i+2<in.size())?A[v&63]:'=';
    }
    return o;
}
}

std::string WsCodec::acceptKey(const std::string& clientKey) {
    static const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // trim whitespace
    size_t b=clientKey.find_first_not_of(" \t\r\n"), e=clientKey.find_last_not_of(" \t\r\n");
    std::string k = (b==std::string::npos)?"":clientKey.substr(b,e-b+1);
    return b64(sha1raw(k+magic));
}

std::string WsCodec::encode(const std::string& p, uint8_t opcode) {
    std::string o; o.push_back((char)(0x80|opcode));
    size_t n=p.size();
    if(n<126){ o.push_back((char)n); }
    else if(n<65536){ o.push_back((char)126); o.push_back(char(n>>8)); o.push_back(char(n&255)); }
    else { o.push_back((char)127); for(int i=7;i>=0;--i) o.push_back(char((n>>(i*8))&255)); }
    o+=p;
    return o;
}

std::string WsCodec::encodeClose(uint16_t code, const std::string& reason) {
    std::string p; p.push_back(char(code>>8)); p.push_back(char(code&255)); p+=reason;
    return encode(p, 0x8);
}

std::optional<WsFrame> WsCodec::decode(const char* d, size_t len, size_t& consumed, bool& errClose) {
    consumed=0; errClose=false;
    if(len<2) return std::nullopt;
    uint8_t b0=(uint8_t)d[0], b1=(uint8_t)d[1];
    bool fin=(b0&0x80)!=0;
    if(b0&0x70){ errClose=true; return WsFrame{0,"",true}; } // RSV must be 0 (no ext in 4.1)
    uint8_t op=b0&0x0F;
    if(op>0xA||(op>0x2&&op<0x8)){ errClose=true; return WsFrame{0,"",true}; }
    bool masked=(b1&0x80)!=0;
    if(!masked){ errClose=true; return WsFrame{0,"",true}; } // client MUST mask
    uint64_t n=b1&0x7F; size_t pos=2;
    if(n==126){ if(len<4) return std::nullopt; n=((uint8_t)d[2]<<8)|(uint8_t)d[3]; pos=4; }
    else if(n==127){ if(len<10) return std::nullopt; n=0; for(int i=0;i<8;i++) n=(n<<8)|(uint8_t)d[2+i]; pos=10; }
    if(n>kMaxFrame){ errClose=true; return WsFrame{0,"",true}; }
    if(op>=0x8 && (n>125||!fin)){ errClose=true; return WsFrame{0,"",true}; }
    if(len<pos+4+(size_t)n) return std::nullopt;
    const uint8_t* mask=(const uint8_t*)(d+pos); pos+=4;
    std::string p((size_t)n,'\0');
    for(uint64_t i=0;i<n;i++) p[(size_t)i]=d[pos+i]^mask[i%4];
    consumed=pos+(size_t)n;
    return WsFrame{op,std::move(p),fin};
}
