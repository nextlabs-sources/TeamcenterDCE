# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_dispatcher_module
# Description   :: Adding tonxlfile translator module to Dispatcher Module
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_dispatcher_module' do
    block do
        log_info(LOGMSG::INSTALL_DISPATCHER_MODULE)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        disp_module_dir = File.join(node['DISP_ROOT'], 'Module')
        if File.exist?(disp_module_dir)
            tonxlfile_dir = File.join(disp_module_dir, 'Translators', 'tonxlfile')
            log_info("Make #{tonxlfile_dir} directory")
            FileUtils.mkdir_p tonxlfile_dir
            tonxlfile_bin_dir = File.join(tonxlfile_dir, 'bin')
            FileUtils.mkdir_p tonxlfile_bin_dir
            src_dir = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin')
            copy_files(src_dir, tonxlfile_bin_dir)

            log_info('Overwrite tonxlfile SkyDRM configuration...')
            path = File.join(tonxlfile_bin_dir, 'config.properties')
            file = File.new(path, 'w+')
            file.puts 'ROUTER_URL=' + node['skydrm_router_url']
            # file.puts 'TENANT_NAME=' + node['skydrm_tenant_name']
            file.puts 'APP_ID=' + node['skydrm_app_id']
            file.puts 'APP_KEY=' + node['skydrm_app_key'] # or ENV['skydrm_app_key']
            file.close

            log_info('Modifying translator.xml...')
            filepath_translator = File.join(disp_module_dir, 'conf', 'translator.xml')
            if File.exist?(filepath_translator) && !regexp?(filepath_translator, /.*tonxlfile.*/)
                file = ::File.new(filepath_translator)
                doc = REXML::Document.new(file)
                option_array = Array.new(15)
                (0...15).each do |i|
                    option_array[i] = REXML::Element.new 'Option'
                    option_array[i].add_attribute('name', 'clientoption')
                    option_array[i].add_attribute('string', '')
                end
                option_array[0].add_attribute('optionkey', 'INPUTPATH')
                option_array[1].add_attribute('optionkey', 'INPUTFILENAME')
                option_array[2].add_attribute('optionkey', 'OUTPUTPATH')
                option_array[3].add_attribute('optionkey', 'NXLACTION')
                option_array[4].add_attribute('optionkey', 'INPUTARGSFILENAME')
                option_array[0].add_attribute('description', 'Full path to the input file.')
                option_array[1].add_attribute('description', 'File name of the input file.')
                option_array[2].add_attribute('description', 'Full path to the output file.')
                option_array[3].add_attribute('description', 'Translation action, protect or updateTag.')
                option_array[4].add_attribute('description', 'File name of the args file.')
                (5...15).each do |i|
                    option_array[i].add_attribute('optionkey', 'TAG' + (i - 4).to_s)
                    option_array[i].add_attribute('description', 'TAG' + (i - 4).to_s)
                end

                ele_options = REXML::Element.new 'Options'
                (0...15).each do |i|
                    ele_options.add_element option_array[i]
                end
                ele_trans_executable = REXML::Element.new 'TransExecutable'
                ele_trans_executable.add_attribute('dir', '&MODULEBASE;/Translators/tonxlfile/bin')
                ele_trans_executable.add_attribute('name', Script::RMX_TRANSLATOR)
                ele_trans_executable.add_element ele_options

                ele_tonxlfile = REXML::Element.new 'ToNxlFile'
                ele_tonxlfile.add_attribute('provider', 'NEXTLABS')
                ele_tonxlfile.add_attribute('service', 'tonxlfile')
                ele_tonxlfile.add_attribute('isactive', 'true')
                ele_tonxlfile.add_element ele_trans_executable

                doc.elements['Translators'].add_element ele_tonxlfile
                file = File.new(filepath_translator, 'w')
                doc.write(file, 4)
                file.close

                # bug fix 48584: & will become &amp; in REXML
                lines = IO.readlines(filepath_translator).map do |line|
                    if line.include?('Translators/tonxlfile/bin')
                        "        <TransExecutable dir='&MODULEBASE;/Translators/tonxlfile/bin' name='#{Script::RMX_TRANSLATOR}'>"
                    else
                        line
                    end
                end
                File.open(filepath_translator, 'w') do |f|
                    f.puts lines
                end
            end

            log_info('Modifying AdminClientUI...')
            filepath = File.join(node['DISP_ROOT'], 'AdminClient', 'ui', 'swing', 'AdminClientUI.xml')
            if File.exist?(filepath) && !regexp?(filepath, /.*tonxlfile.*/)
                file = File.new(filepath)
                doc = REXML::Document.new(file)

                ele_file_filter = REXML::Element.new 'FileFilter'
                ele_file_filter.add_attribute('id', '*')
                ele_file_filter.add_attribute('string', '*')
                ele_file_filter.add_attribute('desc', 'Binary and text files')

                ele_service = REXML::Element.new 'Service'
                ele_service.add_attribute('string', 'tonxlfile')
                ele_service.add_attribute('ext', '.nxl')
                ele_service.add_element ele_file_filter

                ele_provider = REXML::Element.new 'Provider'
                ele_provider.add_attribute('name', 'NEXTLABS')
                ele_provider.add_attribute('desc', 'NEXTLABSServices')
                ele_provider.add_element ele_service

                doc.elements['AdminClient/ProviderMap'].add_element ele_provider
                file = File.new(filepath, 'w')
                doc.write(file, 4)
                file.close
            end

            if RUBY_PLATFORM =~ /linux/
                # all the directories created should have full rights in linux server
                tonxlfile_dir = File.join(node['DISP_ROOT'], 'Module', 'Translators', 'tonxlfile')
                grant_full_rights(tonxlfile_dir)
            end
        else
            log_info("Cannot find #{disp_module_dir}, skip deploying dispatcher module extension.")
        end
    end
end
