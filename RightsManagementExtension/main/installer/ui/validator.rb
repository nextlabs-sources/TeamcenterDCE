require 'json'
require 'resolv'
require 'tmpdir'
require 'fileutils'
require 'rexml/document'
require 'open3'
require 'singleton'
require 'net/http'
require 'openssl'

# Validate inputs by user during GUI Installer
module Validator
    MESSAGE = JSON.parse(IO.read(File.join(File.dirname(__FILE__), 'message_properties.json'), encoding: 'utf-8'))

    # define recommended disk space for installation package: 1500 MB
    MINIMUM_DISK_SPACE = 1500 * 1024 * 1024

    # define constant names for Teamcenter utilities
    TC_PROFILEVARS = RUBY_PLATFORM =~ /mswin|mingw|windows/ ? 'tc_profilevars.bat' : 'tc_profilevars'
    GETSITE_INFO = RUBY_PLATFORM =~ /mswin|mingw|windows/ ? 'getsite_info.exe' : 'getsite_info'

    # validator modules includes some valiators that are
    # methods that accept an input and returns (valid, error_msg)
    @validator_errors_msgs = MESSAGE['validator_errors']

    def validate_java_env
        if ENV['JAVA_HOME'].nil?
            error_msg = "Environment variable JAVA_HOME is required.\nPlease close GUI and update your environment variable."
            return [false, error_msg]
        end

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            java_home_bin = File.join(ENV['JAVA_HOME'], 'bin').tr('/', '\\')
            is_java_exe_under_bin_folder = File.exist?(File.join(java_home_bin, 'java.exe'))
            is_java_home_bin_in_path = ENV['PATH'].tr('/', '\\').split(';').include?(java_home_bin)
        else
            java_home_bin = File.join(ENV['JAVA_HOME'], 'bin')
            is_java_exe_under_bin_folder = File.exist?(File.join(java_home_bin, 'java'))
            is_java_home_bin_in_path = ENV['PATH'].split(':').include?(java_home_bin)
        end

        if !is_java_home_bin_in_path
            error_msg = "%JAVA_HOME%\\bin is required in %PATH% environment variable.\nPlease close GUI and update your environment variable."
            return [false, error_msg]
        elsif !is_java_exe_under_bin_folder
            error_msg = "Invalid JAVA_HOME environment variable.\nUnable to find java executable under current %JAVA_HOME%\\bin.\nPlease close GUI and update your environment variable."
            return [false, error_msg]
        end
        [true, '']
    end

    def validate_fms_home
        if ENV['FMS_HOME'].nil?
            error_msg = "Detected that environment able to deploy PartialDownload but following environment variable FMS_HOME is not set.\nPlease close GUI and update your environment variable in order to deploy PartialDownload"
            return [false, error_msg]
        end
        valid = FileTest.directory?(ENV['FMS_HOME']) ? true : false
        error_msg = valid ? '' : 'FMS_HOME directory is not a valid directory'
        [valid, error_msg]
    end

    # Given install_path, ensure enough disk space for installation in that drive
    # Alternative by ruby: https://stackoverflow.com/questions/3258518/ruby-get-available-disk-drives
    def validate_disk_space(install_path)
        # Only check on Window for now.
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            begin
                drive_of_install_path = install_path[0..1]
                available_space = -1
                full_cmd = "wmic LogicalDisk Get DeviceID,FreeSpace|find /I \"#{drive_of_install_path}\""
                Open3.popen2e(full_cmd) do |_stdin, stdout_err, _wait_thr|
                    while line = stdout_err.gets
                        available_space = line.gsub(drive_of_install_path, '').strip.to_i if line.include?(drive_of_install_path)
                    end
                end
                return [true, ''] if available_space == -1 # In case executed success but not get the disk space available, allow pass anyway

                return available_space > MINIMUM_DISK_SPACE ? [true, ''] : [false, 'Not enough disk spaces to install. Recommend to have at least 1 GB free of disk space for installation process.']
            rescue StandardError
                return [true, ''] # In case cmd not execute, allow pass anyway
            end
        end
        [true, ''] # Pass this test for Linux for now since not test this platform yet
    end

    def validate_install_dir(dir)
        return validate_linux_install_dir(dir) if RUBY_PLATFORM =~ /linux/
        validate_window_install_dir(dir)
    end

    def validate_window_install_dir(dir)
        valid = FileTest.directory?(dir) ? true : false
        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['dir']) || 'The Window directory is not exist or valid')

        [valid, error_msg]
    end

    def validate_linux_install_dir(dir)
        valid = true
        error_msg = ''
        if !FileTest.directory?(dir)
            valid = false
            error_msg = (@validator_errors_msgs && @validator_errors_msgs['dir']) || 'The Linux directory is not exist or valid'
        elsif dir.include?(' ') # if the installation is under linux, then path with space will result into some strange behavior
            valid = false
            error_msg = (@validator_errors_msgs && @validator_errors_msgs['dir_contain_space']) || 'The Linux directory path should not contain space'
        end

        [valid, error_msg]
    end

    def validate_port(port)
        valid, error_msg = validate_non_empty(port)
        return [valid, error_msg] unless valid

        valid = (port.to_s =~ /^[1-9]\d{0,4}$/ ? true : false) && (port.to_i <= 65_535)
        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['port']) || 'Not a valid port')
        [valid, error_msg]
    end

    # Currently no usage of this function
    def validate_ip(ip)
        # ip address or localhost
        # for validate ip adreess, refer to http://goo.gl/RsnfkC
        valid = ip.eql?('localhost') || (ip =~ Resolv::IPv4::Regex ? true : false)
        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['ip']) || 'Not a valid ip')
        [valid, error_msg]
    end

    def validate_file_exist(path)
        valid = FileTest.file?(path) ? true : false
        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['file']) || 'The file you specified is not exist')
        [valid, error_msg]
    end

    def validate_non_empty(input)
        valid = !input.nil? && (input != '')
        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['non_empty']) || 'This field should not be empty')
        [valid, error_msg]
    end

    def validate_tc_data(path)
        return [true, ''] if path.nil?

        path = path.strip
        tc_profilevar = File.join(path, TC_PROFILEVARS)
        tcxml = File.join(path, 'TCXML.xsd')
        valid = path == '' || ((FileTest.file? tc_profilevar) && (FileTest.file? tcxml)) ? true : false

        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['invalid']) || 'The tc_data directory is invalid')
        [valid, error_msg]
    end

    def validate_tc_root(path)
        valid = false
        if path.nil?
            valid = true
        else
            path = path.strip
            config_xml = File.join(path, 'install', 'configuration.xml')
            valid = true if path == '' || (FileTest.file? config_xml)
        end

        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['invalid']) || 'The tc_root directory is invalid')
        [valid, error_msg]
    end

    def validate_disp_root(path)
        valid = false
        if path.nil?
            valid = true
        else
            path = path.strip
            translator_xml = File.join(path, 'Module', 'conf', 'translator.xml')
            valid = true if path == '' || (FileTest.file? translator_xml)
        end

        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['invalid']) || 'The dispatcher root directory is invalid')
        [valid, error_msg]
    end

    def validate_bb_dir(path)
        valid = false
        if path.nil?
            valid = true
        else
            path = path.strip
            bb_exe = File.join(path, 'BriefcaseBrowser.exe')
            valid = true if path == '' || (FileTest.file? bb_exe)
        end

        error_msg = valid ? '' : ((@validator_errors_msgs && @validator_errors_msgs['invalid']) || 'The bb directory is invalid')
        [valid, error_msg]
    end

    def validate_infodba_password(tc_root, tc_data, password)
        ENV['TC_DATA'] = tc_data.tr('\\', '/').gsub('Program Files', 'PROGRA~1')
        ENV['TC_ROOT'] = tc_root.tr('\\', '/').gsub('Program Files', 'PROGRA~1')
        site_info = File.join(tc_root, 'bin', GETSITE_INFO)
        tc_pro = '"' + File.join(tc_data, TC_PROFILEVARS) + '"'
        tc_pro = '. ' + tc_pro if RUBY_PLATFORM =~ /linux/
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            site_info = site_info.tr('/', '\\')
            tc_pro = tc_pro.tr('/', '\\')
        end
        valid = true
        begin
            result = system("#{tc_pro} && \"#{site_info}\" -u=infodba -p=#{password} -g=dba")
            valid = false unless result
        rescue StandardError => _err
            puts('Failed to execute GETSITE_INFO command. Failed to validate infodba password')
            valid = false
        end
        error_msg = valid ? '' : 'Invalid Password'
        [valid, error_msg]
    end

    def validate_creo_dir(path)
        return [true, ''] if path == '' # Allow Creo Dir is empty
        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The Creo directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'bin', 'parametric.exe'))
            valid = false
            error_msg = 'Unable to find parametric.exe under bin folder of this directory'
        end
        [valid, error_msg]
    end

    def validate_ipem_dir(path)
        return [true, ''] if path == '' # Allow IPEM Dir empty
        
        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The IPEM directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'ipem.jar'))
            valid = false
            error_msg = 'Unable to find ipem.jar under this IPEM directory. Please check IPEM dir again.'
        end
        [valid, error_msg]
    end

    def validate_solidworks_dir(path)
        return [true, ''] if path == '' # Allow SolidWorks Dir is empty
        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The Solidworks directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'SOLIDWORKS', 'SLDWORKS.exe'))
            valid = false
            error_msg = 'Unable to find SLDWORKS.exe under SOLIDWORKS folder of this directory'
        end
        [valid, error_msg]
    end

    def validate_swim_dir(path)
        return [true, ''] if path == '' # Allow SWIM Dir empty
        
        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The SWIM directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'swim.jar'))
            valid = false
            error_msg = 'Unable to find swim.jar under this SWIM directory. Please check SWIM dir again.'
        end
        [valid, error_msg]
    end

    def validate_autocad_dir(path)
        return [true, ''] if path == '' # Allow AutoCAD Dir empty 

        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The AutoCAD directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'acad.exe'))
            valid = false
            error_msg = 'Unable to find acad.exe under this AutoCAD Directory. Please check again'
        end
        [valid, error_msg]
    end

    def validate_catia_dir(path)
        return [true, ''] if path == '' # Allow CATIA Dir empty 

        valid = true
        error_msg = ''
        if !FileTest.directory?(path)
            valid = false
            error_msg = 'The CATIA directory is an invalid directory'
        elsif !File.exist?(File.join(path, 'code', 'bin', 'CNEXT.exe'))
            valid = false
            error_msg = 'Unable to find CNEXT.exe under this CATIA Directory. Please check again'
        end
        [valid, error_msg]
    end
    
    def validate_skydrm_connection(router, app_id, app_key)
        return true

        # Send REST API to SkyDRM and verify this 3 information
    end

    def validate_cc_connection(cc_host, cc_port)
        url = URI("https://#{cc_host.strip}:#{cc_port.strip}")
        http = Net::HTTP.new(url.host, url.port)

        http.use_ssl = true
        # CC use self-signed SSL certificate. And only this verify_mode allow to send REST API, otherwise get certificate error
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE

        begin
            response = http.get("/cas/security/csrfToken")

            if response.code == "200"
                return [true, '']
            else
                return [false, 'Connection to Control Center failed.']
            end
        rescue StandardError => ex
            # Usually get error, exeception about wrong host, will raise problem here
            return [false, 'Connection Failed. Possible Control Center host or port not recognized.']
        end
    end

    def validate_cc_api(cc_host, cc_port, cc_apiuser, cc_apisecret)
        url = URI("https://#{cc_host.strip}:#{cc_port.strip}")
        http = Net::HTTP.new(url.host, url.port)
        http.read_timeout = 5
        http.use_ssl = true
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE

        request = Net::HTTP::Post.new("/cas/token?grant_type=client_credentials&client_id=#{cc_apiuser.strip}&client_secret=#{cc_apisecret.strip}&expires_in=31536000")

        begin
            response = http.request(request)
            if response.body.include?("access_token")
                return [true, '']
            else
                return [false, 'Failed to authenticate API Account.']
            end
        rescue StandardError => ex
            # Usually get error, exeception about wrong host/port, will raise problem here
            return [false, 'Connection Failed. Possible Control Center host or port not recognized.']
        end
    end

    def validate_jpc_connection(jpc_host, jpc_eva_port, is_jpc_https)
        url = URI("https://#{jpc_host.strip}:#{jpc_eva_port.strip}")
        http = Net::HTTP.new(url.host, url.port)

        if is_jpc_https
            http.use_ssl = true
            http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        end

        begin
            response = http.get("/dpc/authorization/pdp/")

            if response.code == "200"
                return [true, '']
            else
                return [false, 'Connection to Policy Controller failed.']
            end
        rescue StandardError => ex
            # Usually get error, exeception about wrong host/port, will raise problem here
            return [false, 'Connection Failed. Possible Policy Controller host or port not recognized.']
        end
    end
end