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
# Teamcenter Rights Management eXtension Installer GUI Launcher for Linux Systems
# version : Teamcenter RMX 5.1
#
# Copyright 2019, Nextlabs Inc.
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

# export "UI_LOG_LOCATION=${START_DIR}/installer.log"
# Location for storing generated json properties file from UI for later reference
# export "CHEF_JSON_PROPERTIES_FILE_BACKUP_LOCATION=${START_DIR}/rmx_properties_ui.json"

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

echo "Starting GUI installer"
/opt/chef/embedded/bin/ruby "${START_DIR}/ui/app.rb"

echo "Removing and Cleaning files"
if [ -d /opt/chef ]; then
	rm -rf /opt/chef
fi
if [ -d /tmp/gems ]; then
	rm -rf /tmp/gems
fi
if [ -f /tmp/client.rb ]; then
	rm -f /tmp/client.rb
fi
if [ -f /tmp/cc_properties.json ]; then
	rm -f /tmp/cc_properties.json
fi
if [ -d /tmp/local-mode-cache ]; then
	rm -rf /tmp/local-mode-cache
fi
echo "Installer execution completed"
