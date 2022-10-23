# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_proxy_runner
# Description   :: Remove Nextlabs proxy runner
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'remove_proxy_runner' do
    block do
        log_info(LOGMSG::UNINSTALL_PROXY_RUNNER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Recover original Teamcenter runner.exe...')
        tc_portal = File.join(node['TC_ROOT'], 'portal')

        # rename Teamcenter runner
        newfile = File.join(tc_portal, 'tcrunner.exe')
        oldfile = File.join(tc_portal, 'runner.exe')
        runner_old = File.join(tc_portal, 'runner.exe.old')

        if File.exist?(newfile)
            if File.exist?(oldfile)
                FileUtils.remove_file(oldfile, true)
                FileUtils.remove_file(runner_old, true)
            end
            File.rename(newfile, oldfile)
        end
    end
end
