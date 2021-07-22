#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <string.h>

#include "types.hpp"
#include "crc_32.hpp"

struct Ether2Frame
{
    unsigned int dst : 6;
    unsigned int src : 6;
    unsigned int type : 2;
    char *data;
    unsigned int CRC;

    Ether2Frame(MAC &dst, MAC &src, const char *const data, unsigned int data_size)
        : dst(dst.to_bytes()), src(src.to_bytes())
    {
        this->data = (char*) malloc(sizeof(char) * data_size);
        memcpy(this->data, data, data_size);
        CRC = CRC32(data, data_size);
    }
};

struct MAC
{
    MAC_PARTS parts;

    unsigned int to_bytes()
    {
        unsigned int bytes = 0;
        for (int i = 0; i < 6; i++)
        {
            bytes |= parts[i];
            bytes <<= 8;
        }
    }

    std::string to_string()
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(2) << (int)parts[0] << ":"
           << std::setw(2) << (int)parts[1] << ":"
           << std::setw(2) << (int)parts[2] << ":"
           << std::setw(2) << (int)parts[3] << ":"
           << std::setw(2) << (int)parts[4] << ":"
           << std::setw(2) << (int)parts[5];
        return ss.str();
    }

    MAC(const std::string &str)
    {
        std::string m(str);
        std::replace(m.begin(), m.end(), ':', ' ');
        std::stringstream ss(m);
        ss >> std::hex >> parts[0] >> parts[1] >> parts[2] >> parts[3] >> parts[4] >> parts[5];
    }

    MAC(MAC_PARTS parts)
    {
        for (int i = 0; i < 6; i++)
            this->parts[i] = parts[i];
    }

    MAC(unsigned int bytes)
    {
        for (int i = 0; i < 6; i++)
        {
            parts[i] = bytes & 0xFF;
            bytes >>= 8;
        }
    }
};

class DataReceiver
{
    virtual void sendMessage(Data &data) = 0;
};

class PC : public DataReceiver
{
    MAC mac;
    DataReceiver *port;

public:
    virtual void sendMessage(Data &data) override
    {
        if (data.dst != this->mac)
            return;
    }
};

class Switch : public DataReceiver
{
private:
    std::unordered_map<MAC, unsigned int> m_CAMTable;
    // std::unordered_map<IP, MAC> m_ARPTable;
public:
    DataReceiver *ports[32];
    virtual void sendMessage(Data &data) override
    {
    }

    Switch()
    {
    }
};

int main(int argc, char const *argv[])
{
    PC
        *dalton = new PC(),
        *marucs = new PC();

    Switch *local = new Switch();

    local->ports[0] = marucs;
    local->ports[1] = dalton;

    marucs->ports[0] = local;
    dalton->ports[0] = local;

    Data d = marucs.createMessage(dalton.MAC, "Tu é foda!");
    marucs->ports[0]->sendMessage(d);

    return 0;
}
