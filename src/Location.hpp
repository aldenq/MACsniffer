
class Location{
    public:
    double longitude = 0;
    double latitude = 0;

    Location() = default;
    Location(double lo, double la) 
        : longitude(lo), latitude(la) {}
    Location(const Location& l) = default;
    Location(Location&& l) = default;

    bool withinRangeOf(const Location& other, double range) const noexcept;
    bool operator==(const Location& other) const noexcept;



};

namespace std{
    template<>
    struct hash<Location>{
        std::size_t operator()(const Location& l) const;
    };
};