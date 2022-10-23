# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_workflow_initial_protection
# Description   :: Deploy dll files to support workflow protection
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_workflow_initial_protection' do
    block do
        log_info(LOGMSG::INSTALL_WORKFLOW_INITIAL_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        tc_bin = 'TC_BIN' + node['tc_version']
        # bug fix 41598
        # stop Teamcenter pool manager service before replacing dll files
        # otherwise, it would have en error "Permission denied"
        tc_pool_service = node['service_tcserver']
        log_info('Stop Teamcenter Pool Manager')
        stop_system_service(tc_pool_service)

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            des_dir = File.join(node['TC_ROOT'], 'bin')
            src_dir = File.join(ENV['RMX_ROOT'], 'NxlAutoProtect', tc_bin, 'Windows')
        else
            des_dir = File.join(node['TC_ROOT'], 'lib')
            src_dir = File.join(ENV['RMX_ROOT'], 'NxlAutoProtect', tc_bin, 'Linux')
        end

        if File.exist?(des_dir)
            log_info('Copy Nextlabs dll files to Teamcenter bin directory...')
            copy_files(src_dir, des_dir)
        else
            Chef::Log.error("#{des_dir} does not exist in this system")
        end

        log_info('Start Teamcenter Pool Manager')
        start_system_service(tc_pool_service)
    end
end