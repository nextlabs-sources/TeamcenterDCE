# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_bb_integration
# Description   :: Recover __nx.jar file and original BB LaunchFile.bat
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_bb_integration' do
    block do
        log_info(LOGMSG::UNINSTALL_BB_INTEGRATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Restore nx jar...')
        modify_nx_jar(node['temp_dir'], node['bb_dir'], BBPlugin::NXLNXPOSTOPEN, BBPlugin::NXPOSTOPEN)

        # Only 1 jar inside but still find it, it does not have fixed name (base on version)
        jar_files = get_file_basenames_by_ext(File.join(ENV['RMX_ROOT'], 'BBIntegration', 'plugins'), '*.jar')
        jar_files.each do |jarname|
            if jarname.start_with?('com.nextlabs.bbextension')
                plugin_jar = jarname

                if File.exist?(File.join(node['bb_dir'], 'plugins', plugin_jar))
                    FileUtils.remove(File.join(node['bb_dir'], 'plugins', plugin_jar))
                end
            end
        end

        # Remove BBRMX.dll
        java_home = ENV['JAVA_HOME']
        jre_home = ENV['JRE_HOME']
        jre64_home = ENV['JRE64_HOME']
        if !jre64_home.nil?
            target_folder = File.join(jre64_home, 'bin')
        elsif !jre_home.nil?
            target_folder = File.join(jre_home, 'bin')
        elsif !java_home.nil?
            target_folder = File.join(java_home, 'bin')
        else
            raise('JAVA_HOME/JRE_HOME/JRE64_HOME environment variable are not set, abort')
        end
        log_info('Deleting BBRMX.dll from JAVA folder')
        filelist = %w(BBRMX.dll libeay32.dll)
        delete_files(target_folder, filelist)
    end
end
