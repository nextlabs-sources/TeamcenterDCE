# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_workflow_initial_protection
# Description   :: Remove dll files relate to workflow protection
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'remove_workflow_initial_protection' do
    block do
        log_info(LOGMSG::UNINSTALL_WORKFLOW_INITIAL_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        tc_pool_manager = node['service_tcserver']
        log_info('Stop Teamcenter Pool Manager')
        stop_system_service(tc_pool_manager)

        log_info('Remove Nextlabs lib and NlJavaSDK')
        src_folder = File.join(node['TC_ROOT'], 'bin') # if RUBY_PLATFORM =~ /mswin|mingw|windows/
        src_folder = File.join(node['TC_ROOT'], 'lib') if RUBY_PLATFORM =~ /linux/
        lib_util = RMXLibrary::NXLUTILS64 # if RUBY_PLATFORM =~ /mswin|mingw|windows/
        lib_util = RMXLibrary::LIBNXLUTILS if RUBY_PLATFORM =~ /linux/
        filenames = [RMXLibrary::NXLAUTOPROTECT, lib_util, 'nlJavaSDK2.jar']
        delete_files(src_folder, filenames)

        log_info('Start Teamcenter Pool Manager')
        start_system_service(tc_pool_manager)
    end
end
