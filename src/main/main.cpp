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
#include "tui.hpp"
#include <cstdlib>
#include <ctime>
#include "tests.hpp"

using namespace std::chrono_literals;
using namespace tui::text_literals;
using namespace tui::text;

using TT = tui::text::Text;

#include "types.hpp"
#include "crc_32.hpp"

int main(int argc, char const *argv[])
{
    while (true)
    {
        tui::clear();
        tui::printl("Welcome to our data-link layer simulation"_fblu);

        tui::printl("Select a story to run:"_fwhi.Bold());
        tui::printl("  1. (CRC): A-B (S1, S2)  with TTL expiring and promiscuous mode on C"_fgre);
        tui::printl("  2. (CRC): B-C (S1, S2)  with C sending a frame to itself and A in promiscuous mode"_fgre);
        tui::printl("  3. (CRC): B-C (S2 only) with bit flipping (tranmission interference)"_fgre);
        //TODO: add CRC, EVEN, ODD stories
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
            B_C_errorCRC();
            break;
            // case ''

        case 'q':
            return 0;
        }

        tui::printl("End of the story!"_fmag);
        tui::printl("Press enter to restart..."_fwhi.Bold());
        tui::readline();
    }

    return 0;
}
