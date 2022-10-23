# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_proxy_runner
# Description   :: Re-deploy Nextlabs proxy runner
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_proxy_runner' do
    block do
        log_info(LOGMSG::UPGRADE_PROXY_RUNNER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # ProxyRunner.exe now reside in %RMX_ROOT%\Runners folder
        log_info('Replace NextLabs ProxyRunner.exe in Teamcenter portal directory')
        proxy_runner = File.join(ENV['RMX_ROOT'], 'NxlAutoProtect', 'Runners', 'ProxyRunner.exe')
        tc_portal = File.join(node['TC_ROOT'], 'portal')
        FileUtils.cp_r(proxy_runner, tc_portal)

        original_tcrunner = File.join(tc_portal, 'tcrunner.exe')
        nextlabs_tcrunner = File.join(tc_portal, 'runner.exe')
        new_nextlabs_tc_runner = File.join(tc_portal, 'ProxyRunner.exe')
        log_info('Inserting new runner.exe')
        if File.exist?(original_tcrunner) && File.exist?(nextlabs_tcrunner)
            File.rename(new_nextlabs_tc_runner, nextlabs_tcrunner)
        end
    end
end
