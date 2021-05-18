namespace Database{
    char* passnone(char* x) { return x;}
    void FileHeaderMap::writeToMemory(char* dest) const {
        // Copy each field into memory:
        char* ogdest = dest; passnone(ogdest); 
        // copy devices
        memcpy(dest, &devices, sizeof(devices));
        dest+= sizeof(devices);
        
        // Copy each mapped pair
        for (auto& pair : devicePositionMap){
            // First & second are copied seperate to overcome ABI breaks / inconsistent implimentations
            memcpy(dest, &pair.first, sizeof(pair.first));
            dest += sizeof(pair.first);
            
            memcpy(dest, &pair.second, sizeof(pair.second));
            dest += sizeof(pair.second);


        }
        // copy locationsPosition
        *(FilePtrdiff*)dest = locationsPosition;
        //memcpy(dest, &locationsPosition, sizeof(locationsPosition));
        dest += sizeof(locationsPosition);
        *(FilePtrdiff*)dest = devicesPosition;

    }

    size_t FileHeaderMap::getWrittenSize() const noexcept {
        // Number of mapped devices * size of one mapping = number of bytes
        size_t mappedDevices = devicePositionMap.size();
        // Sum: Devices field, locations/devices Pos fields, number of mapped devices * the size of one device mapping
        //                                                                             ^ each is one MACAdress and one size_t
        //
        return sizeof(devices) + sizeof(locationsPosition) + sizeof(devicesPosition) + (mappedDevices * (  sizeof(MACAdress)+sizeof(size_t)  ));

    }

    size_t FileHeaderMap::loadFromMemory(const char* source) {
        // Copy in devices field
        memcpy(&devices, source, sizeof(devices));
        source += sizeof(devices);
        // The number of devices given determines the number of device maps that will follow:
        for (   size_t i = 0 ; 
                i < devices ; 
                i ++
            ) 
        {   
            MACAdress mac;
            size_t position;
            mac = *(MACAdress*)(source);
            source += sizeof(MACAdress);
            position = *(size_t*)(source);
            source += sizeof(size_t);
            devicePositionMap[ mac ] = position;
        }

        memcpy(&locationsPosition, source, sizeof(locationsPosition));
        source += sizeof(locationsPosition);
        memcpy(&devicesPosition, source, sizeof(devicesPosition));
        return getWrittenSize();
    }



    FilePtrdiff FileHeaderMap::getDevicesEnd(  ){

        auto max = std::max_element( 
            devicePositionMap.begin(), 
            devicePositionMap.end(), 
            [](auto a, auto b){ 
                return a.second < b.second;
            } 
        );

        if (max != devicePositionMap.end()){
            size_t off = max->second;
            return off + sizeOfWrittenDevice(off);
        }else {
            return -1;
        }



    }





    dberr_t createNewCachefile(const std::filesystem::path& p){

        // Open / create the new cache file
        FILE* f = fopen64(p.c_str(), "w+");
        if (!f) {
            std::cerr << "Could not open file: " << p; perror(": ");
            return DBFAILURE;
        }

        // Create a default header to put in the start of the file
        FileHeader basic_header;
        // Write the header
        fwrite( &basic_header, sizeof(FileHeader), 1, f );
        if (ferror(f)){
            perror("Could not write: ");
            return DBFAILURE;
        }

        // Create the default headermap to copy into the file 
        FileHeaderMap emptyMap;
        emptyMap.devices = 0;
        emptyMap.devicesPosition = sizeof(FileHeader) + emptyMap.getWrittenSize() + ANTIFRAG_PADDSIZE_HUGE;
        emptyMap.locationsPosition = MIN_CACHESIZE - (MIN_CACHESIZE/4);

        
        // copy the default header into the file:
        // {
        size_t headerMapSize =  emptyMap.getWrittenSize();
        char* buff = (char*) malloc(headerMapSize);
        emptyMap.writeToMemory(buff);
        fwrite(buff, headerMapSize, 1, f );
        if (ferror(f)){
            perror("Could not write: ");
            return DBFAILURE;
        }

        // }
        
        // Truncate the file to the minimum cache size ( will grow the file with zeros )
        int err = ftruncate64(fileno(f), MIN_CACHESIZE);
        if (err) { // error check
            std::cerr << "Could not truncate: " << strerror(errno) << std::endl;
            return DBFAILURE;
        }

        // cleanup
        fclose(f);
        free( buff );
        return DBSUCCESS;

    }

    

    dberr_t loadCacheFile(const std::filesystem::path& p){
        // Open the file for r/w
        fd_t f = open64(p.c_str(), O_RDWR, FILEMODE);
        //FILE* f = fopen64(p.c_str(), "a");
        if (!f) {
            std::cerr << "Could not open file: " << p; perror(": ");
            return DBFAILURE;
        }

        // Determine the file's size:


        // seek to end
        ssize_t len_file;
        if ( (len_file = lseek64(f, 0, SEEK_END)) == -1 ) {
            perror("Could not seek: ");
            return DBFAILURE;
        }
        // seek back to start
        if ( lseek64(f, 0, SEEK_SET) == -1 ) {
            perror("Could not seek: ");
            return DBFAILURE;
        }
        
        // Map the file to memory, at address fptr
        FilePtr fptr = (FilePtr) mmap64(NULL, len_file,PROT_READ|PROT_WRITE,MAP_SHARED,(f),0);
        if (fptr == MAP_FAILED){
            std::cerr << "Could not map memory"; perror(" : ");
            return DBFAILURE;
        }

        // Load data into the current_cachefile variable:

        current_cachefile.fd = f;
        current_cachefile.map.start = fptr;
        current_cachefile.map.end = fptr + len_file;
        current_cachefile.header = (FileHeader*) fptr;

        current_cachefile.headerMap.loadFromMemory( fptr + sizeof(FileHeader) );

        current_cachefile.devices.start = fptr + current_cachefile.headerMap.devicesPosition;
        current_cachefile.locations.start = fptr + current_cachefile.headerMap.locationsPosition;
        current_cachefile.devices.end = current_cachefile.locations.start;
        current_cachefile.locations.end = current_cachefile.map.end;

        return DBSUCCESS;

    }

    void setFile(const std::filesystem::path& p) {
        if ( !std::filesystem::exists(p) ) {
            std::clog << "Creating new file: " << p << "\n";
            dberr_t err = createNewCachefile(p);
            if (err == DBFAILURE){
                throw NoCachefileError(p);
            }
        }
        dberr_t err = loadCacheFile(p);
        if (err == DBFAILURE){
          throw NoCachefileError(p);
        }


    }

    Location _base_loadLocation(size_t index){


        return ((Location*)current_cachefile.locations.start) [ index ];

    }


    Device _base_loadDevice(MACAdress mac){

        FilePtrdiff pos = current_cachefile.headerMap.devicePositionMap[mac];

        FilePtr fdevice = current_cachefile.map.start + pos;
        FileDeviceNodeHeader * fdnh = (FileDeviceNodeHeader*) fdevice;


        size_t locations = fdnh->locationCount;
        fdevice += sizeof(FileDeviceNodeHeader);

        Device out;
        out.addr = mac;


        for (;  locations;
                locations --,
                fdevice += sizeof(size_t)
            )
        {
            out.locations.push_back( _base_loadLocation( *(size_t*)(fdevice) ) );
        }

        return out;
    }
    size_t sizeToWrite(const Device& d){
        return sizeof(d.addr) + (d.locations.size() * sizeof(size_t))+sizeof(size_t);
    }
    size_t sizeOfWrittenDevice( FileDeviceNodeHeader* fdnh ){
        return sizeof(MACAdress) + (fdnh->locationCount * sizeof(size_t)) + sizeof(size_t);
    }
    size_t sizeOfWrittenDevice( FilePtrdiff off ){
        return sizeOfWrittenDevice( (FileDeviceNodeHeader*) (current_cachefile.map.start + off) );
    }


    FilePtr findNextDeviceAddress(size_t sizeToWriteThis){
        FilePtr address = nullptr;

        // Get the location of the end of the final device in the file
        FilePtrdiff newspot = current_cachefile.headerMap.getDevicesEnd();


        // Normal conditions:
        if (newspot != -1ULL){
            address = current_cachefile.map.start + newspot;
        // When there are no existing devices:
        }else{
            address = current_cachefile.devices.start;
        }

        size_t extraspace = current_cachefile.devices.end - (address + sizeToWriteThis);
        if (extraspace <= 0) {
            puts("Resize File");
        }
        
        return address;
    }


    FilePtr resizeDevice(FilePtrdiff deviceLocation, size_t sizeofNew){
        FileDeviceNodeHeader* fdnh = (FileDeviceNodeHeader*)( current_cachefile.map.start + deviceLocation );
        size_t sizeOfDevice = sizeOfWrittenDevice(fdnh);

        FilePtr nextOpenDeviceAddress = findNextDeviceAddress( sizeofNew );
        memcpy( nextOpenDeviceAddress, fdnh, sizeOfDevice );
        memset( (char*)fdnh, 0, sizeOfDevice );

        return nextOpenDeviceAddress;

    }


    void _base_writeDevice(const Device& d){
        FilePtr address = nullptr;
        size_t sizeToWriteThis = sizeToWrite(d);
        if (deviceExists(d.addr)){

            FilePtrdiff existingLocation = current_cachefile.headerMap.devicePositionMap[d.addr];

            if (sizeOfWrittenDevice(existingLocation) == sizeToWriteThis){
                address = current_cachefile.map.start + existingLocation;
            }else {
                puts("Resize");
                address = resizeDevice( existingLocation, sizeToWriteThis );
                current_cachefile.headerMap.devicePositionMap[d.addr] = address-current_cachefile.map.start;
                //resize
            }

        }else{
            current_cachefile.header->devices ++;
            current_cachefile.headerMap.devices ++;

            address = findNextDeviceAddress(sizeToWriteThis);

            current_cachefile.headerMap.devicePositionMap[d.addr] = address - current_cachefile.map.start;

        }

        FileDeviceNodeHeader* fdnh = (FileDeviceNodeHeader*) address;
        fdnh->mac = d.addr;
        fdnh->locationCount = d.locations.size();
        address += sizeof(FileDeviceNodeHeader);
        size_t *indexes = (size_t*)address;
        for ( size_t i = 0; i < d.locations.size(); i++ )
        {

            // *(size_t*)(address) = _base_createLocationIndex(d.locations.at(i));
            indexes[i] = _base_createLocationIndex(d.locations.at(i));

        }






    }
    size_t _base_createLocationIndex(const Location& l){

        Location* locationArray = (Location*) current_cachefile.locations.start;
        size_t i;
        for ( 
                i = 0;
                i < current_cachefile.header->locations;
                i ++
            )
        {
            if (locationArray[i].withinRangeOf(l, LOCATION_RANGE_THRESHOLD) ) {
                return i;

            }
        }
        // No existing locations were found, a new one needs to be allocated
        if (&locationArray[i] < (Location*) current_cachefile.locations.end){

            locationArray[i] = l;
            current_cachefile.header->locations++;
            return i;

        }else{
            puts("resize");
            // resize
        }
        
        
        return i;

    }



    void cleanExit(){

        fileTasks.join();

        current_cachefile.headerMap.writeToMemory( (FilePtr) current_cachefile.map.start + sizeof(FileHeader) );


        munmap(current_cachefile.map.start, current_cachefile.map.size());
        close(current_cachefile.fd);

    }



};