


class MACAdress{
    private:
    unsigned long long int m_base = 0;
    public:


    
    MACAdress() = default;
    MACAdress(const MACAdress& other) = default;
    MACAdress(unsigned long long int number) noexcept : m_base(number) {}

    unsigned long long int getManufacturerCode() const noexcept;
    const char* getManufacturerName() const noexcept;
    bool operator == (const MACAdress& other) const noexcept;
    bool operator != (const MACAdress& other) const noexcept;


};