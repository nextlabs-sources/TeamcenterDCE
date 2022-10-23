# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_user_initial_protection
# Description   :: Remove nxlrunner from portal folder. Delete regedit
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'win32/registry'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'remove_user_initial_protection' do
    block do
        log_info(LOGMSG::UNINSTALL_USER_INITIAL_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Remove com.nextlabs.menuitem.jar from TC_ROOT plugins folder')
        plugin_dir = File.join(node['TC_ROOT'], 'portal', 'plugins')
        FileUtils.remove_file(File.join(plugin_dir, 'com.nextlabs.menuitem.jar'), true)

        # Disable this part. Genregxml will run in RAC RMX feature as well
        # log_info('Remove NextLabs plugin from Rich Client Application')
        # genregxml = File.join(node['TC_ROOT'], 'portal', 'registry', Script::GENREGXML)
        # genregxml_cmd = "\"#{genregxml}\""
        #call_system_command('Remove Nextlabs MenuItem plugin', 'genregxml', genregxml_cmd)

        log_info('Remove nxlrunner in registry')
        system('reg delete "HKEY_CLASSES_ROOT\\nxlfile" /f /reg:64')
        system('reg delete "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextlabs\\TeamcenterRMX" /f /reg:64')
    end
end
