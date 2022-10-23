# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_partialdownload
# Description   :: Move nextlabs third_party jar file to tccs folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_partialdownload' do
    block do
        log_info(LOGMSG::INSTALL_PARTIALDOWNLOAD)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Killing any existing FCC process running for PartialDownload deployment')
        fcc_kill = File.join(node['TC_ROOT'], 'tccs', 'bin', 'fccstat')
        fcc_kill = fcc_kill.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_kill_cmd = "\"#{fcc_kill}\" -kill | exit 0"
        call_system_command('Kill FCC process', 'fccstat', fcc_kill_cmd)

        nxl_dir = File.join(node['TC_ROOT'], 'tccs', 'third_party', 'nxl')
        fccxml = File.join(node['TC_ROOT'], 'tccs', 'fcc.xml')
        if check_vis_server_installed(node['TC_ROOT'])
            log_info('Deploy FMS PartialDownload Server solution')
            src_dir = File.join(ENV['RMX_ROOT'], 'PartialDownload')
            copy_files(src_dir, nxl_dir)
            FileUtils.mv(File.join(nxl_dir, 'log4j2.properties'), File.join(node['TC_ROOT'], 'tccs', 'log4j2.properties'))

            edit_fccxml(fccxml, 'server')

            starttccs_file = File.join(node['TC_ROOT'], 'tccs', 'starttccs.bat')
            log_info('Modify starttccs.bat file for new flags of RMJavaSDK')
            if File.exist?(starttccs_file) && !File.read(starttccs_file).include?('-Dsdk.cache.token=true -Dsdk.cache.config="%FMS_HOME%/third_party/nxl/sdk.properties" ')
                backup_old_file(starttccs_file, File.dirname(starttccs_file))
                line_will_change = ''
                before_flag = 'set TCCS_CMD="%TCCS_JAVA%\\bin\\java" '
                new_line = ''
                File.foreach(starttccs_file) do |line|
                    if line.start_with?(before_flag)
                        line_will_change = line
                        after_flag = line.gsub(before_flag, '')
                        new_line = before_flag
                        new_line += '-Dsdk.cache.token=true -Dsdk.cache.config="%FMS_HOME%/third_party/nxl/sdk.properties" '
                        new_line += after_flag
                        break
                    end
                end

                contents = File.read(starttccs_file)
                replaced_content = contents.gsub(line_will_change, new_line)
                File.open(starttccs_file, 'w') { |f| f.puts replaced_content }
            end
        else
            log_info('Deploy PartialDownload Client solution')
            src_dir = File.join(ENV['RMX_ROOT'], 'PartialDownloadClient')
            copy_files(src_dir, nxl_dir)

            edit_fccxml(fccxml, 'client')

            log_info('Move FMSRMX to tccs/lib folder')
            FileUtils.mv(File.join(nxl_dir, 'FMSRMX.dll'), File.join(node['TC_ROOT'], 'tccs', 'lib', 'FMSRMX.dll'))
            FileUtils.mv(File.join(nxl_dir, 'libeay32.dll'), File.join(node['TC_ROOT'], 'tccs', 'lib', 'libeay32.dll'))
        end

        # add nxl jars in tccs_classpath
        tccs_classpath = File.join(node['TC_ROOT'], 'tccs', Script::TCCS_CLASSPATH)
        jar_basenames = get_file_basenames_by_ext(nxl_dir, '*.jar')

        log_info('Modify tccs_classpath.bat file for NextLabs third party library')
        backup_old_file(tccs_classpath, File.dirname(tccs_classpath))
        if File.exist?(tccs_classpath) && !regexp?(tccs_classpath, /.*nextlabs partialdownload integration.*/)
            file = File.new(tccs_classpath, 'a+')
            file.puts "\n" unless file_endwith_new_line(tccs_classpath)
            file.puts 'rem nextlabs partialdownload integration - start'
            jar_basenames.each do |jar_name|
                file.puts "set TCCS_CP=%TCCS_CP%;%FMS_HOME%\\third_party\\nxl\\#{jar_name}"
            end
            file.puts 'rem nextlabs partialdownload integration - end'
            file.close
        end

        log_info('Start FCC process')
        fcc_start = File.join(node['TC_ROOT'], 'tccs', 'bin', 'fccstat')
        fcc_start = fcc_start.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_start_cmd = "\"#{fcc_start}\" -start | exit 0"
        call_system_command('Start FCC process', 'fccstat', fcc_start_cmd)
    end

    def edit_fccxml(fccxml, component)
        nextlabs_plugins = "com.nextlabs.teamcenter.fms.decrypt.segment.#{component}.NextLabsDecryptingInputStream"
        if File.exist?(fccxml) && !regexp?(fccxml, /.*NextLabsDecryptingInputStream.*/)
            backup_old_file(fccxml, File.dirname(fccxml))
            file = ::File.new(fccxml)
            doc = REXML::Document.new(file)
            ele = REXML::Element.new 'property'
            ele.add_attribute('name', 'FCC_DecryptingInputStreamClass')
            ele.add_attribute('value', nextlabs_plugins)
            ele.add_attribute('overridable', 'true')
            doc.elements['fccconfig/fccdefaults'].add_element ele
            file = File.new(fccxml, 'w')
            doc.write(file, 2)
            file.close
        end
    end
end
