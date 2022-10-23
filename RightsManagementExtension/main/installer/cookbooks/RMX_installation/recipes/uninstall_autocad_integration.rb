# Teamcenter RMX 13.3
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_autocad_integration
# Description   :: Remove AutoCAD RMX by running Window Installer
# Author        :: Phan Anh Tuan
# Copyright 2022, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_autocad_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_AUTOCAD_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if is_cad_rmx_installed(:autocad)
            log_info('Remove NextLabs Rights Management For AutoCAD by msi installer')
            autocadrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For AutoCAD.msi').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'autocadrmx_uninstall_log.txt')
            msi_cmd = "msiexec /qn /x \"#{autocadrmx_msi_installer}\" /L*V \"#{log_location}\""
            call_system_command('Run AutoCAD RMX installer', 'msiexec', msi_cmd)
        end
    end
end
