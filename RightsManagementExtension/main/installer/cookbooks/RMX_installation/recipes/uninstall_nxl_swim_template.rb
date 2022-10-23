# Teamcenter RMX 5.2
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_nxl_swim_template
# Description   :: Restore Nextlabs template support SolidWorks files
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)
