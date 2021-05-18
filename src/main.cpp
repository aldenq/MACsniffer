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
    d.locations.push_back({37,37});
    d.locations.push_back({35,35});
    // d.locations.push_back({206,206});

    Database::writeDevice(d);
    //Database::_base_writeDevice(d);

    


    Device dw2;
    dw2.addr = 12345;
    dw2.locations.push_back({32,32});
    dw2.locations.push_back({32,32});
    // dw2.locations.push_back({206,206});
    // dw2.locations.push_back({69,69});


    Database::writeDevice(dw2);

    //Database::_base_writeDevice(dw2);    

    



    Device another;
    another.addr = 6789;
    another.locations.push_back({1024,1024});
    Database::writeDevice(another);

    Device anotherr = Database::getDevice(6789)->await();
    
    
    
    std::cout << "Location Stores Count: " << Database::current_cachefile.header->locations << '\n';
    std::cout << (std::string)anotherr.addr << std::endl;
    for (const auto& l : anotherr.locations){
        std::cout << l << std::endl;
    }

    Device d3 = Database::getDevice(12345)->await();

    std::cout << (std::string)d3.addr<<std::endl;
    
    for (const auto& l : d3.locations) {
        std::cout << l << std::endl;
    }

    Device d2 = Database::getDevice( 0x34f39a7ccee1  )->await();
    //Device d2 = Database::_base_loadDevice( 0x34f39a7ccee1 );
    std::cout << (std::string)d2.addr << std::endl;
    
    for (const auto& l : d2.locations) {
        std::cout << "{ " << l.longitude << " , " << l.latitude << " }\n";
    }


    Database::cleanExit();

    return 0;
}