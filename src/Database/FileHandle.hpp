namespace Database{

    #define BASE_PAGESIZE 4096
    #define MIN_CACHESIZE (BASE_PAGESIZE*8)
    #define ANTIFRAG_PADDSIZE 64
    #define ANTIFRAG_PADDSIZE_HUGE 512
    #define FILEMODE S_IRWXU | S_IRGRP | S_IROTH

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

    };

    struct FileDeviceNodeHeader{

        FileDeviceNodeHeader() = default;

        MACAdress mac;
        size_t locationCount;
        std::vector<size_t> locations;

    };


    void setFile(const std::filesystem::path& p);

    dberr_t createNewCachefile(const std::filesystem::path& p);
    dberr_t loadCacheFile(const std::filesystem::path& p); 

    


    void cleanExit();
    

};