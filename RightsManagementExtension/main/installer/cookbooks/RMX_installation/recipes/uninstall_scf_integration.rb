# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_scf_integration
# Description   :: Remove Nextlabs files relate to SCF process
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'remove_scf_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_SCF_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            # Remove NxlTcImportIntegration from NX customdirs.dat
            ugii_root_dir = ENV['UGII_ROOT_DIR']
            ugii_root_dir = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root_dir.nil? && !ENV['UGII_BASE_DIR'].nil?
            custom_dir_file = File.join(ugii_root_dir, 'menus', 'custom_dirs.dat')

            if File.exist?(custom_dir_file)
                log_info('Recover custom_dirs.dat file')
                delete_file_content_with_label(custom_dir_file, 'NxlSCFIntegration')
            end

            # Remove dlls from TC_ROOT/bin
            log_info('Remove NextLabs dll in TC_ROOT/bin')
            FileUtils.remove_file(File.join(node['TC_ROOT'], 'bin', 'NxlSCFIntegration.dll'), true)

            # Remove TRANSLATOR_HOME environment variable
            remove_environment_variable('TRANSLATOR_HOME', :system)

        elsif RUBY_PLATFORM =~ /linux/
            # Remove NxlTcImportIntegration from NX customdirs.dat
            ugii_root_dir = ENV['UGII_ROOT_DIR']
            ugii_root_dir = File.join(ENV['UGII_BASE_DIR'], 'ugii') if ugii_root_dir.nil? && !ENV['UGII_BASE_DIR'].nil?
            custom_dir_file = File.join(ugii_root_dir, 'menus', 'custom_dirs.dat')

            if File.exist?(custom_dir_file)
                log_info('Recover custom_dirs.dat file')
                delete_file_content_with_label(custom_dir_file, 'NxlSCFIntegration')
            end

            # Copy so files to TC_ROOT/bin
            tc_version = node['tc_version']
            FileUtils.remove_file(File.join(node['TC_ROOT'], 'lib', 'NxlSCFIntegration.so'), true)

            # Remove TRANSLATOR_HOME env
        end

        # Detach SCF XLST hook
        log_info('Detach Nextlabs XSLT file for SCFIntegration module')
        detach_utility = File.join(node['TC_ROOT'], 'bin', TeamcenterProcess::XSLT_UTILITY)
        detach_utility = detach_utility.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        xsl_file = File.join(ENV['RMX_ROOT'], 'XSLT', 'remove_nxl.xslt')
        detach_command = "#{node['tc_profilevar']} && \"#{detach_utility}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -transfermode=TIEUnmanagedExportForNX_TM -xsl_file=\"#{xsl_file}\" -action=detach"
        call_system_command('Detach NextLabs SCF XSLT', 'plmxml_tm_edit_xsl', detach_command, :no)
    end
end
