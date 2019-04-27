#pragma once

#include "Network.h"
#include "Outcome.h"
#include "Meta.h"

#include <string>

class Endpoint
{
public:

    enum Type : uint8_t
    {
        kNone,
        kIPv4,
        kIPv6
    };

    Endpoint();
    Endpoint(const std::string& acEndpoint);
    Endpoint(const Endpoint& acRhs);
    Endpoint(uint32_t aNetIPv4, uint16_t aPort);
    Endpoint(const uint16_t* acpNetIPv6, uint16_t aPort);

    bool IsIPv6() const;
    bool IsIPv4() const;
    bool IsValid() const;

    void SetPort(uint16_t aPort);
    uint16_t GetPort() const;
    const uint8_t* GetIPv4() const;
    uint8_t* GetIPv4();
    const uint16_t* GetIPv6() const;
    uint16_t* GetIPv6();

    bool ToNetIPv4(uint32_t& aDestination) const;
    bool ToNetIPv6(in6_addr& aDestination) const;

    Endpoint& operator=(const Endpoint& acRhs);
    bool operator==(const Endpoint& acRhs) const;
    bool operator!=(const Endpoint& acRhs) const;

protected:

    void Parse(const std::string& acEndpoint);

private:

    union
    {
        uint8_t m_ipv4[4];
        uint16_t m_ipv6[8];
    };

    friend struct std::hash<Endpoint>;
    
    Type m_type;
    uint16_t m_port;
};

namespace std
{
    template<>
    struct hash<Endpoint>
    {
        typedef Endpoint argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const noexcept
        {
            result_type result = 0;
            hash_combine(result, (uint8_t)s.m_type);
            hash_combine(result, s.m_port);

            result_type tmp = 0;
            if (s.IsIPv6())
            {
                for (auto c : s.m_ipv6)
                {
                    tmp = tmp * 31 + std::hash<uint16_t>()(c);
                }
            }
            else if (s.IsIPv4())
            {
                for (auto c : s.m_ipv4)
                {
                    tmp = tmp * 31 + std::hash<uint8_t>()(c);
                }
            }
            hash_combine(result, tmp);

            return result;
        }
    };
}