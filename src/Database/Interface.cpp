namespace Database{

    void flushCache(){
        for (const auto& p : cache){
            deposit(p.second);
        }
    }
    void demoteFlushCache(){
        flushCache();
        cache.clear();
    }
    void precache(std::vector<MACAdress> macs){

        for (auto mac : macs){
            load(mac, cache);
        }

    }


    void setFile(const std::filesystem::path& f){

        cachefilePath = f;
        openCachefile(cachefileStream);
        
        

    }

    void addNewDevice(const Device&& d){
        cache[d.addr] = d;
        flushCache();
    }


};