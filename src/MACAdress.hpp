


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
    unsigned long long int raw() const noexcept;
    unsigned getByte(size_t b) const noexcept;

    explicit operator std::string() const noexcept;

};

namespace std{
    template<>
    class hash<MACAdress>{
        public:
        std::size_t operator()(const MACAdress& m) const;
    };

};