#!/bin/bash
apt update

REQUIRED_PACKAGES="git gcc g++ cmake qt6-base-dev qt6-tools-dev libfreetype-dev nlohmann-json3-dev"

apt -y install $REQUIRED_PACKAGES
