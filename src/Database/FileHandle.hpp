namespace Database{

    #define DEVICE_STATIC_LOCATION_LIMIT 1

    /**
     * 
     * Definitions:
     *  -position:
     *      A position in this context refers to a file read position.
     * 
     * File Format:
     * 
     *  FileHeader {
     *      id, version, devices, locations
     *  }
     *  
     * locations.maccache:
     *  This will store a contiguous array of all locations, which may be indexed
     *  by mac files.
     * 
     *  <macadress>.macdevicecache:
     *  This will store a single device, and all of its analyzed properties.
     * 
     */
    
    typedef size_t FileheapPtr;
    typedef int    DBErr_t; // 0 = success, !0 = fail

    const DBErr_t DBSuccess = 0;
    const DBErr_t DBFailure = 1;


    struct FileHeader{

        char ftid[16] = "MACSnifferCache";
        int version = MACSNIFFER_VERSIONNO;
        size_t size = sizeof(ftid)+sizeof(version);


    };

    struct DeviceFileHeader{

    };

    struct LocationFileHeader{
        
    };


    void init();

    DBErr_t createCache(const std::filesystem::path& _folder);

    DBErr_t createCacheFile(const std::filesystem::path& p);
    DBErr_t openCachefile(std::fstream& f);
    DBErr_t loadHeader(std::fstream& f);

    DBErr_t loadLocationFile();
    DBErr_t addLocation();

    DBErr_t load(const MACAdress& m, DeviceMap& dest);
    DBErr_t deposit(const Device& d);




};