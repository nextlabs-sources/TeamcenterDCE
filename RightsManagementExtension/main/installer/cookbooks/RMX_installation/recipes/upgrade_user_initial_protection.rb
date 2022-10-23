# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_user_initial_protection
# Description   :: Update menuitem jar and rerun nxlrunner registry
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_user_ini_pro' do
    block do
        log_info(LOGMSG::UPGRADE_USER_INITIAL_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Copy com.nextlabs.menuitem.jar to TC_ROOT plugins folder')
        tc_plugins_dir = File.join(node['TC_ROOT'], 'portal', 'plugins')
        menu_item = File.join(ENV['RMX_ROOT'], 'Customized Menu Item', 'com.nextlabs.menuitem.jar')
        FileUtils.cp_r(menu_item, tc_plugins_dir)

        log_info('Registering NextLabs plug-in into the Rich Client Application')
        genregxml = File.join(node['TC_ROOT'], 'portal', 'registry', Script::GENREGXML)
        genregxml_cmd = "\"#{genregxml}\""
        call_system_command('Update Nextlabs MenuItem plugin', 'genregxml', genregxml_cmd)

        log_info('Update nxlrunner into registry')
        nxlrunner_reg = File.join(ENV['RMX_ROOT'], 'Setup', 'nxlrunner.reg')
        system("reg import \"#{nxlrunner_reg}\" /reg:64")
    end
end
