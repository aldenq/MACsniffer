namespace Database{

    #define DEVICES_PER_CACHELINE 32


    // In the MACSnifferCache file, Locations are stored by index.
    // This is a way to map them to their indexes
    std::unordered_map<Location, size_t> locationIndexMap;

    // Cache stores devices loaded from the cachefile next to a device that was 
    // loaded explicitly. So, when you try to load one device, the neighboring
    // devices are also loaded so that they can be accessed more quickly. Once
    // the cache grows to be too large, some unkown devices will be written back
    // to the file from the cache.
    DeviceMap cache;
    // Reserved devices are taken out of the cache and placed here so that they will
    // not be written back to the file before the user is done editing or analyzing
    // their contents.
    DeviceMap reserved;

    
    std::filesystem::path folder;
    std::filesystem::path locationStorage;
    
    std::fstream locationStream;


    

};