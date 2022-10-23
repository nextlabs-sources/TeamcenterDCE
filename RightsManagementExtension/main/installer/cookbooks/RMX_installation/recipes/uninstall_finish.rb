# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_finish
# Description   :: Remove TcDRM folder, Recover JCEPolicy files, Remove registry
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_finish' do
    block do
        log_info('[Progress] Uninstall finishing')
        log_info('Clean up installation files')
        FileUtils.remove_file(File.join(node['temp_dir'], 'rmx_properties_ui.json'), true) if ENV['rollback'] != 'true'
        FileUtils.remove_file(File.join(node['temp_dir'], InstallFile::HAS_INSTALLED_TXT), true)
        FileUtils.remove_file(File.join(node['temp_dir'], 'break_point.json'), true)
        # remove TcDRM from install directory
        tcdrm = File.join(node['install_dir'], 'TcDRM')
        FileUtils.remove_dir(tcdrm, true)

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            # remove RMX_ROOT env var during uninstallation
            remove_environment_variable('RMX_ROOT', :system)
        end

        log_info(LOGMSG::UNINSTALL_FINISHED)
    end
end
