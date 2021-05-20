#include "MACSniffer.hpp"




int main(){


    Database::setFile("dist/dump/maccache.MACSniffer");
    
    Device test;
    test.addr = 123;
    test.stats.bytesTransfered = 2;
    test.locations.push_back({69,69});
    Database::_base_writeDevice(test);
    
    Database::reallocFile();
    Device dr = Database::getDevice(123)->await();
    std::cout << (std::string) dr.addr << std::endl;
    std::cout << "bt: " << dr.stats.bytesTransfered << '\n';
    std::cout << "l: " << dr.locations.at(0) << '\n';


    // for (int i = 0; i < 1000; i++){
    //     Device d;
    //     d.addr = i;
    //     d.stats.bytesTransfered = i;
    //     d.locations.push_back({(double)i,(double)i});
    //     Database::writeDeviceSync(d)->await();
    // }

    // for (int i = 0; i < 100; i++){

    //     Device dr = Database::getDevice(i)->await();
    //     std::cout << (std::string) dr.addr << std::endl;
    //     std::cout << "bt: " << dr.stats.bytesTransfered << '\n';
    //     std::cout << "l: " << dr.locations.at(0) << '\n';

    // }

    Database::cleanExit();

    return 0;
}