

bool Location::withinRangeOf(const Location& other, double range) const noexcept {
    return sqrt((longitude-other.longitude)*(longitude-other.longitude)+(latitude-other.latitude)*(latitude-other.latitude)) < range;
}