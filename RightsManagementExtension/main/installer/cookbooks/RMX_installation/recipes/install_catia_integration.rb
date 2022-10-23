# Teamcenter RMX 13.3
# Cookbook Name :: RMX_installation
# Recipes       :: install_catia_integration
# Description   :: Run CATIA RMX installer (msi)
# Author        :: Phan Anh Tuan
# Copyright 2022, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_catia_integration' do
    block do
        log_info(LOGMSG::INSTALL_CATIA_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        unless is_cad_rmx_installed(:catia)
            # Running Windows Installer for CATIA RMX (Fresh Install mode). INSTALLFOLDER will always be default folder [Nextlabs]\CATIARMX
            catiarmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For CATIA.msi').tr('/', '\\')

            log_location = File.join(node['temp_dir'], 'catiarmx_install_log.txt')
            msi_cmd = "msiexec /qn /i \"#{catiarmx_msi_installer}\" CATIA_DIR=\"#{node['catia_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
            log_info('Running NextLabs Rights Managment eXtension For CATIA Windows Installer')
            call_system_command('Run CATIA RMX installer', 'msiexec', msi_cmd)
        end
    end
end
