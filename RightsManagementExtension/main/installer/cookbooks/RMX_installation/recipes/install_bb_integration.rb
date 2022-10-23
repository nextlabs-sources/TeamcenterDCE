# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_bb_integration
# Description   :: Change __nx.jar file and cutomize BB LaunchFile.bat
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_bb_integration' do
    block do
        log_info(LOGMSG::INSTALL_BB_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Editing nx jar')
        modify_nx_jar(node['temp_dir'], node['bb_dir'], BBPlugin::NXPOSTOPEN, BBPlugin::NXLNXPOSTOPEN)

        # Only 1 jar inside but still find it, it does not have fixed name (base on version)
        jar_files = get_file_basenames_by_ext(File.join(ENV['RMX_ROOT'], 'BBIntegration', 'plugins'), '*.jar')
        jar_files.each do |jarname|
            if jarname.start_with?('com.nextlabs.bbextension')
                plugin_jar = File.join(ENV['RMX_ROOT'], 'BBIntegration', 'plugins', jarname)

                FileUtils.cp_r(plugin_jar, File.join(node['bb_dir'], 'plugins'))
                break
            end
        end

        # Move BBRMX.dll to %J**_HOME%\bin
        java_home = ENV['JAVA_HOME']
        jre_home = ENV['JRE_HOME']
        jre64_home = ENV['JRE64_HOME']
        if !jre64_home.nil?
            target_folder = File.join(jre64_home, 'bin')
            log_info('Deploying RMX to JRE64_HOME\\bin')
        elsif !jre_home.nil?
            target_folder = File.join(jre_home, 'bin')
            log_info('Deploying RMX to JRE_HOME\\bin')
        elsif !java_home.nil?
            target_folder = File.join(java_home, 'bin')
            log_info('Deploying RMX to JAVA_HOME\\bin')
        else
            raise('JAVA_HOME/JRE_HOME/JRE64_HOME environment variable are not set, abort')
        end
        
        bbrmx_dll = File.join(ENV['RMX_ROOT'], 'BBIntegration', 'bin', 'BBRMX.dll')
        FileUtils.cp_r(bbrmx_dll, target_folder)
        libeay_dll = File.join(ENV['RMX_ROOT'], 'BBIntegration', 'bin', 'libeay32.dll')
        unless (File.exist?(File.join(target_folder, 'libeay32.dll')))
            copy_files(File.join(ENV['RMX_ROOT'], 'BBIntegration', 'bin'), target_folder)
        end
    end
end
