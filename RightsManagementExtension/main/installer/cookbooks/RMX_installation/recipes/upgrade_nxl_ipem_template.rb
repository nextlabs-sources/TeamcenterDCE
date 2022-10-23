# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_nxl_ipem_template
# Description   :: Upgrade Nextlabs template support Creo files
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)
