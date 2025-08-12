#!/bin/bash

# List of Open5GS services to stop
services=(
    "open5gs-mmed"
    "open5gs-sgwcd"
    "open5gs-smfd"
    "open5gs-amfd"
    "open5gs-sgwud"
    "open5gs-upfd"
    "open5gs-hssd"
    "open5gs-pcrfd"
    "open5gs-nrfd"
    "open5gs-scpd"
    "open5gs-seppd"
    "open5gs-ausfd"
    "open5gs-udmd"
    "open5gs-pcfd"
    "open5gs-nssfd"
    "open5gs-bsfd"
    "open5gs-udrd"
    "open5gs-webui"
)

# Stop each service
for service in "${services[@]}"; do
    echo "Stopping $service..."
    if sudo systemctl stop "$service"; then
        echo "$service stopped successfully."
    else
        echo "Failed to stop $service." >&2
    fi
done

echo "All Open5GS services processed."
