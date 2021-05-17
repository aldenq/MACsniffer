#include "MACSniffer.hpp"




int main(){


    Database::setFile("dist/dump/maccache.MACSniffer");
    
    Device d;
    d.addr = 0x34f39a7ccee1;
    d.locations.push_back({300,400});
    d.locations.push_back({500,600});
    
    Database::writeDevice(d);

    Device dw2;
    dw2.addr = 12345;
    dw2.locations.push_back({32,32});

    Database::writeDevice(dw2);

    Device d2 = Database::getDevice( 0x34f39a7ccee1  )->await();
    std::cout << (std::string)d2.addr << std::endl;
    
    for (const auto& l : d2.locations) {
        std::cout << "{ " << l.longitude << " , " << l.latitude << " }\n";
    }

    Database::getDevice(12345)->block_then( [](auto& d){

        std::cout << (std::string)d.addr<<std::endl;
        
        for (const auto& l : d.locations) {
            std::cout << l << std::endl;
        }

        return d;
    } );

    Database::cleanExit();

    return 0;
}