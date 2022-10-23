# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_dispatcher_client_extension
# Description   :: Replace tonxlfile translator module
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_disp_client_extension' do
    block do
        log_info(LOGMSG::UPGRADE_DISPATCHER_CLIENT_EXTENSION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        disp_client_dir = File.join(node['DISP_ROOT'], 'DispatcherClient')
        if File.exist?(disp_client_dir)
            log_info('Remove unecessary library from old RMX version')
            library_to_remove = %w(nlJavaSDK2.jar commons-logging-1.1.1.jar)        # This library stay inside DC/lib folder but not needed for RMX 5.2
            lib_dir = File.join(disp_client_dir, 'lib')
            delete_files(lib_dir, library_to_remove)

            log_info('Replace jar files from RMX_ROOT/Dispatcher to DISP_ROOT/DispatcherClient/lib')
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
            end
        end
    end
end
