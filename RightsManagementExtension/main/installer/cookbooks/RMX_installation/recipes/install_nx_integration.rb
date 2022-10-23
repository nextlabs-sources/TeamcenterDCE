# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: install_nx_integration
# Description   :: Run RMX For NX msi installer
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'edit_custom_dirs' do
    block do
        log_info(LOGMSG::INSTALL_NX_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        unless is_cad_rmx_installed(:nx)
            # Running Windows Installer for NX RMX (Fresh Install mode). INSTALLFOLDER will always be default folder in [NextLabs]\NXRMX
            nxrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For NX.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'nxrmx_install_log.txt')
            msi_cmd = "msiexec /qn /i \"#{nxrmx_msi_installer}\" /L*V \"#{log_location}\""
            log_info('Running NextLabs Rights Managment eXtension For NX Windows Installer')
            call_system_command('Run NX RMX installer', 'msiexec', msi_cmd)
        end
    end
end
