


class MACAdress{
    private:
    unsigned long long int m_base = 0;
    public:


    
    MACAdress() = default;
    MACAdress(const MACAdress& other) = default;
    MACAdress(unsigned long long int number) noexcept : m_base(number) {}

    unsigned long long int 
    getManufacturerCode() const noexcept {
        // This method looks unintuitive, but it is because the MAC address only occupies 6 of m_base's 8 bytes.
        return ((m_base << 16) >> 40);
    }

    const char* getManufacturerName() const noexcept {
        const char* maybeName = ::getManufacturerName(getManufacturerCode());
        if (maybeName) return maybeName;
        return "Unkown Manufacturer";
    }

    bool operator == (const MACAdress& other) const noexcept 
        { return m_base == other.m_base; }

    bool operator != (const MACAdress& other) const noexcept
        { return m_base != other.m_base; }



};