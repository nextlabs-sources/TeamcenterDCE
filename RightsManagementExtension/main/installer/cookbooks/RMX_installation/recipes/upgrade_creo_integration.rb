# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_creo_integration
# Description   :: Register creotk.dat file to Creo plugin folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_creo_integration' do
    block do
        log_info(LOGMSG::UPGRADE_CREO_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        creormx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For Creo.msi').tr('/', '\\')
        log_location = File.join(ENV['TEMP'], 'creormx_install_log.txt')
        run_msi_cmd = "msiexec /qn /i \"#{creormx_msi_installer}\" CREO_DIR=\"#{node['creo_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
        log_info('Running NextLabs Rights Managment eXtension For Creo Window Installer')
        call_system_command('Run Creo RMX installer', 'msiexec', run_msi_cmd)
    end
end
