# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_nxl_ipem_template
# Description   :: Deploy Nextlabs template support Creo files
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)
