# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_scf_integration
# Description   :: Move Nextlabs .dll files to TC_BIN relate to SCF process
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_scf_integration' do
    block do
        log_info(LOGMSG::INSTALL_SCF_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            # Add NxlTcImportIntegration into NX customdirs.dat. This installation assume NX also installed in same machine
            ugii_root_dir = ENV['UGII_ROOT_DIR']
            ugii_root_dir = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root_dir.nil? && !ENV['UGII_BASE_DIR'].nil?
            custom_dir_file = File.join(ugii_root_dir, 'menus', 'custom_dirs.dat')

            if File.exist?(custom_dir_file) && !regexp?(custom_dir_file, /.*NxlSCFIntegration.*/)
                log_info('Remove read-only attribute from custom_dirs.dat file...')
                remove_readonly_old_attr = "attrib -R \"#{custom_dir_file}\""
                call_system_command('Remove read-only on old custom_dirs.dat file', 'attrib-R', remove_readonly_old_attr)

                log_info('Edit custom_dirs.dat file')
                nxl_scf_integration = File.join(ENV['RMX_ROOT'], 'SCFIntegration', 'NX_RMX_ROOT', 'NxlTcImportIntegration').tr('/', '\\')
                file = File.new(custom_dir_file, 'a+')
                file.puts "\n" unless file_endwith_new_line(custom_dir_file)
                file.puts '# nextlabs NxlSCFIntegration - start'
                file.puts nxl_scf_integration
                file.puts '# nextlabs NxlSCFIntegration - end'
                file.close
            end

            # Copy dlls to TC_ROOT/bin
            tc_version = node['tc_version']
            dll_file = File.join(ENV['RMX_ROOT'], 'SCFIntegration', "TC_BIN#{tc_version}", 'NxlSCFIntegration.dll')
            FileUtils.cp_r(dll_file, File.join(node['TC_ROOT'], 'bin'))

            # Copy bczmodifier.bat to %RMX_ROOT%\shared_libs (should be done with build_compile)
            # Add TRANSLATOR_HOME environment variable
            translators_folder = File.join(node['DISP_ROOT'], 'Module', 'Translators') if File.exist?(node['DISP_ROOT'])
            translators_folder = translators_folder.tr('/', '\\')
            set_environment_variable('TRANSLATOR_HOME', translators_folder, :system)

        elsif RUBY_PLATFORM =~ /linux/
            # Add NxlTcImportIntegration into NX customdirs.dat. This installation assume NX also installed in same machine
            ugii_root_dir = ENV['UGII_ROOT_DIR']
            ugii_root_dir = File.join(ENV['UGII_BASE_DIR'], 'ugii') if ugii_root_dir.nil? && !ENV['UGII_BASE_DIR'].nil?
            custom_dir_file = File.join(ugii_root_dir, 'menus', 'custom_dirs.dat')

            if File.exist?(custom_dir_file) && !regexp?(custom_dir_file, /.*NxlSCFIntegration.*/)
                log_info('Edit custom_dirs.dat file')
                nxl_scf_integration = File.join(ENV['RMX_ROOT'], 'SCFIntegration', 'TC_BIN_LINUX', 'NxlTcImportIntegration')
                file = File.new(custom_dir_file, 'a+')
                file.puts "\n" unless file_endwith_new_line(custom_dir_file)
                file.puts '# nextlabs NxlSCFIntegration - start'
                file.puts nxl_scf_integration
                file.puts '# nextlabs NxlSCFIntegration - end'
                file.close
            end

            # Copy so files to TC_ROOT/lib
            tc_version = node['tc_version']
            so_file = File.join(ENV['RMX_ROOT'], 'SCFIntegration', 'TC_BIN_LINUX', "NxlSCFIntegration#{tc_version}.so")
            FileUtils.cp_r(so_file, File.join(node['TC_ROOT'], 'lib', 'NxlSCFIntegration.so'))

            # Add TRANSLATOR_HOME env
            translators_folder = File.join(node['DISP_ROOT'], 'Module', 'Translators') if File.exist?(node['DISP_ROOT'])

            nextlabs_env = '/etc/profile.d/nextlabs_env.sh'
            file = File.new(nextlabs_env, 'a+')
            file.puts "export TRANSLATOR_HOME=#{translators_folder}"
            file.close

            # Grant all rights to bczmodifier.sh
            grant_full_rights(File.join(ENV['RMX_ROOT'], 'shared_libs', 'bczmodifier.sh'))
        end


        # Attach SCF XLST hook
        log_info('Attach Nextlabs XSLT file for SCFIntegration module')
        attach_utility = File.join(node['TC_ROOT'], 'bin', TeamcenterProcess::XSLT_UTILITY)
        attach_utility = attach_utility.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        xsl_file = File.join(ENV['RMX_ROOT'], 'XSLT', 'remove_nxl.xslt')
        attach_command = "#{node['tc_profilevar']} && \"#{attach_utility}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -transfermode=TIEUnmanagedExportForNX_TM -xsl_file=\"#{xsl_file}\" -action=attach"
        call_system_command('Attach NextLabs SCF XSLT', 'plmxml_tm_edit_xsl', attach_command, :no)
    end
end
