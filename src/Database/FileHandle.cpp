namespace Database{



    void FileHeaderMap::writeToMemory(char* dest) const {
        // Copy each field into memory:
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
            //Copy mac and position
            MACAdress mac;
            size_t position;
            mac = *(MACAdress*)(source);
            source += sizeof(MACAdress);
            position = *(size_t*)(source);
            source += sizeof(size_t);
            // Apply the mac and position the the unordered_map
            devicePositionMap[ mac ] = position;
        }
        // Copy remaining fields
        memcpy(&locationsPosition, source, sizeof(locationsPosition));
        source += sizeof(locationsPosition);
        memcpy(&devicesPosition, source, sizeof(devicesPosition));
        return getWrittenSize();
    }



    FilePtrdiff FileHeaderMap::getDevicesEnd(  ){
        // Find the largest file position in the map
        auto max = std::max_element( 
            devicePositionMap.begin(), 
            devicePositionMap.end(), 
            [](auto a, auto b){ 
                return a.second < b.second;
            } 
        );
        // if the position exists:
        if (max != devicePositionMap.end()){
            // Return the position + the size of the existing device.
            // This yields the first valid address to begin a new device.
            size_t off = max->second;
            return off + sizeOfWrittenDevice(off);
        }
        // If the position doesn't exist, return -1 for external handle
        else {
            return -1;
        }



    }

    template<typename T>
    void loadField(T& dest, FilePtr& source)
        { dest = *(T*)(source); source += sizeof(T); }

    template<typename T>
    void depositField(FilePtr& dest, T& source)
        { *(T*)(dest) = source; dest += sizeof(T); }

    /**
     * @returns LocationCount
     */
    size_t FileDeviceNodeHeader::load(Device& d){

        FilePtr address = (FilePtr) this;
        // d.addr = *(size_t*)(address);
        // address += sizeof(size_t);
        // size_t locations = *(size_t*)(address);
        // address += sizeof(size_t);
        // d.stats.bytesTransfered = *(size_t*)(address);
        size_t addr;
        loadField(addr, address);
        d.addr = addr;
        size_t locations;
        loadField(locations, address);
        loadField(d.stats.bytesTransfered, address);


        return locations;

    }

    void FileDeviceNodeHeader::deposit(const Device& d){

        FilePtr address = (FilePtr) this;
        // *(size_t*)(address) = d.addr.raw();
        // address += sizeof(size_t);
        // *(size_t*)(address) = d.locations.size();
        // address += sizeof(size_t);
        // *(size_t*)(address) = d.stats.bytesTransfered;
        // address += sizeof(size_t);
        size_t rawaddr = d.addr.raw();
        size_t locations = d.locations.size();
        size_t bytesTransfered = d.stats.bytesTransfered;
        depositField(address, rawaddr);
        depositField(address, locations);
        depositField(address, bytesTransfered);

    
    }

    size_t FileDeviceNodeHeader::staticSize() noexcept{
        return sizeof(size_t) + sizeof(size_t) + ScalarDeviceStats::writtenSize();
    }
    size_t FileDeviceNodeHeader::writtenSize(){
        return staticSize() + (*((size_t*)(this)+1) * sizeof(size_t));
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
        current_cachefile.filepath = p;
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
        // Check if the file exists already:
        if ( !std::filesystem::exists(p) ) {
            // if not, create it
            std::clog << "Creating new file: " << p << "\n";
            dberr_t err = createNewCachefile(p);
            if (err == DBFAILURE){
                throw NoCachefileError(p);
            }
        }
        // Weather it was just created or it already existed,
        // attempt to load it
        dberr_t err = loadCacheFile(p);
        if (err == DBFAILURE){
          throw NoCachefileError(p);
        }


    }

    void defragmentDevices(){

        // Copy the map into a vector for sorting
        std::vector<std::pair<MACAdress, size_t> > sortedMap;
        for (auto& pair : current_cachefile.headerMap.devicePositionMap) {
            sortedMap.push_back(pair);
        }

        // Sort the map by position in decending order
        std::sort( sortedMap.begin(), sortedMap.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        } );

        // Check the distance between each device to see if there is space between them
        std::pair<MACAdress, size_t> prevPair = sortedMap.at(0);
        for (size_t i = 1; i < sortedMap.size(); i++){
            std::pair<MACAdress, size_t> pair = sortedMap.at(i);
            // If there is a space between two adjacent devices
            if ( pair.second - prevPair.second > sizeOfWrittenDevice(prevPair.second) ) {
                // Slide the lower one up to the higher one
                FilePtrdiff newSpot = prevPair.second + sizeOfWrittenDevice(prevPair.second);
                FilePtr address = current_cachefile.map.start + newSpot;
                // Move the memory
                memmove( address, current_cachefile.map.start + pair.second, sizeOfWrittenDevice(pair.second));
                // Update the map
                current_cachefile.headerMap.devicePositionMap[pair.first] = newSpot;
            }

            prevPair = pair;

        }
        // Check for a distance between the first device and the start of
        // of the mapped device block
        std::pair<MACAdress, size_t> firstPair = sortedMap.at(0);
        FilePtr firstAddress = current_cachefile.map.start + firstPair.second;
        // If the first device is not at the start of the mapped device block,
        // The entire field can be shifted up to free up space.
        if (firstAddress != current_cachefile.devices.start) {
            FilePtrdiff diff = firstAddress - current_cachefile.devices.start;
            // Shift the entire devices section up for more space
            memmove(current_cachefile.devices.start, firstAddress, current_cachefile.devices.size() - diff);
            // All of the mappings need to be updated
            for ( auto& p : current_cachefile.headerMap.devicePositionMap ) {
                p.second -= diff;
            }

        }


    }


    Location _base_loadLocation(size_t index){

        // Using the start of the Locations map as an array of locations, and 
        // taking the index:
        return ((Location*)current_cachefile.locations.start) [ index ];

    }


    Device _base_loadDevice(MACAdress mac){

        // Get the file position from the map
        FilePtrdiff pos = current_cachefile.headerMap.devicePositionMap[mac];

        // Get the address based on the position
        FilePtr fdevice = current_cachefile.map.start + pos;

        // Interperate the address as a FileDeviceNodeHeader to get the MAC and locationCount
        FileDeviceNodeHeader * fdnh = (FileDeviceNodeHeader*) fdevice;


        //size_t locations = fdnh->locationCount;
        //fdevice += sizeof(FileDeviceNodeHeader);

        Device out;
        size_t locations = fdnh->load(out);
        fdevice += FileDeviceNodeHeader::staticSize();
        out.addr = mac;

        // For each location: load
        for (;  locations;
                locations --,
                fdevice += sizeof(size_t)
            )
        {
            // *(size_t*)(fdevice) returns each index stored in the array
            out.locations.push_back( _base_loadLocation( *(size_t*)(fdevice) ) );
        }

        return out;
    }

    size_t sizeToWrite(const Device& d){
        //     \/ Address               \/ Locations                    \/ LocationC            \/ Scalars
        return sizeof(size_t) + (d.locations.size() * sizeof(size_t))+sizeof(size_t) + ScalarDeviceStats::writtenSize();
    }
    size_t sizeOfWrittenDevice( FileDeviceNodeHeader* fdnh ){
        return (fdnh->writtenSize());
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
        FilePtr nextOpenDeviceAddress;
        
        // If the device we are resizing is not already at the end:
        if ( current_cachefile.headerMap.getDevicesEnd() != deviceLocation ){
            // Find a new open address, and copy over existing data
            nextOpenDeviceAddress = findNextDeviceAddress( sizeofNew );

            memcpy( nextOpenDeviceAddress, fdnh, sizeOfDevice );
            memset( (char*)fdnh, 0, sizeOfDevice );

        }
        // If the device is already at the end
        else {
            // It can already be expanded with no modification
            nextOpenDeviceAddress = (FilePtr) fdnh;

        }

        //fileTasks.launch(defragmentDevices);
        defragmentDevices();
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
        fdnh->deposit(d);
        //fdnh->mac = d.addr;
        //fdnh->locationCount = d.locations.size();
        address += (FileDeviceNodeHeader::staticSize());
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

    void writeMapToHeader(){
        
        // The location to write the map at will always be the same:
        FilePtr address =  (FilePtr) current_cachefile.map.start + sizeof(FileHeader);
        // Determine the currently available amount of space
        FilePtrdiff space = (current_cachefile.devices.start - address);
        // Determine the size needed to write the map
        size_t sizeToWriteThis = current_cachefile.headerMap.getWrittenSize();
        // if there is enough space, simply write it
        if (space > sizeToWriteThis){
            current_cachefile.headerMap.writeToMemory( address );
        } else {
            // If there is not enough space, the next step
            // is to check if the devices can be shifted out of the way.
            FilePtrdiff endOfDevices = current_cachefile.headerMap.getDevicesEnd();
            // Determine the remaining space after all of the devices
            FilePtrdiff endspace = current_cachefile.devices.end - (current_cachefile.map.start+endOfDevices);
            // If the devices can be shifted, do so
            if (endspace > sizeToWriteThis){
                // move all the memory over
                memmove( 
                        current_cachefile.devices.start + sizeToWriteThis, 
                        current_cachefile.devices.start, 
                        current_cachefile.devices.size() - endspace
                    );
                // Update the devices position
                current_cachefile.headerMap.devicesPosition = sizeof(FileHeader) + sizeToWriteThis;
                // Update the map
                for (auto& p : current_cachefile.headerMap.devicePositionMap) {
                    p.second += sizeToWriteThis;
                }
                // Finally, write to the address now that there is enough room
                current_cachefile.headerMap.writeToMemory(address);

            }else {
                //resize:
                current_cachefile.headerMap.writeToMemory(address);
            }

        }


    }

    void reallocFile(){
        // Much like adding to a vector, reallocFile will double the capacity
        // of the current file.
        // If the user wishes to shrink the file back down for storage reasons,
        // there will be a compression function to do so
        
        // ftruncate64 will expand the file itself to double its current size
        int err = ftruncate64(current_cachefile.fd, current_cachefile.map.size()*2);
        if (err) {
            perror("Could not truncate for realloc");
            exit(1);
        }
        // Now that the file has been expanded, the memory map
        // needs to be remapped with the new size
        FilePtr maperr = (char*)mremap(
            current_cachefile.map.start, 
            current_cachefile.map.size(),
            current_cachefile.map.size()*2,
            MREMAP_MAYMOVE
        );
        if (maperr == MAP_FAILED){
            perror("Could not remap file after truncate");
            exit(1);
        }

        // Now that the file and memory map are expanded,
        // the internal headers/maps need to be updated
        // with the new information.
        size_t oldsize = current_cachefile.map.size();
        size_t newsize = oldsize*2;
        current_cachefile.map.start = maperr;
        current_cachefile.map.end = current_cachefile.map.start + newsize;
        
        MappedChunk newLocations;
        newLocations.end = current_cachefile.map.end;
        newLocations.start = current_cachefile.map.end - current_cachefile.locations.size();
        
        MappedChunk oldLocations;
        oldLocations.start = current_cachefile.map.end - current_cachefile.locations.size();
        oldLocations.end = current_cachefile.map.end;

        current_cachefile.headerMap.locationsPosition = newLocations.start - current_cachefile.map.start;
        

        memmove (
            newLocations.start,
            oldLocations.start,
            current_cachefile.locations.size()
        );

        // memset(
        //     oldLocations.start,
        //     0,
        //     current_cachefile.locations.size()
        // );


        current_cachefile.locations = newLocations;

        size_t devicesSize = current_cachefile.devices.size();
        current_cachefile.devices.start = 
            current_cachefile.map.start + 
            sizeof(FileHeader) + 
            current_cachefile.headerMap.getWrittenSize();

        current_cachefile.devices.end = current_cachefile.devices.start + devicesSize;

        current_cachefile.header = (FileHeader*) current_cachefile.map.start;

        writeMapToHeader();
        munmap(current_cachefile.map.start, current_cachefile.map.size());
        close(current_cachefile.fd);

        loadCacheFile(current_cachefile.filepath);

    }  



    void cleanExit(){

        fileTasks.join();

        writeMapToHeader();

        munmap(current_cachefile.map.start, current_cachefile.map.size());
        close(current_cachefile.fd);

    }



};