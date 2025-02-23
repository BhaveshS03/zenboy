#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "../headers/cart.hpp"


// Reads ROM data from a file and stores it in romData
int Cart::read_rom(const std::string& path) {
    std::ifstream romFile(path, std::ios::binary | std::ios::ate);

    if (!romFile.is_open()) {
        std::cerr << "Error: Unable to open ROM file " << path << std::endl;
        return -1; // Error code
    }

    // Get file size and resize romData accordingly
    std::streamsize fileSize = romFile.tellg();
    romData.resize(static_cast<size_t>(fileSize));

    // Read ROM data into the vector
    romFile.seekg(0, std::ios::beg);
    if (!romFile.read(reinterpret_cast<char*>(romData.data()), fileSize)) {
        std::cerr << "Error: Failed to read ROM file " << path << std::endl;
        return -1; // Error code
    }

    romFile.close();
    return 0; // Success
}
std::vector<u8> Cart::get_rom_data(){
    return romData;
}