#include "MACSniffer.hpp"




int main(){


    Database::setFile("dist/dump/maccache.MACSniffer");
    
    // Device d;
    // d.addr = 0x34f39a7ccee1;
    // d.locations.push_back({1,2});
    
    // Database::_base_writeDevice(d);

    Device d2 = Database::getDevice( 0x34f39a7ccee1  )->await();
    std::cout << (std::string)d2.addr << std::endl;

    Database::cleanExit();

    return 0;
}