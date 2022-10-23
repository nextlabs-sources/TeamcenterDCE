# Teamcenter RMX 5.4
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_rights_management_desktop
# Description   :: No action here
# Author        :: Phan Anh Tuan
# Copyright 2020, Nextlabs Inc.

require 'fileutils'
require 'json'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/