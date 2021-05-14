namespace Database{

    void createCacheFile(const std::filesystem::path& p){

        std::ofstream f;

        

        f.open(p);
        if ( !f.is_open() ) {
            throw NoCachefileError(cachefilePath);
        }

        FileHeader fh;
        f.write((char*)&fh, sizeof(fh));
        FileHeaderMap fhm;
        fhm.lastDevice = sizeof(FileHeader);
        fhm.lastLocation = sizeof(FileHeader)+sizeof(size_t);
        

    }

    void openCachefile(std::fstream& f){
        
        f.open(cachefilePath, std::ios::in | std::ios::out);
        if ( !f.is_open() ) {
            throw NoCachefileError(cachefilePath);
        }

    }

    void loadHeaders(std::fstream& f){

        f.seekg(0);
        f.read((char*)&cachefileHeader, sizeof(FileHeader));
        if (cachefileHeader.version != MACSNIFFER_VERSIONNO){
            // do something when versions don't match
        }

        for(size_t i = 0; i < cachefileHeader.locations;i++){
            std::pair<size_t, size_t> locationMapNode;
            f.read((char*)&locationMapNode,sizeof(locationMapNode));
            cachefileHeaderMap.locationIndexPositionMap[locationMapNode.first] = locationMapNode.second;
        }

        for (size_t i = 0; i < cachefileHeader.devices; i++){
            std::pair<MACAdress,size_t> macIndexnode;
            f.read((char*)&macIndexnode, sizeof(macIndexnode));
            cachefileHeaderMap.devicePositionMap[macIndexnode.first] = macIndexnode.second;
        }


    }


    void seekToDevice(const MACAdress& m, std::fstream& f, const FileHeaderMap& map)
    {
        assert(f.is_open());

        auto maybePos = map.devicePositionMap.find(m);
        if (maybePos != map.devicePositionMap.end()){
            size_t filepos = maybePos->second;
            f.seekg(filepos);
        }else{
            throw NoDeviceError(m);
        }
        

    }
    void seekToLocation(size_t index, std::fstream& f, const FileHeaderMap& map){

        assert(f.is_open());
        
        auto maybePos = map.locationIndexPositionMap.find( index );
        if (maybePos != map.locationIndexPositionMap.end()){
            size_t filepos = maybePos->second;
            f.seekg(filepos);
        } else {
            throw DatabaseError("Could not find location at index: "+std::to_string(index));
        }


    }



    void _loadSingleDevice(const MACAdress& m, Device* dest){
        size_t cpos = cachefileStream.tellg();
        seekToDevice(m, cachefileStream, cachefileHeaderMap);
        FileDeviceNodeHeader fdnh{};
        cachefileStream.read((char*)&fdnh.mac, sizeof(fdnh.mac));
        cachefileStream.read((char*)&fdnh.locationCount, sizeof(fdnh.locationCount));
        
        while(fdnh.locationCount){
            size_t reader = 0;
            cachefileStream.read((char*)&reader, sizeof(size_t));
            fdnh.locationCount--;
            fdnh.locations.push_back(reader);
        }


        cachefileStream.seekg(cpos);

        dest->addr = fdnh.mac;
        for(size_t i = 0; i < fdnh.locations.size(); i++){
            Location locloader;
            _loadSingleLocation( fdnh.locations[i], &locloader );
            dest->locations.push_back(locloader);
        }

    }
    void _loadSingleLocation(size_t index, Location* dest){

        size_t cpos = cachefileStream.tellg();
        seekToLocation(index, cachefileStream, cachefileHeaderMap);
        cachefileStream.read((char*)dest, sizeof(Location));
        cachefileStream.seekg(cpos);

    }
    void load(const MACAdress& m, DeviceMap& dest){
        
        Device holder;
        _loadSingleDevice(m, &holder );
        dest[m] = holder;

    }

    void deposit(const Device& d){

        try{
            seekToDevice(d.addr, cachefileStream, cachefileHeaderMap.devicePositionMap);
        }catch (const NoDeviceError& e) {

            cachefileHeaderMap.lastDevice

        }

    }
    void add(const Device& d);


};