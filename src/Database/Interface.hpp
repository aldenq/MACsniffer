namespace Database{

    void addNewDevice(const Device&& d);
    
    [[nodiscard]] Device 
    checkout(const MACAdress& m);

    void checkin(const Device& d);

    void setFile(const std::filesystem::path& f);

};