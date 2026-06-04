from ns import ns
import cppyy

# Minimal C++ shim that calls back into Python, then wrap it as an EventImpl
ns.cppyy.cppdef(r"""
#include "CPyCppyy/API.h"
using namespace ns3;

void PySayHello() {
  CPyCppyy::Eval("say_hello()");
}

EventImpl* PySayHelloEvent() {
  return MakeEvent(&PySayHello);
}
""")

def say_hello():
    print("Hello from Python after 1 second")

def main():
    ev = ns.cppyy.gbl.PySayHelloEvent()           # MakeEvent(...) from C++ side
    ns.core.Simulator.Schedule(ns.core.Seconds(1.0), ev)
    ns.core.Simulator.Stop(ns.core.Seconds(2.0))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()

if __name__ == "__main__":
    main()
