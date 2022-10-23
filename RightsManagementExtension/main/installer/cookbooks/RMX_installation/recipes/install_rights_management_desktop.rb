# Teamcenter RMX 5.4
# Cookbook Name :: RMX_installation
# Recipes       :: install_rights_management_desktop
# Description   :: Run RMD setup.exe
# Author        :: Phan Anh Tuan
# Copyright 2020, Nextlabs Inc.

require 'fileutils'
require 'json'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/

ruby_block 'deploy_rmd_rpm' do
    block do
        # Install RMD/RPM should be Window only
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            log_info(LOGMSG::INSTALL_RMD)
            certify_version = node['certify_rmd']
            is_rmd_installed, existing_version = get_rmd_existing_version()
            rmd_installer = File.join(ENV['RMX_ROOT'], 'Setup.exe').tr('/', '\\')
            log_location = File.join(node['temp_dir'], 'installrmd.log').tr('/', '\\')

            if !is_rmd_installed
                log_info('Deploying RMD/RPM')
                cmd = "cd \"#{ENV['RMX_ROOT']}\" & setup.exe /s /v\"/qn REBOOT=Suppress\" /v\""                                 # This command will work on both GUI and silence mode
                # cmd = "cd \"#{ENV['RMX_ROOT']}\" & setup.exe /s /v\"/qn REBOOT=Suppress\" /v\"/l*v \"#{log_location}\""       # This command will work on GUI mode only...?
                call_system_command('Run RMD installer', 'msiexec', cmd)
            elsif existing_version.to_i < certify_version.to_i
                log_info('Dected older version of RMD/RPM. Deploying new RMD/RPM')
                cmd = "cd \"#{ENV['RMX_ROOT']}\" & setup.exe /s /v\"/qn REBOOT=Suppress\" /v\""
                # Bug 68052: Upgrade RMD will produce exit code 3010 - ERROR_SUCCESS_REBOOT_REQUIRED.
				# In Ruby, if process exit with 3010, exitstatus will be 194 (strangely...).
				# Can be reproduce by execute system("exit 3010") and $? consecutively
                call_system_command_code_list('Run RMD installer', 'msiexec', cmd, [194])
            else
                log_info('Detect existing certify version RMD/RPM. Abort redeploy RMD/RPM')
            end
        end
    end
end

