# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_scf_integration
# Description   :: Move Nextlabs .dll files to TC_BIN relate to SCF process
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_scf_integration' do
    block do
        log_info(LOGMSG::UPGRADE_SCF_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)
    end
end

