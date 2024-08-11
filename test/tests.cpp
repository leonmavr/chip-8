#include "chip8_tester.hpp"
#include "chip8.hpp"
#include <cstdint>
#include <memory>
#include <array>
#include <string>
#include <iostream>


constexpr static auto ibm_logo = []() {
    return std::array<uint8_t, 32*64> {
        []() constexpr{
            std::array<uint8_t, 32*64> ret {};
            auto mp = std::make_pair<int, int>;
            std::array<std::pair<int, int>, 38> ones = {
                mp(524, 531), mp(533, 541), mp(545, 549), mp(559, 563), mp(652, 659),
                mp(661, 671), mp(673, 678), mp(686, 691), mp(782, 785), mp(791, 793),
                mp(797, 799), mp(803, 807), mp(813, 817), mp(910, 913), mp(919, 925),
                mp(931, 937), mp(939, 945), mp(1038, 1041), mp(1047, 1053), mp(1059, 1061),
                mp(1063, 1069), mp(1071, 1073), mp(1166, 1169), mp(1175, 1177),
                mp(1181, 1183), mp(1187, 1189), mp(1192, 1196), mp(1199, 1201),
                mp(1292, 1299), mp(1301, 1311), mp(1313, 1317), mp(1321, 1323),
                mp(1327, 1331), mp(1420, 1427), mp(1429, 1437), mp(1441, 1445),
                mp(1450, 1450), mp(1455, 1459)
            };
            for (const auto& o: ones) {
                for (auto oo = o.first; oo <= o.second; ++oo)
                    ret[oo] = 1;
            }
            return ret;
        }()
    };
}();


int main(int argc, char** argv) {
    auto ch8 = std::make_unique<Chip8>();
    auto tester = std::make_unique<Chip8Tester>();
    ch8->LoadRom("./roms/test/Ibm_logo.ch8");
    ch8->Run(1000);
    tester->AssertFrameBuffer(*ch8, ibm_logo); 
    std::cout << "\033[2J\033[H";
    std::cout << std::string(80, '=') + "\n" + "All tests passed!\n" + std::string(80, '=') << std::endl;;
    return 0;
}
