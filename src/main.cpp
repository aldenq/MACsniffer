#include "MACSniffer.hpp"




int main(){


    Database::setFile("dist/dump/maccache.MACSniffer");
    
    // for ( size_t i = 0; i < Database::current_cachefile.header->locations; i++ ){
    //     std::cout << ((Location*)Database::current_cachefile.locations.start )[ i ] << std::endl;
    // }

    // return 0;



    Device d;
    d.addr = 0x34f39a7ccee1;
    d.locations.push_back({300,400});
    d.locations.push_back({500,600});
    d.locations.push_back({27,27});

    //Database::writeDevice(d);
    Database::_base_writeDevice(d);


    //Device d2 = Database::getDevice( 0x34f39a7ccee1  )->await();
    Device d2 = Database::_base_loadDevice( 0x34f39a7ccee1 );
    std::cout << (std::string)d2.addr << std::endl;
    
    for (const auto& l : d2.locations) {
        std::cout << "{ " << l.longitude << " , " << l.latitude << " }\n";
    }


    Device dw2;
    dw2.addr = 12345;
    dw2.locations.push_back({32,32});
    dw2.locations.push_back({35,35});
    dw2.locations.push_back({206,206});


    //Database::writeDevice(dw2);

    Database::_base_writeDevice(dw2);    

    Device d3 = Database::_base_loadDevice(12345);

    std::cout << (std::string)d3.addr<<std::endl;
    
    for (const auto& l : d3.locations) {
        std::cout << l << std::endl;
    }


    Database::cleanExit();

    return 0;
}