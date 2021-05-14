
unsigned long long int 
MACAdress::getManufacturerCode() const noexcept {
    // This method looks unintuitive, but it is because the MAC address only occupies 6 of m_base's 8 bytes.
    return ((m_base << 16) >> 40);
}

const char* MACAdress::getManufacturerName() const noexcept {
    const char* maybeName = ::getManufacturerName(getManufacturerCode());
    if (maybeName) return maybeName;
    return "Unkown Manufacturer";
}

bool MACAdress::operator == (const MACAdress& other) const noexcept 
    { return m_base == other.m_base; }

bool MACAdress::operator != (const MACAdress& other) const noexcept
    { return m_base != other.m_base; }
unsigned long long int MACAdress::raw() const noexcept 
    { return m_base; }




MACAdress::operator std::string() const noexcept{
    std::string out;
    out.reserve(18);
    out.resize(12);
    sprintf(out.data(), "%llx", m_base );
    out.insert(out.begin()+2, ':');
    out.insert(out.begin()+5, ':');
    out.insert(out.begin()+8, ':');
    out.insert(out.begin()+11, ':');
    out.insert(out.begin()+14, ':');

    return out;
}


namespace std{
    std::size_t hash<MACAdress>::operator()(const MACAdress& m) const {
        return m.raw();
    }
};