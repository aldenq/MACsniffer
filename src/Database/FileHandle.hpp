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
     *  LocationsMap {
     *      {index, position}
     *      {index, position}
     *      ...
     *  }
     *  DevicesMap {
     *      {MACAdress, position}
     *      {MACAdress, position}
     *  }
     * 
     *  Devices{
     *      mac,
     *      locationCount,
     *      Locations {
     *          index,
     *          index
     *          ...
     *      }
     *      Heap* extraLocations
     *  }
     * 
     *  Locations {
     *      {long, lat},
     *      {long, lat},
     *      {long, lat}
     *  }
     *  
     * 
     *  Heap {
     *      ...
     *  }
     * 
     *  ^ The heap is used to store extra stuff that doesn't fit in any of the other fields.
     * For example, if a given device has been seen in more than (x) number of places maybe the 
     * run-on locations start getting stored there. The heap is indexed through other fields in 
     * the file.
     * 
     */
    
    typedef size_t FileheapPtr;


    struct FileHeader{

        char ftid[16] = "MACSnifferCache";
        int version = MACSNIFFER_VERSIONNO;
        size_t devices = 0;
        size_t locations = 0;


    };

    struct FileHeaderMap{
        
        // Map a location index to the position in the file
        std::unordered_map<size_t, size_t> locationIndexPositionMap;
        size_t lastLocation = 0;
        // Map a mac adress to that device's position in the file
        std::unordered_map<MACAdress, size_t> devicePositionMap;
        size_t lastDevice = 0;

        [[nodiscard]]
        char* write() const noexcept;

    };

    struct FileDeviceNodeHeader{

        FileDeviceNodeHeader() = default;

        MACAdress mac;
        size_t locationCount;
        std::vector<size_t> locations;

    };




    void createCacheFile(const std::filesystem::path& p);
    void openCachefile(std::fstream& f);
    void loadHeaders(std::fstream& f);


    void seekToDevice(const MACAdress& m, std::fstream& f, const FileHeaderMap& map);
    void seekToLocation(size_t index, std::fstream& f, const FileHeaderMap& map);



    void _loadSingleDevice(const MACAdress& m, Device* dest);
    void _loadSingleLocation(size_t index, Location* dest);
    void load(const MACAdress& m, DeviceMap& dest);

    void deposit(const Device& d);
    void add(const Device& d);

};