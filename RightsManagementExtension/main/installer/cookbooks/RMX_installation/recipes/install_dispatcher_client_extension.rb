# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_dispatcher_client_extension
# Description   :: Adding tonxlfile translator module, modify Service.properties file
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_disp_client_extension' do
    block do
        log_info(LOGMSG::INSTALL_DISPATCHER_CLIENT_EXTENSION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        disp_client_dir = File.join(node['DISP_ROOT'], 'DispatcherClient')
        if File.exist?(disp_client_dir)
            log_info('Copy jar files from RMX_ROOT/Dispatcher to DISP_ROOT/DispatcherClient/lib')
            source_folder = File.join(ENV['RMX_ROOT'], 'Dispatcher')
            dest_folder = File.join(disp_client_dir, 'lib')
            copy_files(source_folder, dest_folder)

            # Addtion nextlabs openaz library will be put into lib/openaz folder (Support CC 9.1)
            log_info('Copy nextlabs openaz folder into DISP_ROOT/DispatcherClient/lib/openaz')
            openaz_source_folder = File.join(ENV['RMX_ROOT'], 'Dispatcher', 'openaz')
            openaz_dest_folder = File.join(disp_client_dir, 'lib', 'openaz')
            FileUtils.mkdir_p(openaz_dest_folder)
            copy_files(openaz_source_folder, openaz_dest_folder)

            # Manage addition classpath from openaz folder
            if RUBY_PLATFORM =~ /mswin|mingw|windows/
                # Create openaz_tscpappend.bat
                openaz_classpath_file = File.new(File.join(disp_client_dir, 'bin', 'openaz_tscpappend.bat'), 'w+')
                openaz_classpath_file.puts('set classpath=%classpath%;..\\lib\\openaz\\%~nx1')
                openaz_classpath_file.close

                # Change setDispatcherClientEnv to include openaz classpath file
                log_info('Include openaz classpath into DispatcherClient classpath')
                set_dispclient_env = File.join(disp_client_dir, 'bin', 'setDispatcherClientEnv.bat')
                set_dispclient_env_temp = File.join(disp_client_dir, 'bin', 'setDispatcherClientEnv_temp.bat')
                if File.exist?(set_dispclient_env)
                    backup_old_file(set_dispclient_env, File.dirname(set_dispclient_env))
                    File.open(set_dispclient_env_temp, 'w') do |out|
                        File.foreach(set_dispclient_env) do |line|
                            out.puts line
                            if line =~ /%HOME%\\bin\\tscpappend.bat/
                                out.puts 'for %%i in (%HOME%\\lib\\openaz\\*.jar) do call %HOME%\\bin\\openaz_tscpappend.bat %%i'
                            end
                        end
                    end
                    FileUtils.mv(set_dispclient_env_temp, set_dispclient_env)
                end
            else
                # On LINUX platform
                log_info('Include openaz classpath into runDispatcherClient.sh (Linux)')
                run_dispclient = File.join(disp_client_dir, 'bin', 'runDispatcherClient.sh')
                run_dispclient_temp = File.join(disp_client_dir, 'bin', 'runDispatcherClient_temp.sh')
                if File.exist?(run_dispclient)
                    backup_old_file(run_dispclient, File.dirname(run_dispclient))
                    File.open(run_dispclient_temp, 'w') do |out|
                        File.foreach(run_dispclient) do |line|
                            if line =~ /fccclient.jar/      # Put nextlabs openaz classpath before the line contains this string
                                out.puts 'for i in $HOME/lib/openaz/*.jar'
                                out.puts 'do'
                                out.puts '   export CLASSPATH=$CLASSPATH:$i'
                                out.puts 'done'
                                out.puts ''
                            end
                            out.puts line
                        end
                    end
                    FileUtils.mv(run_dispclient_temp, run_dispclient)
                end
                grant_full_rights(run_dispclient)
            end

            log_info('Modifying Service.properties...')
            filepath = File.join(disp_client_dir, 'conf', 'Service.properties')
            # Backup Service.properties file
            backup_old_file(filepath, File.dirname(filepath))
            if File.exist?(filepath) && !regexp?(filepath, /.*tonxlfile.*/)
                regexp = /import.*TSBasic.*\n/
                new_line = ''
                File.foreach(filepath) do |line|
                    if line =~ regexp
                        new_line = line.chop + ",tonxlfile\n"
                        break
                    end
                end
                text = File.read(filepath)
                replace = text.gsub(regexp, new_line)
                File.open(filepath, 'w') { |file| file.puts replace }
            elsif !File.exist?(filepath)
                log_info("#{filepath} does not exist, modify failed.")
            end
        else
            log_info("Cannot find #{disp_client_dir}, skip deploying dispatcher client extension.")
        end
    end
end
