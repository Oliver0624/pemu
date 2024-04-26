//
// Created by Oliver on 04/25/2024
//

#ifndef PEMU_PGBAIO_H
#define PEMU_PGBAIO_H

namespace c2d {

    class PGBAIo : public c2d::C2DIo {

    public:
#ifdef __PSP2__

        std::string getDataPath() override {
            return "ux0:/data/pgba/";
        }

#elif __PS4__
        std::string getDataPath() override {
            return "/data/pgba/";
        }
#ifndef NDEBUG
        std::string getRomFsPath() override {
            return "/data/pgba/";
        }
#endif
#elif __3DS__
#ifndef NDEBUG
        std::string getDataPath() override {
            return "/3ds/pgba/";
        }
#endif
#elif __SWITCH__
#if __DEBUG_SVC__
        std::string getDataPath() override {
            return "/switch/pgba/";
        }
#endif
#endif
    };
}

#endif //PEMU_PGBAIO_H
