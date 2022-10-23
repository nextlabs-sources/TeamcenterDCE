# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_partialdownload
# Description   :: Remove nextlabs third_party jar file in tccs folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_partialdownload' do
    block do
        log_info(LOGMSG::UNINSTALL_PARTIALDOWNLOAD)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Killing any existing FCC process running')
        fcc_kill = File.join(node['TC_ROOT'], 'tccs', 'bin', 'fccstat')
        fcc_kill = fcc_kill.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_kill_cmd = "\"#{fcc_kill}\" -kill | exit 0"
        call_system_command('Kill FCC process', 'fccstat', fcc_kill_cmd)

        # recover fcc.xml
        fcc_xml = File.join(node['TC_ROOT'], 'tccs', 'fcc.xml')
        delete_xml_element(fcc_xml, 2, "/fccconfig/fccdefaults/property[@name='FCC_DecryptingInputStreamClass']")

        # remove nextlabs integration & recover tccs_classpath.bat
        log_info('Recover tccs_classpath.bat file')
        nxl_dir = File.join(node['TC_ROOT'], 'tccs', 'third_party', 'nxl')
        FileUtils.remove_dir(nxl_dir, true)
        tccs_classpath = File.join(node['TC_ROOT'], 'tccs', Script::TCCS_CLASSPATH)
        delete_file_content_with_label(tccs_classpath, 'nextlabs partialdownload integration')

        if check_vis_server_installed(node['TC_ROOT'])
            FileUtils.remove_file(File.join(node['TC_ROOT'], 'tccs', 'log4j2.properties'), true)
            # Only remove with FMS PartialDownload Server
            log_info('Remove RMJavaSDK flags from starttccs.bat file')
            starttccs_file = File.join(node['TC_ROOT'], 'tccs', 'starttccs.bat')
            if File.exist?(starttccs_file) && File.read(starttccs_file).include?('-Dsdk.cache.token=true -Dsdk.cache.config="%FMS_HOME%/third_party/nxl/sdk.properties" ')
                line_will_change = ''
                start_part = 'set TCCS_CMD="%TCCS_JAVA%\\bin\\java" '
                new_line = ''
                File.foreach(starttccs_file) do |line|
                    if line.start_with?(start_part)
                        line_will_change = line
                        new_line = line.gsub('-Dsdk.cache.token=true -Dsdk.cache.config="%FMS_HOME%/third_party/nxl/sdk.properties" ', '')
                        break
                    end
                end

                contents = File.read(starttccs_file)
                replaced_content = contents.gsub(line_will_change, new_line)
                File.open(starttccs_file, 'w') { |file| file.puts replaced_content }
            end
        else
            log_info('Remove FMSRMX from tccs/lib folder')
            FileUtils.remove_file(File.join(node['TC_ROOT'], 'tccs', 'lib', 'FMSRMX.dll')) if File.exist?(File.join(node['TC_ROOT'], 'tccs', 'lib', 'FMSRMX.dll'))
            FileUtils.remove_file(File.join(node['TC_ROOT'], 'tccs', 'lib', 'libeay32.dll')) if File.exist?(File.join(node['TC_ROOT'], 'tccs', 'lib', 'libeay32.dll'))
        end
        

        log_info('Start FCC process')
        fcc_start = File.join(node['TC_ROOT'], 'tccs', 'bin', 'fccstat')
        fcc_start = fcc_start.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_start_cmd = "\"#{fcc_start}\" -start | exit 0"
        call_system_command('Start FCC process', 'fccstat', fcc_start_cmd)
    end
end
