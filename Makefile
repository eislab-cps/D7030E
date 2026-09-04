.PHONY: help setup check lab0 run

NS3_DIR ?= $(HOME)/ns-allinone-3.47/ns-3.47
FILE ?=
ARGS ?=

help:
	@echo "Targets:"
	@echo "  setup  Install and build native ns-3.47 in WSL"
	@echo "  check  Verify the native ns-3.47 C++ environment"
	@echo "  lab0   Build and run the Lab 00 C++ starter"
	@echo "  run    Run FILE=<path-to-.cc> with optional ARGS='--name=value'"

setup:
	scripts/install_wsl.sh

check:
	NS3_DIR="$(NS3_DIR)" scripts/ci_smoke.sh

lab0:
	NS3_DIR="$(NS3_DIR)" scripts/run_cpp.sh Lab-00-Introduction/code/Lab0_Cpp_Hello.cc

run:
	@test -n "$(FILE)" || (echo "Usage: make run FILE=Lab-01-Propagation/code/Lab1_Cpp_Friis.cc ARGS='--distance=100'" && exit 2)
	NS3_DIR="$(NS3_DIR)" scripts/run_cpp.sh "$(FILE)" $(ARGS)
