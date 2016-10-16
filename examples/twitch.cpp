#include "rtmp.h"
#include "log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

template <class T>
void endian_swap(T * objp) {
    unsigned char * memp = reinterpret_cast<unsigned char *>(objp);
    std::reverse(memp, memp + sizeof(T));
}

void write_u24be(std::stringstream & buf, unsigned int u32le) {
    endian_swap(&u32le);
    buf.write(1 + (char *)&u32le, 3);
}

int read_u24be(std::istream & buf) {
    int res = 0;
    buf.read(1 + (char *)&res, 3);
    endian_swap(&res);
    return res;
}

static bool write_vdo_packet(RTMP * rtmp, int timestamp, std::string content) {
    std::stringstream buf;
    buf >> std::noskipws;
    buf << '\x9';

    int len = content.length();
    write_u24be(buf, len);
    write_u24be(buf, timestamp);
    buf << '\0';

    buf << '\0'; buf << '\0'; buf << '\0';
    buf << content;

    len += 11;

    std::string packet = buf.str();
    return RTMP_Write(rtmp, packet.c_str(), packet.length());
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <twitch-stream-key> <flv>" << std::endl;
    }

    std::stringstream urlstr;
    urlstr << "rtmp://live-lhr.twitch.tv/app/" << argv[1];
    //urlstr << "rtmp://localhost/app/meh";
    std::string url = urlstr.str();
    std::cerr << "URL: " << url << std::endl;

    std::ifstream flv(argv[2]);
    if (!flv) {
        std::cerr << "Error opening " << argv[2] << std::endl;
        return 1;
    }

    auto rtmp = std::unique_ptr<RTMP, void(*)(RTMP *)>(RTMP_Alloc(), RTMP_Free);
    RTMP_Init(rtmp.get());
    //RTMP_LogSetLevel(RTMP_LOGALL);
    RTMP_LogSetLevel(RTMP_LOGDEBUG);
    RTMP_LogSetOutput(stderr);
    RTMP_SetupURL(rtmp.get(), &url[0]); // TODO: why RTMP_SetupURL doesn't receive "const char *"?
    RTMP_EnableWrite(rtmp.get());
    if (!RTMP_Connect(rtmp.get(), nullptr)) {
        std::cerr << "Error while connecting" << std::endl;
        return 1;
    }
    if (!RTMP_ConnectStream(rtmp.get(), 0)) {
        std::cerr << "Error while connecting stream" << std::endl;
        return 1;
    }

    std::cerr << std::hex;
    flv >> std::noskipws;

    while (RTMP_IsConnected(rtmp.get())) {
        flv.clear();
        flv.seekg(0x9, std::ios::beg);
        while (!flv.eof() && RTMP_IsConnected(rtmp.get())) {
            flv.seekg(4, std::ios::cur);
    
            int type = flv.get();
            if (type == -1) break;

            int size = read_u24be(flv);
            //std::cerr << "Frame with type " << type << " and size " << size << std::endl;

            flv.seekg(-4, std::ios::cur);
    
            std::string packet;
            std::copy_n(std::istream_iterator<char>(flv), size + 11, std::back_inserter(packet));
            if (!RTMP_Write(rtmp.get(), packet.c_str(), packet.length())) {
                std::cerr << "Error while writing" << std::endl;
                return 1;
            }
        }
    }

    /*
    int timestamp = 0;
    while (RTMP_IsConnected(rtmp.get())) {
        using namespace std::chrono_literals;

        std::cerr << "Sending frame at " << timestamp << std::endl;
        
        if (!write_vdo_packet(rtmp.get(), timestamp, frame.str())) {
            std::cerr << "Error while writing" << std::endl;
            return 1;
        }
        std::this_thread::sleep_for(5000ms);
        timestamp += 5000;
    }
    */

    std::cerr << "Disconnected" << std::endl;

    return 0;
}

