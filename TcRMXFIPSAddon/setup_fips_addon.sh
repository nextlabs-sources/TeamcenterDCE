#!/usr/bin/env bash
# 
#
#  /$$   /$$                    /$$    /$$               /$$                      /$$$$$$
# | $$$ | $$                   | $$   | $$              | $$                     |_  $$_/
# | $$$$| $$ /$$$$$$ /$$   /$$/$$$$$$ | $$       /$$$$$$| $$$$$$$  /$$$$$$$        | $$  /$$$$$$$  /$$$$$$$
# | $$ $$ $$/$$__  $|  $$ /$$|_  $$_/ | $$      |____  $| $$__  $$/$$_____/        | $$ | $$__  $$/$$_____/
# | $$  $$$| $$$$$$$$\  $$$$/  | $$   | $$       /$$$$$$| $$  \ $|  $$$$$$         | $$ | $$  \ $| $$
# | $$\  $$| $$_____/ >$$  $$  | $$ /$| $$      /$$__  $| $$  | $$\____  $$        | $$ | $$  | $| $$
# | $$ \  $|  $$$$$$$/$$/\  $$ |  $$$$| $$$$$$$|  $$$$$$| $$$$$$$//$$$$$$$/       /$$$$$| $$  | $|  $$$$$$$/$$
# |__/  \__/\_______|__/  \__/  \___/ |________/\_______|_______/|_______/       |______|__/  |__/\_______|__/
#
#
# Teamcenter Rights Management eXtension FIPS SDK Add-on Installer for Windows Systems
# version : Teamcenter RMX 5.3
#
# Copyright 2020, Nextlabs Inc.
# All rights reserved - Do Not Redistribute 
#

# The program should be executed by root
[ "$(whoami)" != "root" ] && exec sudo -- "$0" "$@"

# Ignore SIGINT
# This is needed, because to stop the installation half way, we send
# SIGINT to the process group which includes this script process
trap '' INT

# START_DIR is used by UI and chef recipes, must be set before proceed
export START_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "Current working directory:${START_DIR}"

echo "Renew FIPS SDK Library Files"
mv fips fips_old
mv fips_new fips

echo "Uncompressing engine_linux"
unzip -o -q "${START_DIR}/engine/engine_linux.zip" -d /opt
echo "Finished uncompressing engine"

echo "Copy required gems to chef embedded ruby"
unzip -o -q "${START_DIR}/engine/gems_linux.zip" -d /tmp
cp -rf /tmp/gems/gems/* /opt/chef/embedded/lib/ruby/gems/2.1.0/gems/
cp -rf /tmp/gems/specifications/* /opt/chef/embedded/lib/ruby/gems/2.1.0/specifications/
cp -rf /tmp/gems/extensions/x86_64-linux/2.1.0/* /opt/chef/embedded/lib/ruby/gems/2.1.0/extensions/x86_64-linux/2.1.0/
cp -rf ${START_DIR}/engine/rubyzip-1.2.0/ /opt/chef/embedded/lib/ruby/gems/2.1.0/gems/
cp -rf ${START_DIR}/engine/rubyzip-1.2.0.gemspec /opt/chef/embedded/lib/ruby/gems/2.1.0/specifications/
echo "Finished copy gems"

echo "Starting FIPS SDK Updater"
/opt/chef/embedded/bin/ruby "${START_DIR}/fips/install_fips_sdk.rb"

echo "FIPS SDK Update finished"
