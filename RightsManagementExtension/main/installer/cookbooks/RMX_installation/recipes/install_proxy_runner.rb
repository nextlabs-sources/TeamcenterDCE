# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_proxy_runner
# Description   :: Install Nextlabs proxy runner
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_proxy_runner' do
    block do
        log_info(LOGMSG::INSTALL_PROXY_RUNNER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # ProxyRunner.exe now reside in %RMX_ROOT%\Runners folder
        log_info('Copy ProxyRunner.exe to Teamcenter portal directory')
        proxy_runner = File.join(ENV['RMX_ROOT'], 'NxlAutoProtect', 'Runners', 'ProxyRunner.exe')
        tc_portal = File.join(node['TC_ROOT'], 'portal')
        FileUtils.cp_r(proxy_runner, tc_portal)

        log_info('Inserting ProxyRunner')
        tc_runner = File.join(tc_portal, 'runner.exe')
        tc_runner_new_name = File.join(tc_portal, 'tcrunner.exe')
        nextlabs_tc_runner = File.join(tc_portal, 'ProxyRunner.exe')
        backup_old_file(tc_runner, tc_portal)
        # After rename, NextLabs ProxyRunner.exe should be runner.exe && Teamcenter runner.exe will be tcrunner.exe
        unless File.exist?(tc_runner_new_name)
            File.rename(tc_runner, tc_runner_new_name)
            File.rename(nextlabs_tc_runner, tc_runner)
        end
    end
end
