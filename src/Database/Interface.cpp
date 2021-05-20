namespace Database{

    bool deviceExists(MACAdress mac) noexcept {

        if (current_cachefile.map.size()){

            return current_cachefile.headerMap.devicePositionMap.find(mac) 
                != current_cachefile.headerMap.devicePositionMap.end();

        }

        return false;

    }

    [[nodiscard]]
    async::Promise<Device> getDevice(MACAdress mac){

        return fileTasks.execute<Device> ( [mac](){ return _base_loadDevice(mac); } );

    }

    void writeDevice(Device d){

        fileTasks.launch ( [d](){

            _base_writeDevice(d);

        } );

    }


    [[nodiscard]] 
    async::Promise<bool> writeDeviceSync(Device d){
        return fileTasks.execute ( [d]{

            _base_writeDevice(d);

        } );
    }



    [[nodiscard]]
    async::Promise<bool> flushBuffersToFile(){
        return fileTasks.execute ( writeMapToHeader );
    }

    [[nodiscard]]
    async::Promise<size_t> absoluteCompress(){
        return fileTasks.execute<size_t> ( [] { return 0ULL; } );
    }


};