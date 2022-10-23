# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_finish
# Description   :: Put .txt file indicate RMX is installed. Update RMX registry
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/
ruby_block 'upgrade_finish' do
    block do
        FileUtils.mkdir_p(node['temp_dir'])
        file_path = File.join(node['temp_dir'], InstallFile::HAS_INSTALLED_TXT)
        File.new(file_path, 'w+')
        FileUtils.remove_file(File.join(node['temp_dir'], 'break_point.json'), true)
        FileUtils.remove_file(File.join(node['temp_dir'], 'config.properties'), true) if File.exist?(File.join(node['temp_dir'], 'config.properties'))
        FileUtils.remove_file(File.join(node['temp_dir'], 'connection.properties'), true) if File.exist?(File.join(node['temp_dir'], 'connection.properties'))
        FileUtils.remove_file(File.join(node['temp_dir'], 'policy_evaluation.properties'), true) if File.exist?(File.join(node['temp_dir'], 'policy_evaluation.properties'))

        # write solution type to registry on Windows
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            root_registry = ::Win32::Registry::HKEY_LOCAL_MACHINE.open('SOFTWARE\Classes')
            tc_rmx = root_registry.create('Nextlabs').create('Teamcenter Rights Management Extension')
            tc_rmx.write_s('solution', node['solution_type'])
            tc_rmx.write_s('version', '5.2.0.0')
        end
        log_info(LOGMSG::UPGRADE_FINISHED)
    end
end
