# Teamcenter RMX 5.3
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_rac_rmx
# Description   :: Remove RAC RMX component from TC
# Author        :: Phan Anh Tuan
# Copyright 2020, Nextlabs Inc.

require 'fileutils'
require 'open3'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_rac_rmx' do
    block do
        log_info(LOGMSG::UNINSTALL_RAC_RMX)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # STEP: Remove nextlabs folder from TC portal plugin folder
        racrmx_folder = File.join(ENV['RMX_ROOT'], 'RACRMX')
        log_info('Remove nextlabs folders from plugin folder')
        plugin_folder = File.join(node['TC_ROOT'], 'portal', 'plugins')
        FileUtils.remove_dir(File.join(plugin_folder, 'nextlabs'), true)

        # STEP: Restore jar files
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

        jar_files = get_file_basenames_by_ext(racrmx_tcversion, '*.jar')
        jar_files.each do |filename|
            file_location = File.join(plugin_folder, filename)
            recover(file_location, file_location + '.old')
        end

        # STEP: Remove dll files from JRE_HOME/bin
        log_info('Remove nextlabs dlls from TC_JRE_HOME')

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
            FileUtils.remove_file(File.join(jre_bin, 'libeay32.dll'), true)
            FileUtils.remove_file(File.join(jre_bin, 'RACRMX.dll'), true)
            FileUtils.remove_file(File.join(jre_bin, 'viewwatermark64.dll'), true)
        else
            log_info('TC_JRE_HOME not found or not exist. Using default JRE_HOME or JRE64_HOME')
            jre_home = ENV['JRE_HOME']
            if ENV['JRE_HOME'].nil?
                jre_home = ENV['JRE64_HOME']
            end
            jre_bin = File.join(jre_home, 'bin')
            FileUtils.remove_file(File.join(jre_bin, 'libeay32.dll'), true)
            FileUtils.remove_file(File.join(jre_bin, 'RACRMX.dll'), true)
            FileUtils.remove_file(File.join(jre_bin, 'viewwatermark64.dll'), true) 
        end

        # STEP: Restore portal.bat
        log_info('Restore portal.bat')
        portal_bat = File.join(node['TC_ROOT'], 'portal', 'portal.bat')
        recover(portal_bat, portal_bat + '.old')

        # STEP: Call genregxml.bat
        log_info('Regenerate regxml')
        genregxml = File.join(node['TC_ROOT'], 'portal', 'registry', Script::GENREGXML)
        genregxml_cmd = "\"#{genregxml}\""
        call_system_command('Unregister Nextlabs RAC RMX plugin', 'genregxml', genregxml_cmd)
    end
end
