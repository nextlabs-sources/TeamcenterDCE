# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_creo_integration
# Description   :: Register creotk.dat file to Creo plugin folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_creo_integration' do
    block do
        log_info(LOGMSG::INSTALL_CREO_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        unless is_cad_rmx_installed(:creo)
            # Running Windows Installer for Creo RMX (Fresh Install mode). INSTALLFOLDER will always be default folder [Nextlabs]\CreoRMX
            creormx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For Creo.msi').tr('/', '\\')

            creo_common_dir = File.expand_path('../Common Files', node['creo_dir'])

            log_location = File.join(node['temp_dir'], 'creormx_install_log.txt')
            msi_cmd = "msiexec /qn /i \"#{creormx_msi_installer}\" CREO_DIR=\"#{node['creo_dir'].tr('/', '\\')}\" IPEM=\"#{node['ipem_dir'].tr('/', '\\')}\" CREO_COMMON_DIR=\"#{creo_common_dir.tr('/', '\\')}\" /L*V \"#{log_location}\""
            if node['ipem_dir'] == ''
                msi_cmd = "msiexec /qn /i \"#{creormx_msi_installer}\" CREO_DIR=\"#{node['creo_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
            end

            log_info('Running NextLabs Rights Managment eXtension For Creo Windows Installer')
            call_system_command('Run Creo RMX installer', 'msiexec', msi_cmd)
        end
    end
end
