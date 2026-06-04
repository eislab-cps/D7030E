// Minimal "Hello, Simulator!" example in C++
#include "ns3/core-module.h"
#include <iostream>

using namespace ns3;

int main(int argc, char *argv[])
{
    // Use nanosecond resolution for all Time values
    Time::SetResolution(Time::NS);

    // Schedule a print event at t=1.0s
    Simulator::Schedule(Seconds(1.0), []() {
        std::cout << "Hello Simulator" << std::endl;
    });

    // Run simulation
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}