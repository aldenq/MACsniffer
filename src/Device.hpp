struct ScalarDeviceStats {
    size_t bytesTransfered = 0;

    
    static size_t writtenSize() noexcept {
        return sizeof(size_t);
    }

};

class Device{
    public:
    MACAdress addr;
    ScalarDeviceStats stats;
    std::vector<Location> locations;
    


};