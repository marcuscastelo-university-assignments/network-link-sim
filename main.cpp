#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct Data
{
    unsigned int src : 4;
    unsigned int dst : 4;
};

using MAC_PARTS = unsigned char[6];

struct MAC
{
    MAC_PARTS parts;

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
};

class DataReceiver
{
    virtual void sendMessage(Data &data) = 0;
};

class PC : public DataReceiver
{

public:
    DataReceiver *port;
    virtual void sendMessage(Data &data) override
    {
        if (data.dst != this->MAC)
            return;
    }
};

class Switch : public DataReceiver
{
public:
    DataReceiver *ports[32];
    virtual void sendMessage(Data &data) override
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
