namespace Database{

    #define BASE_PAGESIZE 4096
    #define MIN_CACHESIZE (BASE_PAGESIZE*8)
    #define ANTIFRAG_PADDSIZE 64
    #define ANTIFRAG_PADDSIZE_HUGE 512
    #define FILEMODE S_IRWXU | S_IRGRP | S_IROTH

    #define LOCATION_RANGE_THRESHOLD 0.000900900901 // 100 meters 

    /**
     * 
     * Definitions:
     *  -position:
     *      A position in this context refers to a file read position.
     *  - antifrag paddsize / paddsize huge
     *      A certain amount of padding space is placed between different fields to allow them
     *      to grow without moving the whole file around
     *  - device mapping
     *      A pair of one MACAdress and one FilePtrdiff (size_t) that shows where
     *      each device is located in the file. e.g:  { aa:aa:aa:aa:aa:aa : 0x23c8ae  }
     * 
     * 
     * File Format:
     * 
     *  FileHeader {
     *      id, version, devices, locations
     *  }
     *  
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
     *  }
     * 
     *  Locations {
     *      {long, lat},
     *      {long, lat},
     *      {long, lat}
     *  }
     *  
     * 
     * 
     */
    



    struct FileHeader{

        char ftid[16] = "MACSnifferCache";
        int version = MACSNIFFER_VERSIONNO;
        size_t devices = 0;
        size_t locations = 0;


    };

    struct FileHeaderMap{
        
        size_t devices = 0;
        // Map a mac adress to that device's position in the file
        std::unordered_map<MACAdress, FilePtrdiff> devicePositionMap;
        FilePtrdiff locationsPosition = 0;
        FilePtrdiff devicesPosition = 0;

        void writeToMemory(char* dest) const;
        size_t getWrittenSize() const noexcept;
        size_t loadFromMemory(const char* source);
        FilePtrdiff getDevicesEnd();

    };

    struct FileDeviceNodeHeader{

        FileDeviceNodeHeader() = default;

        MACAdress mac;
        size_t locationCount;

    };

    /**
     * Determine the number of bytes allocated to a device already in the cache
     */
    size_t sizeOfWrittenDevice( FileDeviceNodeHeader* fdnh );
    /**
     * Determine the number of bytes allocated to a device already in the cache
     */
    size_t sizeOfWrittenDevice( FilePtrdiff off );
    /**
     * Determine the number of bytes required to write a given device to cache
     */
    size_t sizeToWrite(const Device& d);

    /**
     * Load/Create&load a new/existing cachefile
     */
    void setFile(const std::filesystem::path& p);

    /**
     * Create a new cache file with the correct headers at the provided path
     */
    dberr_t createNewCachefile(const std::filesystem::path& p);
    /**
     * Load an existing cache file into the system
     * (populate current_cachefile, map memory, etc...)
     */
    dberr_t loadCacheFile(const std::filesystem::path& p); 


    /**
     * Load a device out of the file
     */
    Device _base_loadDevice(MACAdress mac);
    /**
     * Load a location out of the file
     */
    Location _base_loadLocation(size_t index);

    /**
     * Write a new or existing device
     */
    void _base_writeDevice(const Device& d);
    /**
     * Given a location, either determine it's existing index and return it,
     * or allocated it as a new location and return its new index.
     */
    size_t _base_createLocationIndex(const Location& l);


    /**
     * Correctly free all resources,
     * and write final header map to the file.
     */
    void cleanExit();
    

};