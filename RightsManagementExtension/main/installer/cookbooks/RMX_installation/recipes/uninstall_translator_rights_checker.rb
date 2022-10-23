# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_translator_rights_checker
# Description   :: No task here
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_translator_rights_checker' do
    block do
        log_info(LOGMSG::UNINSTALL_TRANSLATOR_RIGHTS_CHECKER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)
    end
end
