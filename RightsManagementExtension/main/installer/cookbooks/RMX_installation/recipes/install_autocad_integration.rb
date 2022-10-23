# Teamcenter RMX 13.3
# Cookbook Name :: RMX_installation
# Recipes       :: install_autocad_integration
# Description   :: Run AutoCAD RMX installer (msi)
# Author        :: Phan Anh Tuan
# Copyright 2022, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_autocad_integration' do
    block do
        log_info(LOGMSG::INSTALL_AUTOCAD_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        unless is_cad_rmx_installed(:autocad)
            # Running Windows Installer for AutoCAD RMX (Fresh Install mode). INSTALLFOLDER will always be default folder [Nextlabs]\AutoCADRMX
            autocadrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For AutoCAD.msi').tr('/', '\\')

            log_location = File.join(node['temp_dir'], 'autocadrmx_install_log.txt')
            msi_cmd = "msiexec /qn /i \"#{autocadrmx_msi_installer}\" ACAD_DIR=\"#{node['autocad_dir'].tr('/', '\\')}\" /L*V \"#{log_location}\""
            log_info('Running NextLabs Rights Managment eXtension For AutoCAD Windows Installer')
            call_system_command('Run AutoCAD RMX installer', 'msiexec', msi_cmd)
        end
    end
end
