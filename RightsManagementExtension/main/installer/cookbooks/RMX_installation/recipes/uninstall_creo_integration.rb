# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_creo_integration
# Description   :: Remove creotk.dat file in Creo plugin folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_creo_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_CREO_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if is_cad_rmx_installed(:creo)
            log_info('Remove NextLabs Rights Management For Creo by msi installer')
            creormx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For Creo.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'creormx_uninstall_log.txt')
            msi_cmd = "msiexec /qn /x \"#{creormx_msi_installer}\" /L*V \"#{log_location}\""
            call_system_command('Run Creo RMX installer', 'msiexec', msi_cmd)
        end
    end
end
