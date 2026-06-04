from ns import ns


def say_hello():
    print("Hello from Python after 1 second")


def main():
    ns.Simulator.Schedule(ns.Seconds(1.0), ns.MakeCallback(say_hello))
    ns.Simulator.Stop(ns.Seconds(2.0))
    ns.Simulator.Run()
    ns.Simulator.Destroy()

if __name__ == "__main__":
    main()
