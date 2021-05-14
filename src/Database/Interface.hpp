namespace Database{

    void flushCache();
    void demoteFlushCache();
    void precache(std::vector<MACAdress> macs);

    void addNewDevice(const Device&& d);
    
    [[nodiscard]] Device 
    checkout(const MACAdress& m);

    void checkin(const Device& d);


};