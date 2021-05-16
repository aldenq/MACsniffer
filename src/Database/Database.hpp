

namespace Database{



    typedef std::unordered_map<MACAdress, Device> DeviceMap; 
    typedef char* FilePtr;
    typedef size_t FilePtrdiff;
    typedef int fd_t;
    
    enum dberr_t {
        DBSUCCESS,
        DBFAILURE,
        DBOLDFILE
    };


    class DatabaseError : std::exception {
        public:
        std::string message;
        
        DatabaseError() = delete;
        DatabaseError(const char* reason) : message("DatabaseError:")
            { message+=reason; }
        DatabaseError(const std::string_view& reason) : message("DatabaseError:")
            { message+=reason; }

        const char* what() const noexcept {
            return message.c_str();
        }

    };

    class NoCachefileError : DatabaseError {
        public:
        NoCachefileError(const std::filesystem::path& p) 
            : DatabaseError("Could not open or create file: "+p.generic_string()) {}
    };

    class NoDeviceError : DatabaseError {
        public:
        NoDeviceError(const MACAdress& m) 
            : DatabaseError("Could not find device with id: "+(std::string)m) {}
    };

};

#include "FileHandle.hpp"
#include "InternalRuntime.hpp"
#include "Interface.hpp"


#include "FileHandle.cpp"
#include "Interface.cpp"
