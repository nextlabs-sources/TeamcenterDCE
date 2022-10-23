# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_dispatcher_client_extension
# Description   :: Removing tonxlfile translator module
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_disp_client_ext' do
    block do
        log_info(LOGMSG::UNINSTALL_DISPATCHER_CLIENT_EXTENSION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # Remove jars in DISP_ROOT/DispatcherClient/lib
        log_info('Remove DispatcherClient lib files...')
        disp_client_dir = File.join(node['DISP_ROOT'], 'DispatcherClient')
        lib_dir = File.join(node['DISP_ROOT'], 'DispatcherClient', 'lib')
        filenames = get_file_basenames(File.join(ENV['RMX_ROOT'], 'Dispatcher'))
        delete_files(lib_dir, filenames)

        # Remove openaz library folder
        openaz_lib_folder = File.join(disp_client_dir, 'lib', 'openaz')
        FileUtils.rm_rf(openaz_lib_folder)

        # Remove openaz classpath
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            openaz_classpath_file = File.join(disp_client_dir, 'bin', 'openaz_tscpappend.bat')
            FileUtils.remove_file(openaz_classpath_file, true)

            log_info('Remove openaz classpath from DispatcherClient classpath')
            set_dispclient_env = File.join(disp_client_dir, 'bin', 'setDispatcherClientEnv.bat')
            set_dispclient_env_temp = File.join(disp_client_dir, 'bin', 'setDispatcherClientEnv_temp.bat')
            if File.exist?(set_dispclient_env)
                File.open(set_dispclient_env_temp, 'w') do |out|
                    File.foreach(set_dispclient_env) do |line|
                        next if line =~ /%HOME%\\bin\\openaz_tscpappend.bat/
                        out.puts line
                    end
                end
                FileUtils.mv(set_dispclient_env_temp, set_dispclient_env)
            end
        else
            log_info('Remove openaz classpath from runDispatcherClient.sh (Linux)')
            run_dispclient = File.join(disp_client_dir, 'bin', 'runDispatcherClient.sh')
            run_dispclient_temp = File.join(disp_client_dir, 'bin', 'runDispatcherClient_temp.sh')
            if File.exist?(run_dispclient)
                File.open(run_dispclient_temp, 'w') do |out|
                    found_headline = false  # Found line contains openaz headline
                    num_lines = 4           # 4 more lines to remove after headline (include an empty line)
                    File.foreach(run_dispclient) do |line|
                        if line =~ /openaz/
                            found_headline = true
                            next
                        end

                        if found_headline && num_lines > 0
                            num_lines -= 1
                            next
                        end

                        out.puts line
                    end
                end
                FileUtils.mv(run_dispclient_temp, run_dispclient)
            end
        end

        # remove tonxlfile service from DispatcherClient Service.properties
        log_info('Remove tonxlfile service from DispatcherClient Service.properties...')
        filepath = File.join(node['DISP_ROOT'], 'DispatcherClient', 'conf', 'Service.properties')
        if File.exist?(filepath)
            regexp = /,tonxlfile/
            text = File.read(filepath)
            replace = text.gsub(regexp, '')
            File.open(filepath, 'w') { |file| file.puts replace }
        end
    end
end
