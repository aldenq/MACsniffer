namespace Database{


    struct MappedChunk {
        FilePtr start = nullptr;
        FilePtr end = nullptr;

        FilePtrdiff size() const noexcept {
            return end-start;
        }
    
    };

    struct MappedCachefile {

        MappedChunk map;
        MappedChunk devices;
        MappedChunk locations;

        FileHeader* header;
        FileHeaderMap headerMap;

        fd_t fd;
        std::filesystem::path filepath;

    };
    
    MappedCachefile current_cachefile;

    async::EventLoop fileTasks(1, 0);
    
    

};