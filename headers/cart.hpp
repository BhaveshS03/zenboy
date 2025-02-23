#pragma once

#include <string>
#include <common.hpp>
#include <vector>

class Cart{
    private:
        std::vector<u8> romData;
    public:
        int read_rom(const std::string& path);
        std::vector<u8> get_rom_data();

};