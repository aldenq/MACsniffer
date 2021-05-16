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
                i ++, 
                source += sizeof(MACAdress) + sizeof(size_t) 
            ) 
        {   
            // Copy in the mapping.
            MACAdress * mac = (MACAdress*) source;
            size_t    * position = (size_t*) source + sizeof(MACAdress);
            devicePositionMap[ *mac ] = *position;
        }

        memcpy(&locationsPosition, source, sizeof(locationsPosition));
        source += sizeof(locationsPosition);
        memcpy(&devicesPosition, source, sizeof(devicesPosition));
        return getWrittenSize();
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
        std::cout << "devices pos: " << emptyMap.devicesPosition << "\n";
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
        std::cout << len_file << "\n";
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


    void cleanExit(){
        
        munmap(current_cachefile.map.start, current_cachefile.map.size());
        close(current_cachefile.fd);

    }



};