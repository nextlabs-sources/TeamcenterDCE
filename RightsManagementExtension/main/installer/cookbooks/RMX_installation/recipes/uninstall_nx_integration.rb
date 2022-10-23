# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_nx_integration
# Description   :: Remove reference to Nextlabs .dll file in NX startup process
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_nx_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_NX_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if is_cad_rmx_installed(:nx)
            log_info('Remove NextLabs Rights Management For NX by msi installer')
            nxrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For NX.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'nxrmx_uninstall_log.txt')
            msi_cmd = "msiexec /qn /x \"#{nxrmx_msi_installer}\" /L*V \"#{log_location}\""
            call_system_command('Run NX RMX installer', 'msiexec', msi_cmd)
        end
    end
end
