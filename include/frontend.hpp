#ifndef FRONTEND_HPP
#define FRONTEND_HPP

#include <iostream>
#include <string>
#include <array>
#include <cstdint>
#include <iomanip>


namespace Frontend { 
    std::string WriteRight(std::string& input, unsigned row, const std::string& appended) {
        int count = 0;
        size_t pos = 0;
        while ((pos = input.find('\n', pos)) != std::string::npos) {
            count++;
            if (count == row + 1) {
                // Replace the n-th newline with the appended string
                input.replace(pos, 1, "\t" + appended);
                break;
            }
            pos++;
        }
        return input;
    }

    std::string WriteAbove(std::string& input, const std::string& text, int width) {
        return std::string(width - text.length()/2, ' ') + text +
               std::string(width - text.length()/2, ' ') + "\n" + input;
    }

    std::string WriteRegs(std::string& input, std::array<uint8_t, 16> regs) {
        for (int i = 0; i < regs.size(); i += 4) {
            std::stringstream ss;
            ss << std::uppercase <<
                "V" << std::hex << i+0 << ": " << std::setfill('0') << std::setw(2) << (int) regs[i+0] << " " <<
                "V" << std::hex << i+1 << ": " << std::setfill('0') << std::setw(2) << (int) regs[i+1] << " " <<
                "V" << std::hex << i+2 << ": " << std::setfill('0') << std::setw(2) << (int) regs[i+2] << " " <<
                "V" << std::hex << i+3 << ": " << std::setfill('0') << std::setw(2) << (int) regs[i+3] << std::endl; 
            WriteRight(input, i/4, ss.str());
        }
        return input;
    }

    std::string WritePC(std::string& input, uint16_t PC) {
        std::stringstream ss;
        ss << std::hex << "PC: " << std::setw(3) << std::setfill('0') << std::uppercase << (int)PC << std::endl;
        WriteRight(input, 5, ss.str());
        return input;
    }

    std::string WriteStack(std::string& input, std::array<uint16_t, 12>& stack) {
        std::stringstream ss;
        WriteRight(input, 6, "Stack[0-3]:\n");
        ss << std::hex << std::setw(3) << std::setfill('0') << std::uppercase << (int) stack[0] << " ";
        ss << std::hex << std::setw(3) << std::setfill('0') << std::uppercase << (int) stack[1] << " ";
        ss << std::hex << std::setw(3) << std::setfill('0') << std::uppercase << (int) stack[2] << " ";
        ss << std::hex << std::setw(3) << std::setfill('0') << std::uppercase << (int) stack[3] << std::endl;
        WriteRight(input, 7, ss.str());
        return "";
    }

    std::string WriteSP(std::string& input, uint16_t SP) {
        std::stringstream ss;
        ss << std::hex << "SP: " << std::setw(2) << std::setfill('0') << std::uppercase << (int)SP << std::endl;
        WriteRight(input, 8, ss.str());
        return input;
    }


    std::string WriteI(std::string& input, uint16_t I) {
        std::stringstream ss;
        ss << std::hex << "I:  " << std::setw(3) << std::setfill('0') <<std::uppercase << (int)I << std::endl;
        WriteRight(input, 4, ss.str());
        return input;
    }
}; // namespace Frontend

#endif // FRONTEND_HPP
