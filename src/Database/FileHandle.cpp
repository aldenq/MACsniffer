namespace Database{

    DBErr_t createCache(const std::filesystem::path& _folder){

        std::filesystem::create_directory(_folder);
        folder=_folder;

        locationStorage = folder / "locations.maccache";
        createCacheFile(locationStorage);
        
        return DBSuccess;
    }


    DBErr_t createCacheFile(const std::filesystem::path& p){

        std::ofstream f;

        

        f.open(p);
        if ( !f.is_open() ) {
            return DBFailure;
        }

        FileHeader fh;
        f.write((char*)&fh,sizeof(fh));
        
        return DBSuccess;

    }

    DBErr_t openCachefile(const std::filesystem::path& p , std::fstream& f){
        
        f.open(p, std::ios::in | std::ios::out);
        if ( !f.is_open() ) {
            return DBFailure;
        }

        return DBSuccess;

    }

    DBErr_t load(const MACAdress& m, DeviceMap& dest){
        std::fstream f;
        openCachefile( folder / std::filesystem::path((std::string)(m)), f);
        loadHeader(f);

        DeviceFileHeader dfh;
        f.read((char*)&dfh, sizeof(dfh));
        dest[m] = Device(m);


        
        return DBSuccess;


    }
    DBErr_t deposit(const Device& d){


    }


    

};