#include "MACSniffer.hpp"




int main(){


    Database::createCacheFile("dist/dump/maccache.MACSniffer");
    Database::setFile("dist/dump/maccache.MACSniffer");

    Database::add(Device(MACAdress(12345), {}));


    return 0;
}