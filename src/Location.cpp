

bool Location::withinRangeOf(const Location& other, double range) const noexcept {
    if (*this==other) return true;
    return sqrt((longitude-other.longitude)*(longitude-other.longitude)+(latitude-other.latitude)*(latitude-other.latitude)) < range;
}

bool Location::operator==(const Location& other) const noexcept{
    return other.latitude==latitude && longitude==other.longitude;
}

Location& Location::operator=(const Location& other) noexcept
  { longitude=other.longitude; latitude=other.latitude; return *this; }

namespace std{
    std::size_t hash<Location>::operator()(const Location& l) const {
        // The two doubles for long/lat are converted to floats
        float flo, fla;
        flo = l.longitude;
        fla = l.latitude;
        // Those floats are packed into the 8 byte space of the final size_t
        size_t out = 0;
        memcpy(&out,&flo,sizeof(float));
        memcpy((char*)(&out), &fla, sizeof(float));
        // out: [ [ flo ] [ fla ] ]
        //      0        4        8 
        return out;
    }
};

std::ostream& operator << (std::ostream& o, const Location& l){
    return o << "{ " << l.longitude << " , " << l.latitude << " }";
}
