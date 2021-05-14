namespace Database{

    void setFile(const std::filesystem::path& f){

        cachefilePath = f;
        openCachefile(cachefileStream);
        
        

    }

};