# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Library       :: Converge Utility Library
# Description   :: Common utility function used by recipes (CHEF CONVERGE RUN TIME)
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

# General Utility: Use by all recipes

# Reference: https://stackoverflow.com/questions/50713867/ruby-how-to-read-registry-keys-for-32-bit-and-64-bit-applications-from-32-bit
if RUBY_PLATFORM =~ /mswin|mingw|windows/
    module Win32::Registry::Constants
        KEY_WOW64_64KEY = 0x0100
        KEY_WOW64_32KEY = 0x0200
    end
end

module Utility
    require 'fileutils'
    require 'win32/service' if RUBY_PLATFORM =~ /mswin|mingw|windows/
    require 'rexml/document'
    require 'json'
    require 'zip'
    require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/

    # copy files from src_dir to des_dir
    def copy_files(src_dir, des_dir)
        FileUtils.mkdir_p(des_dir) unless File.exist?(des_dir)
        files = File.join(src_dir, '*')
        Dir[files].each do |file|
            FileUtils.cp_r(file, des_dir)
        end
    end

    # copy files with filter, e.g., *.jar, *.txt
    def copy_files_filter(src_dir, des_dir, filter)
        FileUtils.mkdir_p(des_dir) unless File.exist?(des_dir)
        files = File.join(src_dir, filter)
        Dir[files].each do |file|
            FileUtils.cp_r(file, des_dir)
        end
    end

    # get all the basenames of files inside a directory
    def get_file_basenames(dir)
        filenames = []
        files = File.join(dir, '*')
        Dir[files].each do |file|
            name = File.basename(file)
            filenames.push(name)
        end
        filenames
    end

    # get all the basenames of files inside a directory by extension
    # ext: *.jar
    def get_file_basenames_by_ext(dir, ext)
        filenames = []
        files = File.join(dir, ext)
        Dir[files].each do |file|
            name = File.basename(file)
            filenames.push(name)
        end
        filenames
    end

    # delete files contained in filenamses inside a directory
    # filenames: file basenames (not full path)
    def delete_files(dir, filenames)
        filenames.each do |file|
            FileUtils.remove_file(File.join(dir, file), true)
        end
    end

    # New backup function. Automatically handle if .old file is exist
    def backup_old_file(old, des)
        log_info("Backup following file : #{old}")
        unless File.exist?(old)
            log_info("Failed to backup. #{old} not exist.")
            raise("#{old} not exist, backup failed!")
        end

        oldname = File.basename(old)
        newname = oldname + '.old'

        # Already backup once. Only change original file to .old once to avoid loss this file forever
        if File.exist?(File.join(des, newname))
            log_info("Skip backup step. Backup file #{newname} is already exist")
            return
        end

        FileUtils.cp(File.join(des, oldname), File.join(des, newname))
    end

    # Recover orginal file
    def recover(current_file, backup_file)
        return unless File.exist?(current_file) && File.exist?(backup_file)

        FileUtils.remove_file(current_file, true)
        File.rename(backup_file, current_file)
    end

    # Stop a running window process
    def stop_win_process(process_name)
        result = ''

        Open3.popen3("tasklist | find \"#{process_name}\"") do |_stdrin, stdout, _stderr|
            result = stdout.read
        end
        if result.include?(process_name)
            puts "Process #{process_name} is running, which will be terminated for RMX installation."
            system("taskkill /f /im #{process_name} /t")
        end
        t1 = Time.now
        loop do
            Open3.popen3("tasklist | find \"#{process_name}\"") do |_stdrin, stdout, _stderr|
                result = stdout.read
            end
            break unless result.include?(process_name)

            t2 = Time.now
            length = (t2 - t1) * 1000.0
            raise("Kill process #{process_name} take too long (40 s), please retry (un)installation.") if length > 40_000
        end
    end

    # Stop a running linux process
    def stop_linux_process(process_name)
        t1 = Time.now
        system("killall -w #{process_name}")
        t2 = Time.now
        length = (t2 - t1) * 1000.0
        raise("Kill process #{process_name} take too long (40 s), please retry (un)installation.") if length > 40_000
    end

    # Stop a running process
    def stop_running_process(process_name)
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            stop_win_process(process_name)
        elsif RUBY_PLATFORM =~ /linux/
            stop_linux_process(process_name)
        end
    end

    # Stop a running window service
    def stop_window_service(service_name)
        if ::Win32::Service.exists?(service_name)
            t1 = Time.now
            system("sc stop \"#{service_name}\"") if ::Win32::Service.status(service_name)['current_state'] == 'running'
            loop do
                break if ::Win32::Service.status(service_name)['current_state'] == 'stopped'

                t2 = Time.now
                length = (t2 - t1) * 1000.0
                raise("Stop #{service_name} take too long (40 s), please retry (un)installation.") if length > 40_000
            end
        end
    end

    # Start a window service
    def start_window_service(service_name)
        if ::Win32::Service.exists?(service_name)
            system("sc start \"#{service_name}\"") if ::Win32::Service.status(service_name)['current_state'] == 'stopped'
            # wait for 1 sec to start the service
            sleep(1)

            t1 = Time.now
            loop do
                break if Win32::Service.status(service_name)['current_state'] == 'running'

                t2 = Time.now
                length = (t2 - t1) * 1000
                if length > 20_000
                    log_info("WARNING: Start #{service_name} took too long (20 s), please manually start the service after (un)installation")
                    break
                end
            end
        end
    end

    # Stop a running linux Teamcenter service
    def stop_linux_tc_service(service_command)
        result = ""
        Open3.popen3("#\"{service_command}\" status") do |stdrin, stdout, stderr|
            result = stdout.read
        end
        if result.include?("is running")
            re = system("#\"{service_command}\" stop")
            if re == nil || false
                raise "Teamcenter service stop failed: #{service_command}"
            end
            sleep(1)
        end
    end

    # Start a linux Teamcenter service
    def start_linux_tc_service(service_command)
        result = ""
        Open3.popen3("#\"{service_command}\" status") do |stdrin, stdout, stderr|
            result = stdout.read
        end
        if result.include?("is not running")
            re = system("#\"{service_command}\" start")
            if re == nil || false
                raise "Teamcenter service start failed: #{service_command}"
            end
            sleep(1)
        end
    end

    # Start system service for Window and Linux platform
    def start_system_service(service_command)
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            start_window_service(service_command)
        elsif RUBY_PLATFORM =~ /linux/
            # only teamcenter pool_manager service and fsc service are supported
            start_linux_tc_service(service_command)
        end
    end

    # Stop system service for Window and Linux platform
    def stop_system_service(service_command)
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            stop_window_service(service_command)
        elsif RUBY_PLATFORM =~ /linux/
            # only teamcenter pool_manager service and fsc service are supported
            stop_linux_tc_service(service_command)
        end
    end

    # Get Active Workspace Client version. Return first decimal like 3.*, 4.0, 4.1,...
    def get_aws_version(tc_root)
        media_file = File.join(tc_root, 'media_teamcenter_activeworkspace.xml')
        xml_media_file = REXML::Document.new(File.new(media_file))
        awc_version_with_dot = xml_media_file.elements.to_a('media/version')[0].text

        version = awc_version_with_dot[0..2]
        version.to_f
    end

    # awc_update_kit_json function: Improve AWS recipe
    # Active Workspace Support only: Update kit.json file to include NextLabs module (CmdDRM and NXLOverlay)
    def awc_update_kit_json(kit_json, mode, module_name)
        kit_hash = JSON.parse(File.read(kit_json))
        # Check if module_name is in the list?
        module_exist = kit_hash['modules'].include?(module_name)
        if mode == 'install' && !module_exist
            kit_hash['modules'].push(module_name)
        elsif mode == 'uninstall' && module_exist
            kit_hash['modules'].delete(module_name)
        end
        File.open(kit_json, 'w') do |f|
            f.write(JSON.pretty_generate(kit_hash))
        end
    end

    # Check if input file content having this regexp
    def regexp?(filepath, regexp)
        File.foreach(filepath) do |line|
            return true if line =~ regexp
        end
        false
    end

    # Grant full right to a file/folder (Linux only)
    def grant_full_rights(path)
        system("chmod -R 777 \"#{path}\"")
    end

    # New call_system_command function
    # Should provide error stack trace as well on error/exception happens
    # https://blog.bigbinary.com/2012/10/18/backtick-system-exec-in-ruby.html
    # Using popen2e will merge stdout and stderr to same stream. We output to RMX log all of them
    def call_system_command(step, cmd_name, cmd, full_log = :yes)
        Open3.popen2e(cmd) do |_stdin, stdout_err, wait_thr|
            while stdout_err.gets
                # Full log will include Chef format timestamp inside log file.
                if full_log == :yes
                    log_info(stdout_err.gets)
                else # Some Teamcenter utility command return alot of logs and in long line. This way, we can strip the Chef timestamp part.
                    puts(" #{stdout_err.gets}")
                end
            end

            exit_status = wait_thr.value
            unless exit_status.success?
                raise "Command #{cmd_name} failed when try to #{step}"
            end
        end
    end

    # Bug 68052: Upgrade RMD will produce exit code 3010 - ERROR_SUCCESS_REBOOT_REQUIRED. Pass a command with specific exit code
	def call_system_command_code_list(step, cmd_name, cmd, success_return_code_list, full_log = :yes)
		Open3.popen2e(cmd) do |_stdin, stdout_err, wait_thr|
			while stdout_err.gets
				# Full log will include Chef format timestamp inside log file.
				if full_log == :yes
					log_info(stdout_err.gets)
				else # Some Teamcenter utility command return alot of logs and in long line. This way, we can strip the Chef timestamp part.
					puts(" #{stdout_err.gets}")
				end
			end
			
			exit_status = wait_thr.value
			log_info("Upgrade Process : #{exit_status}")
			unless exit_status.success? || success_return_code_list.include?(exit_status.exitstatus)
				raise "Command #{cmd_name} failed when try to #{step}"
			end
		end
	end

    # Set new environment variable. either machine type or user type
    def set_environment_variable(env_name, env_value, target)
        # Build command line string to set env variable
        case target
        when :system
            set_cmd = "setx -m \"#{env_name}\" \"#{env_value}\""
        when :user
            set_cmd = "setx \"#{env_name}\" \"#{env_value}\""
        else
            return
        end

        Open3.popen2e(set_cmd) do |_stdin, stdout_err, return_code|
            if return_code.value.success?
                log_info("Registered #{target} environment variable #{env_name}.")
            else
                log_info("Can't register #{target} environment variable : #{env_name}")
                log_info(stdout_err.gets) while stdout_err.gets
                raise("Failed to register #{target} environment variable #{env_name}")
            end
        end
    end

    # Remove existing environment variable
    def remove_environment_variable(env_name, target)
        # Build command line string to set env variable
        case target
        when :system
            remove_cmd = "REG delete \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\" /F /V \"#{env_name}\""
        when :user
            remove_cmd = "REG delete \"HKCU\\Environment\" /F /V \"#{env_name}\""
        else
            return
        end

        Open3.popen2e(remove_cmd) do |_stdin, stdout_err, return_code|
            if return_code.value.success?
                log_info("Removed #{target} environment variable #{env_name}.")
            else
                log_info("Can't remove #{target} environment variable : #{env_name}")
                log_info(stdout_err.gets) while stdout_err.gets
                raise("Failed to remove #{target} environment variable #{env_name}")
            end
        end
    end

    def log_info(message)
        return if message.nil?
        # Standard log in UI mode.
        Chef::Log.info(message)
        return unless node['silence_install'] == 'yes'

        # If in silent install mode. Message are flush only to log file. Use puts to redirect to console also
        if message.include?('[Progress]') # Need to add new line for new ruby_block run (Better display on console during silence install)
            puts "\n#{message}"
        else
            puts message
        end
    end

    # Set break point when installer error. The recipe will continue execute in the next run
    def set_break_point(tmp_dir, break_recipe)
        hash = {}
        hash['break_point'] = break_recipe
        break_point_path = File.join(tmp_dir, 'break_point.json')
        File.open(break_point_path, 'w+') do |file|
            file.write(JSON.pretty_generate(hash))
        end
    end

    # Extract .zip file to destination folder
    def extract_zip(file, destination)
        FileUtils.mkdir_p(destination) unless File.exist?(destination)

        Zip::File.open(file) do |zip_file|
            zip_file.each do |f|
                fpath = File.join(destination, f.name)
                the_dir = File.dirname(fpath)
                FileUtils.mkdir_p(the_dir) unless File.exist?(the_dir)
                FileUtils.remove_file(fpath, true) if File.exist?(fpath)
                zip_file.extract(f, fpath)
            end
        end
    end

    # Update service name of Teamcenter Service (Dispatcher Module, Pool Manager, ...)
    def preprocess_tc_services(tc_root)
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            Open3.popen3('sc query type= service state= all | find "SERVICE_NAME: Teamcenter"') do |_stdrin, stdout, _stderr|
                output = stdout.read
                output.each_line do |line|
                    # index = "SERVICE_NAME: ".length
                    index = 14
                    sc_name = line[index, line.length].chop
                    # node.override['service_dispModule'] = sc_name if sc_name.include?('Dispatcher Module')
                    node.override['service_tcserver'] = sc_name if sc_name.include?('TeamcenterServerManager') || sc_name.include?('Teamcenter Server Manager')
                end
            end
        elsif RUBY_PLATFORM =~ /linux/
            # only have pool manager, cannot deploy .Net
            tc_server_pool = File.join(tc_root, 'pool_manager', 'confs', '**', 'rc.tc.mgr_*')
            tc_server_pool = Dir.glob(tc_server_pool)[0]
            node.override['service_tcserver'] = tc_server_pool
        end
    end

    # Stop Dispatcher Client and Module
    def stop_dispatcher(disp_root)
        disp_client = File.join(disp_root, 'DispatcherClient', 'bin', Script::RUNDISPATCHERCLIENT)
        disp_module = File.join(disp_root, 'Module', 'bin', Script::RUNMODULE)
        system("\"#{disp_client}\" -stop") if File.exist?(disp_client)
        system("\"#{disp_module}\" -stop") if File.exist?(disp_module)
    end

    # BBIntegration only: Modify bb_nx jar file plugin to include Nextlabs customized path
    def modify_nx_jar(tmp_dir, bb_dir, oldstring, newstring)
        files = File.join(bb_dir, 'plugins', '*.jar')
        jar_content = File.join(tmp_dir, 'jar_content')
        FileUtils.mkdir_p jar_content
        find_file = ''
        Dir[files].each do |file|
            if File.basename(file).include?('com.teamcenter.bce.cad.nx')
                find_file = file
                break
            end
        end
        if find_file != ''
            system("cd /D \"#{jar_content}\" && jar xf \"#{find_file}\"")
            plugin_xml = File.join(jar_content, 'plugin.xml')

            # modify plugin.xml
            lines = IO.readlines(plugin_xml).map do |line|
                if line.include?(oldstring)
                    '            class="' + newstring + '">'
                else
                    line
                end
            end
            File.open(plugin_xml, 'w') do |file|
                file.puts lines
            end

            class_dir = File.join(jar_content, 'com', 'teamcenter', 'bce', 'internal', 'cad', 'nx')
            nxlnxpostopen = File.join(ENV['RMX_ROOT'], 'BBIntegration', 'NxlNXPostOpen.class')
            if newstring == BBPlugin::NXLNXPOSTOPEN
                # install process: copy nextlabs customized NXPostOpen class
                FileUtils.cp_r(nxlnxpostopen, class_dir)
            elsif newstring == BBPlugin::NXPOSTOPEN
                # uninstall process remove nextlabs customized NxlNXPostOpen class
                FileUtils.remove(File.join(class_dir, 'NxlNXPostOpen.class'))
            end

            # pack and replace the new jar
            basename = File.basename(find_file)
            system("cd /D \"#{jar_content}\" && jar cfM #{basename} *")
            FileUtils.cp_r(File.join(jar_content, basename), File.join(bb_dir, 'plugins'))
            FileUtils.remove_dir(jar_content, true)
        end
    end

    # Get JAVA version installed in this machine. Prefer JAVA_HOME over JRE_HOME
    def get_java_version(java_home, jre_home)
        java_dir = java_home.nil? ? jre_home : java_home
        release_file = File.join(java_dir, 'release')
        regexp = /JAVA_VERSION=.*/
        java_version = ''
        File.foreach(release_file) do |line|
            if line =~ regexp
                java_version = line
                break
            end
        end

        version_dir = java_version.split('"')
        # java version = 1.7 / 1.8
        java_version = version_dir[1][0, 3]

        # return: simple version = 7 / 8
        (java_version.to_f * 10).to_i % 10
    end

    # Delete XML elements
    def delete_xml_element(filepath, indent, element)
        file = File.new(filepath)
        doc = REXML::Document.new(file)
        doc.delete_element element
        file = File.new(filepath, 'w')
        doc.write(file, indent)
        file.close
    end

    # Check if this file is ended with new line
    def file_endwith_new_line(filepath)
        file = File.new(filepath)
        lines = file.readlines
        file.close
        lines[lines.length - 1].end_with?("\n")
    end

    # Delete file content with given label
    def delete_file_content_with_label(filepath, label)
        file = File.new(filepath)
        lines = file.readlines
        start_id = -1
        end_id = -1
        (0...lines.length).each do |i|
            next unless lines[i].include?(label)

            if start_id == -1
                start_id = i
            else
                end_id = i
            end
        end
        return if start_id == -1
        end_id = start_id if end_id == -1
        file.close
        lines.slice!(start_id, end_id - start_id + 1)

        file = File.new(filepath, 'w')
        file.puts lines
        file.close
    end

    def check_vis_server_installed(tc_root)
        return false if tc_root.nil? || tc_root == ''

        path = File.join(tc_root, 'install', 'configuration.xml')
        file = File.new(path)
        doc = REXML::Document.new(file)
        doc.elements.each('*/config/features/installed') do |element|
            return true if element.attributes['feature'] == Installer::VIS_SERVER_MANAGER
        end
        false
    end

    # Check if scf feature is install (Different template will be used)
    def scf_install?(feature_list)
        feature_list.each do |feature|
            return true if feature == 'scf_integration'
        end
        false
    end

    def get_secret_from_properties(remove_crypt_prefix = true)
        secret_properties = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'secret.properties')
        return '' unless File.exist?(secret_properties)
        
        File.open(secret_properties, 'r') do |file|
            file.read.each_line do |line|
                line = line.strip!
                if (line.include?('SECRET='))
                    secret_value = line.split('SECRET=')[1]
                    if secret_value.include?('crypt\:') && remove_crypt_prefix
                        secret_without_crypt = secret_value.split('crypt\:')[1]
                        return secret_without_crypt
                    else
                        return secret_value
                    end
                end
            end
        end
    end

    def get_app_key_from_properties
        config_properties = File.join(ENV['RMX_ROOT'], 'config', 'config.properties')
        return '' unless File.exist?(config_properties)

        File.open(config_properties, 'r') do |file|
            file.read.each_line do |line|
                line = line.strip!
                if (line.include?('APP_KEY='))
                    app_key = line.split('APP_KEY=')[1]
                    return app_key
                end
            end
        end
    end

    # Return if a cad application is installed or not in this VM (check via Control Panel Program list)
    def is_cad_rmx_installed(cad_application)
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'

        case cad_application
        when :nx
            rmx_name = 'NextLabs Rights Management eXtension For NX'
        when :creo
            rmx_name = 'NextLabs Rights Management eXtension For Creo'
        when :solidworks
            rmx_name = 'NextLabs Rights Management eXtension For SolidWorks'
        when :autocad
            rmx_name = 'NextLabs Rights Management eXtension For AutoCAD'
        when :catia
            rmx_name = 'NextLabs Rights Management eXtension For CATIA'
        else
            rmx_name = '__RMX__'
        end

        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime|
                    final_key = (control_panel + '\\' + program)
                    registry.open(final_key, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |final_registry|
                        final_registry.each_value do |name, _type, data|
                            if name == "DisplayName" && data.include?(rmx_name)
                                return true
                            end
                        end
                    end
                end
            end
        rescue
            puts 'Registry Not Found'
        end
        return false
    end

    def get_rmd_existing_version()
        return [false, ''] unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'
        rmd_name = 'SkyDRM Desktop for Windows'

        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime|
                    final_key = (control_panel + '\\' + program)
                    registry.open(final_key, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |final_registry|
                        final_registry.each_value do |name, _type, data|
                            if name == "DisplayName" && data.include?(rmd_name)
                                _ , version = final_registry.read("DisplayVersion")
                                version = version.split('.')[-1]
                                return [true, version]
                            end
                        end
                    end
                end
            end
        rescue
            return [false, '']
        end
        return [false, '']
    end
end
