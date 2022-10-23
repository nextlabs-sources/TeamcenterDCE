#!/bin/bash
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
#
# Copyright 2019, Nextlabs Inc.
# Author:: Phan Anh Tuan
#
# All rights reserved - Do Not Redistribute
#
# The program should be executed by root
[ "$(whoami)" != "root" ] && exec sudo -- "$0" "$@"

export START_DIR=$( dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" )

echo "Current working directory:${START_DIR}"
echo "Uncompressing engine_linux to temp folder"
unzip -o -q "${START_DIR}/engine/engine_linux.zip" -d /opt
echo "Finished Uncompressing chef"


echo "Copy required gems to chef embedded ruby"
unzip -o -q "${START_DIR}/engine/gems_linux.zip" -d /tmp
cp -rf /tmp/gems/gems/* /opt/chef/embedded/lib/ruby/gems/2.1.0/gems/
cp -rf /tmp/gems/specifications/* /opt/chef/embedded/lib/ruby/gems/2.1.0/specifications/
cp -rf /tmp/gems/extensions/x86_64-linux/2.1.0/* /opt/chef/embedded/lib/ruby/gems/2.1.0/extensions/x86_64-linux/2.1.0/
cp -rf ${START_DIR}/engine/rubyzip-1.2.0/ /opt/chef/embedded/lib/ruby/gems/2.1.0/gems/
cp -rf ${START_DIR}/engine/rubyzip-1.2.0.gemspec /opt/chef/embedded/lib/ruby/gems/2.1.0/specifications/
echo "Finished copy gems"


echo "Starting Silence Installer..."
/opt/chef/bin/chef-client --config "${START_DIR}/silence/config.rb" -o RMX_installation::main

echo "Removing and Cleaning files"

rm -rf /opt/chef
rm -rf /tmp/client.rb
rm -rf "${START_DIR}/local_mode_cache"
rm -rf "${START_DIR}/nodes"

echo "Installer execution completed"
