#include <unordered_map>
std::unordered_map<int, const char*> knownManufacturers = {
    #include "KnownManufacturers.txt"
};


const char* getManufacturerName(int code) noexcept {

    auto maybeName = knownManufacturers.find(code);
    if (maybeName == knownManufacturers.end()){
        return nullptr;
    }else{
        return maybeName->second;
    }

}