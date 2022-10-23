# Teamcenter RMX 5.2
# Cookbook Name :: RMX_installation
# Recipes       :: install_solidworks_integration
# Description   :: Deploy SolidWorks RMX by running Window Installer
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_solidworks_integration' do
    block do
        log_info(LOGMSG::INSTALL_SOLIDWORKS_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        unless is_cad_rmx_installed(:solidworks)
            # Running Windows Installer for SolidWorks RMX (Fresh Install mode). INSTALLFOLDER will always be default folder [Nextlabs]\SldWorksRMX
            solidworksrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For SolidWorks.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'sldworksrmx_install_log.txt')
            msi_cmd = "msiexec /qn /i \"#{solidworksrmx_msi_installer}\" SLDWORKS_DIR=\"#{node['solidworks_dir'].tr('/', '\\')}\" SWIM=\"#{node['swim_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
            if node['swim_dir'] == ''
                msi_cmd = "msiexec /qn /i \"#{solidworksrmx_msi_installer}\" SLDWORKS_DIR=\"#{node['solidworks_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
            end
            log_info('Running NextLabs Rights Managment eXtension For SolidWorks Windows Installer')
            call_system_command('Run SolidWorks RMX installer', 'msiexec', msi_cmd)
        end
    end
end
