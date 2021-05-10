


class MACAdress{
    private:
    unsigned long long int m_base = 0;
    public:


    
    MACAdress() = default;
    MACAdress(const MACAdress& other) = default;
    MACAdress(unsigned long long int number) noexcept : m_base(number) {}

    int getManufacturerCode() const noexcept {
        return m_base & 0b111111111111111111111111;
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