.PHONY: help docker-build shell check dev lab0

IMAGE ?= ns3-3.47:latest

help:
	@echo "Targets:"
	@echo "  docker-build   Build the Docker image (ns-3.47)"
	@echo "  shell          Run an interactive shell inside the container (mounts repo at /work)"
	@echo "  check          Run CI smoke tests inside the container"
	@echo "  lab0           Run the first Lab-00 Python script found (if any)"
	@echo "  dev            VS Code devcontainer: see .devcontainer/devcontainer.json"

docker-build:
	docker build -t $(IMAGE) .

shell: docker-build
	@# Start or attach to a persistent container, but keep the same entrypoint (bash)
	@if docker ps -a --format '{{.Names}}' | grep -qx 'ns3dev'; then \
		echo "[shell] Attaching to existing container ns3dev"; \
		docker start -ai ns3dev; \
	else \
		echo "[shell] Creating container ns3dev"; \
		docker run --name ns3dev -it -v $$PWD:/work -w /work --init $(IMAGE) bash; \
	fi

check: docker-build
	docker run --rm -v $$PWD:/work -w /work $(IMAGE) bash -lc "scripts/ci_smoke.sh"

lab0: docker-build
	docker run --rm -v $$PWD:/work -w /work $(IMAGE) bash -lc 'source scripts/setup_env.sh && f=$$(ls -1 Lab-00*/code/*.py 2>/dev/null | head -n1); if [ -n "$$f" ]; then python3 "$$f"; else echo "No Lab-00 Python script found"; fi'

dev:
	@echo "Open this folder in VS Code and 'Reopen in Container' (Dev Containers extension)."