# Teamcenter RMX 5.3
# Cookbook Name :: RMX_installation
# Recipes       :: install_rac_rmx
# Description   :: Plug in RAC RMX into Teamcenter RAC
# Author        :: Phan Anh Tuan
# Copyright 2020, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_rac_rmx' do
    block do
        log_info(LOGMSG::INSTALL_RAC_RMX)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # Check jar compatible version of RAC RMX
        racrmx_folder = File.join(ENV['RMX_ROOT'], 'RACRMX')
        if node['tc_version'] == '11'
            racrmx_tcversion = File.join(racrmx_folder, 'tc11')
        elsif node['tc_full_version'].start_with?('123')
            racrmx_tcversion = File.join(racrmx_folder, 'tc12.3')
        elsif node['tc_full_version'].start_with?('124')
            racrmx_tcversion = File.join(racrmx_folder, 'tc12.4')
        elsif node['tc_full_version'].start_with?('12')
            racrmx_tcversion = File.join(racrmx_folder, 'tc12')
        elsif node['tc_full_version'].start_with?('131')
            racrmx_tcversion = File.join(racrmx_folder, 'tc13.1')
        elsif node['tc_full_version'].start_with?('132')
            racrmx_tcversion = File.join(racrmx_folder, 'tc13.2')
        elsif node['tc_full_version'].start_with?('133')
            racrmx_tcversion = File.join(racrmx_folder, 'tc13.3')
        else
            racrmx_tcversion = File.join(racrmx_folder, 'tc13')
        end

        jar_exist = true
        plugin_folder = File.join(node['TC_ROOT'], 'portal', 'plugins')
        jar_files = get_file_basenames_by_ext(racrmx_tcversion, '*.jar')
        jar_files.each do |filename|
            file_location = File.join(plugin_folder, filename)
            jar_exist = jar_exist && File.exist?(file_location)
        end

        if jar_exist
            log_info('Detected compatible version for RAC RMX installation.')
            # STEP: Copy dll file into TC_JRE_HOME/bin
            log_info('Add Nextlabs dlls into TC_JRE_HOME')

            # Find TC_JRE_HOME in tem_init.bat
            tem_init = File.join(node['TC_ROOT'], 'install', 'tem_init.bat')
            tc_jre_home = ''
            if File.exist?(tem_init)
                File.foreach(tem_init) do |line|
                    if line.include?('TC_JRE_HOME')
                        tc_jre_home = line.strip!.split('TC_JRE_HOME=')[1]
                        break
                    end
                end
            end

            if Dir.exist?(tc_jre_home)
                jre_bin = File.join(tc_jre_home, 'bin')
                FileUtils.cp_r(File.join(racrmx_folder, 'libeay32.dll'), jre_bin)
                FileUtils.cp_r(File.join(racrmx_folder, 'RACRMX.dll'), jre_bin)
                FileUtils.cp_r(File.join(racrmx_folder, 'viewwatermark64.dll'), jre_bin)
            else
                log_info('TC_JRE_HOME not found or not exist. Using default JRE_HOME or JRE64_HOME')
                jre_home = ENV['JRE_HOME']
                if ENV['JRE_HOME'].nil?
                    jre_home = ENV['JRE64_HOME']
                end
                jre_bin = File.join(jre_home, 'bin')
                FileUtils.cp_r(File.join(racrmx_folder, 'libeay32.dll'), jre_bin)
                FileUtils.cp_r(File.join(racrmx_folder, 'RACRMX.dll'), jre_bin)
                FileUtils.cp_r(File.join(racrmx_folder, 'viewwatermark64.dll'), jre_bin)  
            end  

            # STEP: Replace jar files in TC portal plugin folder
            log_info('Replace jar files in TC portal plugin folder')
            nextlabs_racrmx = File.join(racrmx_tcversion, 'nextlabs')
            plugin_folder = File.join(node['TC_ROOT'], 'portal', 'plugins')

            FileUtils.cp_r(nextlabs_racrmx, plugin_folder)

            # jar_files = get_file_basenames_by_ext(racrmx_tcversion, '*.jar')
            jar_files.each do |filename|
                file_location = File.join(plugin_folder, filename)
                if File.exist?(file_location)
                    backup_old_file(file_location, plugin_folder)
                    FileUtils.cp_r(File.join(racrmx_tcversion, filename), plugin_folder)
                end
            end

            # STEP: Modify TC portal.bat file (base on TC version)
            log_info('Change portal.bat to include nextlabs jar file')
            portal_bat = File.join(node['TC_ROOT'], 'portal', 'portal.bat')
            portal_bat_tmp = File.join(node['TC_ROOT'], 'portal', 'portal_tmp.bat')

            if File.exist?(portal_bat)
                backup_old_file(portal_bat, File.dirname(portal_bat))
                start_command = 'start Teamcenter.exe '

                file = File.new(portal_bat_tmp, 'w+')

                File.foreach(portal_bat) do |line|
                    if line.start_with?(start_command)
                        # If found the start command, add few lines follow as classpath
                        file.puts "set NXLCP=\"#{node['TC_ROOT']}\\portal\\plugins\\nextlabs\\aspectjrt.jar\""
                        file.puts "set NXLCP=%NXLCP%;\"#{node['TC_ROOT']}\\portal\\plugins\\nextlabs\\aspectjweaver.jar\""

                        if node['tc_version'] == '11'
                            separator = "\"%JRE_HOME%\\lib\\javaws.jar\""
                            between_classpath = line.split(separator)
                            new_line = between_classpath[0] + separator + ";%NXLCP%" + between_classpath[1]
                            file.puts new_line
                        else
                            separator = "-Xmx%VM_XMX%"
                            between_classpath = line.split(separator)
                            new_line = between_classpath[0] + separator + " -Xbootclasspath/a:%NXLCP%" + between_classpath[1]
                            file.puts new_line
                        end
                    else
                        file.puts line
                    end
                end
                file.close
                FileUtils.cp_r(portal_bat_tmp, portal_bat)
            end 
        else
            log_info('Unable to find compatible jar file version. RAC RMX installation will be abort. Please check version of Teamcenter RAC supported')
        end

        # STEP: Call genregxml.bat. It activate also the User Initial Protection Feature (Should run this command whether RACRMX deploy success or not)
        log_info('Regenerate regxml')
        genregxml = File.join(node['TC_ROOT'], 'portal', 'registry', Script::GENREGXML)
        genregxml_cmd = "\"#{genregxml}\""
        call_system_command('Register Nextlabs RAC RMX plugin', 'genregxml', genregxml_cmd)
    end
end
