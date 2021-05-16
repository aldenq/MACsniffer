namespace Database{

    bool deviceExists(MACAdress mac) noexcept ;

    async::Promise<Device> getDevice(MACAdress mac);

};