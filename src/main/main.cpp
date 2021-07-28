/**
 * Arquivo main, contendo inicialização e chamada das funções e métodos implmentados
 */
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <memory>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

#include "tui.hpp"
#include "tests.hpp"

using namespace std::chrono_literals;
using namespace tui::text_literals;
using namespace tui::text;

using TT = tui::text::Text;

#include "types.hpp"
#include "crc_32.hpp"

void interactive(ERROR_CONTROL errorControl)
{
    srand(time(NULL));

    static const TT error_names[] = {"Even bits"_b, "Odd bits"_b, "CRC"_b};
    tui::clear();
    tui::printl("Interactive Session:"_fgre);
    tui::printl("All peers will be created using "_t + error_names[(int)errorControl]);

    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), errorControl);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), errorControl);

    Ref<Switch> S2 = std::make_shared<Switch>(errorControl, 3);

    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    std::string msg;
    do
    {
        tui::print("Type a message to be sent from B to C (or 'q' to quit): "_fwhi.Bold());
        msg = tui::readline();
        Ether2Frame frame(C->m_MAC, B->m_MAC, msg.c_str(), msg.size() + 1, errorControl);
        B->sendFrame(0, frame);

        tui::printl();
        tui::printl("Press enter to clear the screen...");
        tui::readline();
        tui::clear();
    } while (msg != "q");
}

int main(int argc, char const *argv[])
{
    while (true)
    {
        tui::clear();
        tui::printl("Welcome to our data-link layer simulation"_fblu);

        tui::printl("Select a story to run:"_fwhi.Bold());
        tui::printl("  1. (CRC):  A-B (S1, S2)  with TTL expiring and promiscuous mode on C"_fgre);
        tui::printl("  2. (CRC):  B-C (S1, S2)  with C sending a frame to itself and A in promiscuous mode"_fgre);
        tui::printl("  3. (CRC):  B-C (S2 only) with bit flipping (tranmission interference)"_fgre);
        tui::printl("  4. (EVEN): B-C (S2 only) with bit flipping (tranmission interference)"_fgre);
        tui::printl("  5. (ODD):  B-C (S2 only) with bit flipping (tranmission interference)"_fgre);
        tui::printl(""_fgre);
        tui::printl("  7. (CRC):  Interactive with 10% chance of bit flipping"_fgre);
        tui::printl("  8. (EVEN): Interactive with 10% chance of bit flipping"_fgre);
        tui::printl("  9. (ODD):  Interactive with 10% chance of bit flipping"_fgre);

        tui::printl("");
        tui::printl("  q. quit"_fred);
        auto opt = tui::readline();

        if (opt.size() < 1)
            continue;
        switch (opt[0])
        {
        case '1':
            srand(1);
            A_B_ttl_andPromC();
            break;
        case '2':
            srand(1);
            B_C_self_andPromA();
            break;
        case '3':
            srand(10);
            B_C_error(ERROR_CONTROL::CRC);
            break;
        case '4':
            srand(10);
            B_C_error(ERROR_CONTROL::EVEN);
            break;
        case '5':
            srand(10);
            B_C_error(ERROR_CONTROL::EVEN);
            break;
        case '7':
            interactive(ERROR_CONTROL::CRC);
            break;
        case '8':
            interactive(ERROR_CONTROL::EVEN);
            break;
        case '9':
            interactive(ERROR_CONTROL::ODD);
            break;

        case 'q':
            return 0;
        }

        tui::printl("End of the story!"_fmag);
        tui::printl("Press enter to restart..."_fwhi.Bold());
        tui::readline();
    }

    return 0;
}
