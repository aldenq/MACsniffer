
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
