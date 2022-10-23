# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_user_initial_protection
# Description   :: Add nxlrunner to portal folder. Add regedit
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_user_initial_protection' do
    block do
        log_info(LOGMSG::INSTALL_USER_INITIAL_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Copy com.nextlabs.menuitem.jar to TC_ROOT plugins folder')
        tc_plugins_dir = File.join(node['TC_ROOT'], 'portal', 'plugins')
        if !File.exist?(tc_plugins_dir)
            Chef::Log.error('TC plugins directory does not exist in the system')
        else
            menu_item = File.join(ENV['RMX_ROOT'], 'Customized Menu Item', 'com.nextlabs.menuitem.jar')
            FileUtils.cp_r(menu_item, tc_plugins_dir)
        end

        # In RAC RMX, genregxml will be deployed as well. Disable this part
        # log_info('Registering NextLabs plug-in into the Rich Client Application')
        # genregxml = File.join(node['TC_ROOT'], 'portal', 'registry', Script::GENREGXML)
        # genregxml_cmd = "\"#{genregxml}\""
        # call_system_command('Register Nextlabs MenuItem plugin', 'genregxml', genregxml_cmd)

        log_info('Register nxlrunner into registry')
        nxlrunner_reg = File.join(ENV['RMX_ROOT'], 'Setup', 'nxlrunner.reg')
        system("reg import \"#{nxlrunner_reg}\" /reg:64")
    end
end
