# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_nx_integration
# Description   :: Adding reference to Nextlabs .dll file in NX startup process
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'reconfig_custom_dirs' do
    block do
        log_info(LOGMSG::UPGRADE_NX_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # If old RMX was 5.0 then need to remove NXIntegration dir from custom_dirs (In this version, we still use Ruby script to install feature)
        if !node['OLD_RMX_VERSION'].nil? && node['OLD_RMX_VERSION'].start_with?('5.0')
            log_info("Upgrading RMX from #{node['OLD_RMX_VERSION']}")
            ugii_root = ENV['UGII_ROOT_DIR']
            ugii_root = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root.nil? && !ENV['UGII_BASE_DIR'].nil?

            custom_dirs = File.join(ugii_root, 'menus', 'custom_dirs.dat')
            custom_dirs_old = File.join(ugii_root, 'menus', 'custom_dirs.dat.old')

            if File.exist?(custom_dirs_old) && RUBY_PLATFORM =~ /mswin|mingw|windows/
                remove_readonly_old_cmd = "attrib -R \"#{custom_dirs_old}\""
                call_system_command('Remove read-only on old custom_dirs.dat file', 'attrib-R', remove_readonly_old_cmd)
            end

            # In this step need to remove all old NXIntegration dir and rerun NXRMX.msi to fully install
            delete_file_content_with_label(custom_dirs, 'NXIntegration')
        end
        # Running Windows Installer for NX RMX (Install/Update-Install mode)
        nxrmx_msi_installer = File.join(ENV['RMX_ROOT'], 'NextLabs Rights Management For NX.msi').tr('/', '\\')
        log_location = File.join(node['temp_dir'], 'nxrmx_install_log.txt')
        run_msi_cmd = "msiexec /qn /i \"#{nxrmx_msi_installer}\" /L*V \"#{log_location}\""
        log_info('Running NextLabs Rights Managment eXtension For NX Window Installer')
        call_system_command('Run NX RMX installer', 'msiexec', run_msi_cmd)
    end
end
