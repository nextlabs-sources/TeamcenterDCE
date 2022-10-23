# Teamcenter RMX 5.2
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_solidworks_integration
# Description   :: Remove SolidWorks RMX by running Window Installer
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_solidworks_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_SOLIDWORKS_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if is_cad_rmx_installed(:solidworks)
            log_info('Remove NextLabs Rights Management For SolidWorks by msi installer')
            solidworksrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For SolidWorks.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'sldworksrmx_uninstall_log.txt')
            msi_cmd = "msiexec /qn /x \"#{solidworksrmx_msi_installer}\" /L*V \"#{log_location}\""
            call_system_command('Run SolidWorks RMX installer', 'msiexec', msi_cmd)
        end
    end
end
