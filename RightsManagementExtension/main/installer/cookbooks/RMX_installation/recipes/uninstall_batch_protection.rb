# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_batch_protection
# Description   :: Do nothing
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_batch_protection' do
    block do
        log_info(LOGMSG::UNINSTALL_BATCH_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)
    end
end