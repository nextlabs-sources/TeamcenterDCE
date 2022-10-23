# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_bb_integration
# Description   :: Change __nx.jar file and cutomize BB LaunchFile.bat
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_bb_integration' do
    block do
        log_info(LOGMSG::UPGRADE_BB_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)
        # Place holder for this recipe
    end
end
