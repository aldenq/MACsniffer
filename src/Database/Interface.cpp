namespace Database{

    bool deviceExists(MACAdress mac) noexcept {

        if (current_cachefile.map.size()){

            return current_cachefile.headerMap.devicePositionMap.find(mac) 
                != current_cachefile.headerMap.devicePositionMap.end();

        }

        return false;

    }
    async::Promise<Device> getDevice(MACAdress mac){

        return fileTasks.execute<Device> ( [mac](){ return _base_loadDevice(mac); } );

    }





};