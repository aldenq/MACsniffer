#include <vector>
#include <optional>
#include <concepts>
#include <limits>
#include <memory>
#include <math.h>
#include <string>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <arpa/inet.h>
#include <list>
#include <map>
#include <set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <exception>
#include <assert.h>
#include <hashtable.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MACSNIFFER_MACROVERSION 0x01
#define MACSNIFFER_VERSION 0x0100
#define MACSNIFFER_EDITION 0x010000
#define MACSNIFFER_VERSIONNO (MACSNIFFER_MACROVERSION|MACSNIFFER_VERSION|MACSNIFFER_EDITION)


#include "KnownManufacturers.hpp"
#include "MACAdress.hpp"
#include "Location.hpp"
#include "Device.hpp"
#include "Database/Database.hpp"


#include "Location.cpp"
#include "MACAdress.cpp"