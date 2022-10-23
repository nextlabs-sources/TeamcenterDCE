# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: bootstrap
# Description   :: Pre-installation step, setup variables, feature list and common library file
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/
Chef::Resource::RubyBlock.send(:include, Utility)

begin
    Chef::Recipe.send(:include, Compile::Config)

    raise('START_DIR environment variable is not set, abort') if ENV['START_DIR'].nil?
    node.override['TC_ROOT'] = convert_path_to_unix_style(node['tc_root'])
    node.override['TC_DATA'] = convert_path_to_unix_style(node['tc_data'])
    node.override['DISP_ROOT'] = convert_path_to_unix_style(node['disp_root'])
    node.override['install_dir'] = convert_path_to_unix_style(node['install_dir'])
    node.override['bb_dir'] = convert_path_to_unix_style(node['bb_dir'])
    # node.override['creo_dir'] = convert_path_to_unix_style(node['creo_dir'])
    ENV['TC_DATA'] = node['TC_DATA']
    ENV['TC_ROOT'] = node['TC_ROOT']
    ENV['TC_DATA'] = node['TC_DATA'].tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
    ENV['TC_ROOT'] = node['TC_ROOT'].tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/

    # Update tc_profilevar.bat file if TC_DATA is present
    if File.exist?(node['TC_DATA'])
        tc_pro = '"' + File.join(node['TC_DATA'], Script::TC_PROFILEVARS) + '"'
        tc_pro = '. ' + tc_pro if RUBY_PLATFORM =~ /linux/
        tc_pro = tc_pro.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/

        node.override['tc_profilevar'] = tc_pro
    end

    # password setting for for UI install only
    node.override['infodba_psd'] = ENV['infodba_psd'] unless ENV['infodba_psd'].nil?
    node.override['tc_user_psd'] = ENV['tc_user_psd'] unless ENV['tc_user_psd'].nil?
    node.override['skydrm_app_key'] = ENV['skydrm_app_key'] unless ENV['skydrm_app_key'].nil?
    node.override['cc_apisecret'] = ENV['cc_apisecret'] unless ENV['cc_apisecret'].nil?
    # When rollback an uninstallation attempt (skydrm_app_key and cc_apisecret not available to recreate config file). Should collect them from existing config file
    # Only collect APP_KEY and APISECRET if installation is made on server (or else, rollback can fail)
    if ENV['rollback'] == 'true' && node['installation_mode'].eql?('install') && File.exist?(node['TC_DATA'])
        node.override['skydrm_app_key'] = get_app_key_from_properties
        node.override['cc_apisecret'] = get_secret_from_properties(false)
    end
    FileUtils.mkdir_p(node['temp_dir'])

    # Some information during upgrade
    if node['installation_mode'] == 'upgrade'
        tcrmx_old_version
        node.override['OLD_RMX_VERSION'] = ENV['OLD_RMX_VERSION'] unless ENV['OLD_RMX_VERSION'].nil?
        check_template_upgrade
        node.override['template_upgrade'] = ENV['template_upgrade'] unless ENV['template_upgrade'].nil?
    end

    # get Teamcenter version
    # 1. nxlautoprotect lib 10/11
    # 2. partialdownload version
    if File.exist?(File.join(node['TC_ROOT'], 'portal'))
        node.override['tc_full_version'] = get_teamcenter_full_version(node['TC_ROOT'])
        node.override['tc_version'] = node['tc_full_version'][0, 2]
    end

    # check AWS runtime server
    node.override['aws_runtime'] = true if node['TC_ROOT'] != '' && check_teamcenter_feature(node['TC_ROOT'], Installer::AWS_RUNTIME_SERVER)
end

ruby_block 'preparation_step' do
    block do
        log_info(LOGMSG::BOOTSTRAP)
        # Bug fix 47022: stop RMC service, Dispatcher Client and Teamcenter processes
        stop_running_process(TeamcenterProcess::TEAMCENTER_EXE)
        stop_running_process(TeamcenterProcess::TAO_IMR_LOCATOR)
        stop_running_process(TeamcenterProcess::TAO_IMR_ACTIVATOR)
        stop_dispatcher(node['DISP_ROOT'])

        # retrieve names of Teamcenter services
        # mainly aiming at teamcenter server manager service name, have to stop this service before replacing nxlautoprotect.dll
        # otherwise, file-in-use error will pop up
        preprocess_tc_services(node['TC_ROOT'])

        current_dir = String.new(ENV['START_DIR']).tr('\\', '/').strip
        current_dir = current_dir.chop if current_dir.end_with?('/')

        if node['installation_mode'].eql?('install')
            tcdrm = File.join(node['install_dir'], 'TcDRM')
            ENV['RMX_ROOT'] = tcdrm

            # set system environment variable RMX_ROOT on Windows or Linux
            tcdrm_sys = tcdrm.tr('/', '\\')
            if RUBY_PLATFORM =~ /mswin|mingw|windows/
                set_environment_variable('RMX_ROOT', tcdrm_sys, :system)
            else
                nextlabs_env = '/etc/profile.d/nextlabs_env.sh'
                file = File.new(nextlabs_env, 'w+')
                file.puts "export RMX_ROOT=#{tcdrm}"
                file.close
            end

            # If current dir is the install_dir, then no need to perform copy TcDRM folder
            unless node['install_dir'].eql?(current_dir)
                FileUtils.mkdir_p(tcdrm) unless File.exist?(tcdrm)
                # Skip Copy files if RMX is installed and silence re-installing (avoid overwrite config files...)
                unless File.exist?(File.join(node['temp_dir'], 'has_installed.txt')) && node['installation_mode'] == 'install' && node['silence_install'] == 'yes'
                    src = Dir[File.join(current_dir, 'TcDRM', '*')]
                    src.each do |file|
                        FileUtils.cp_r(file, tcdrm)
                    end
                end
            end

            # If feature contains PartialDownload/TRC/SCFIntegration/BBIntegration then prepare SkyDRM config file
            need_skydrm_config = false
            node['feature_list'].each do |feature|
                if (feature == 'partialdownload' && check_vis_server_installed(node['TC_ROOT'])) || feature == 'translator_rights_checker'
                    need_skydrm_config = true
                    break
                end
            end

            if need_skydrm_config
                # All feature using KMS configuration (config.properties) will share same config file in RMX_ROOT/config/config.properties
                # Features using this config file: PartialDownload, TranslatorRightsChecker
                # For now, the password will include "crypt:" prefix (for all feature)
                # Only Window need config/config.properties since partialdownload + translator_rights_checker only support with Windows Server
                if RUBY_PLATFORM =~ /mswin|mingw|windows/
                    log_info('Prepare SkyDRM configuration file')
                    nextlabs_config = File.join(ENV['RMX_ROOT'], 'config', 'config.properties')
                    file = File.new(nextlabs_config, 'w+')
                    file.puts 'ROUTER_URL=' + node['skydrm_router_url']
                    # file.puts 'TENANT_NAME=' + node['skydrm_tenant_name']
                    file.puts 'APP_ID=' + node['skydrm_app_id']
                    file.puts 'APP_KEY=' + node['skydrm_app_key'] # or ENV['skydrm_app_key']
                    file.close

                    encrypt_config_password = File.join(ENV['RMX_ROOT'], 'config', 'encryptConfigPassword.bat')
                    encrypt_config_password_cmd = "\"#{encrypt_config_password}\""

                    # Unique way to run this command. Use call_system_command result in a very ugly display in both log file and silence install console
                    output = Open3.popen3(encrypt_config_password_cmd) { |_stdin, stdout, _stderr, _wait_thr| stdout.read }
                    log_info(output)
                end
            end

            # If feature have TRC, dispatcher_module or nxl_template -> need PDP information in properties file and preferences
            if node['feature_list'].include?('translator_rights_checker') || node['feature_list'].include?('dispatcher_module') || node['feature_list'].include?('nxl_template')
                log_info('Prepare Policy Controller configuration file')
                policy_config = File.join(ENV['RMX_ROOT'], 'config', 'policy_config.properties')
                file = File.new(policy_config, 'w+')
                file.puts 'nextlabs.pdp.engine.name=com.nextlabs.openaz.pdp.RestPDPEngine'
                file.puts 'nextlabs.cloudaz.payload_content_type=application/json'
                file.puts "nextlabs.cloudaz.host=#{node['jpc_remote_ip']}"
                file.puts "nextlabs.cloudaz.port=#{node['jpc_eva_port']}"
                file.puts "nextlabs.cloudaz.https=#{node['jpc_https']}"
                file.puts 'nextlabs.cloudaz.auth_type=OAUTH2'
                file.puts 'nextlabs.cloudaz.oauth2.grant_type=client_credentials'
                file.puts "nextlabs.cloudaz.oauth2.server=#{node['cc_host']}"
                file.puts "nextlabs.cloudaz.oauth2.port=#{node['cc_port']}"
                file.puts "nextlabs.cloudaz.oauth2.https=#{node['cc_oauth2_https']}"
                file.puts "nextlabs.cloudaz.oauth2.client_id=#{node['cc_apiuser']}"
                # Temporary still put plain sercret here for TRC module until fully transfer to encrypted version
                # file.puts "nextlabs.cloudaz.oauth2.client_secret=#{node['cc_apisecret']}"
                file.puts "nextlabs.cloudaz.oauth2.token_endpoint_path=/cas/token"
                file.puts "nextlabs.cloudaz.ignore_https_certificate=#{node['cc_ignore_https_cert']}"
                file.puts "xacml.pdpEngineFactory=com.nextlabs.openaz.pdp.PDPEngineFactoryImpl"
                file.puts "pep.mapper.classes=com.nextlabs.openaz.pepapi.RecipientMapper,com.nextlabs.openaz.pepapi.DiscretionaryPoliciesMapper,com.nextlabs.openaz.pepapi.HostMapper,com.nextlabs.openaz.pepapi.ApplicationMapper"
                file.close

                secret_config = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'secret.properties')
                secret_file = File.new(secret_config, 'w+')
                secret_file.puts "SECRET=#{node['cc_apisecret']}"
                secret_file.close

                if RUBY_PLATFORM =~ /mswin|mingw|windows/
                    encrypt_secret_password = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'encryptSecret.bat')
                    encrypt_secret_password_cmd = "\"#{encrypt_secret_password}\""
                else
                    encrypt_secret_password = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'encryptSecret.sh')
                    grant_full_rights(encrypt_secret_password)
                    encrypt_secret_password_cmd = "\"#{encrypt_secret_password}\""
                end

                output = Open3.popen3(encrypt_secret_password_cmd) { |_stdin, stdout, _stderr, _wait_thr| stdout.read }
                log_info(output)

                # Override cc_apisercret with encrypted version
                node.override['cc_apisecret'] = get_secret_from_properties
            end

            # User will need to copy JCEPolicy download from Oracle site and puts into RMX-5.0..../TcDRM/shared_libs/UnlimitedJCEPolicyJDK{java_version} folder
            # lcw: java environment shall be set in /etc/environment and /etc/profile
            java_home = ENV['JAVA_HOME']
            jre_home = ENV['JRE_HOME']
            if java_home.nil? && jre_home.nil?
                puts(`echo Current shell $0`)
                puts(`env`) if RUBY_PLATFORM =~ /linux/
                puts(`set`) if RUBY_PLATFORM =~ /mswin|mingw|windows/
                raise('JAVA_HOME and JRE_HOME environment variable are not set, abort')
            end
            jre_home = ENV['JRE64_HOME'] if jre_home.nil?
            java_version = get_java_version(java_home, jre_home)
            log_info("Detected a Java #{java_version} installation")
			
        elsif node['installation_mode'].eql?('upgrade')
            tcdrm = ENV['RMX_ROOT']
            # Copy out all config file to retain their value
            log_info('Backup configuration file')
            temp_dir = node['temp_dir']
            general_config = File.join(tcdrm, 'config', 'config.properties')
            connection_config = File.join(tcdrm, 'BatchProtect', 'connection.properties')
            policy_eval_config = File.join(tcdrm, 'TranslatorRightsChecker', 'policy_evaluation.properties')
            
            FileUtils.cp_r(general_config, temp_dir) if File.exist?(general_config) && !File.exist?(File.join(temp_dir, 'config.properties'))
            FileUtils.cp_r(connection_config, temp_dir) if File.exist?(connection_config) && !File.exist?(File.join(temp_dir, 'connection.properties'))
            FileUtils.cp_r(policy_eval_config, temp_dir) if File.exist?(policy_eval_config) && !File.exist?(File.join(temp_dir, 'policy_evaluation.properties'))
            if node['DISP_ROOT'] != ''
                translator_config = File.join(node['DISP_ROOT'], 'Module', 'Translators', 'tonxlfile', 'bin', 'config.properties')
                FileUtils.cp_r(translator_config, temp_dir) if File.exist?(translator_config) && !File.exist?(File.join(temp_dir, 'config.properties'))
            end

            # Renew all file inside TcDRM folder
            log_info('Updating TcDRM file')
            FileUtils.rm_rf(tcdrm)
            FileUtils.mkdir_p(tcdrm) unless File.exist?(tcdrm)
            src = Dir[File.join(current_dir, 'TcDRM', '*')]
            src.each do |f|
                FileUtils.cp_r(f, tcdrm)
            end

            need_skydrm_config = false
            node['feature_list'].each do |feature|
                if (feature == 'partialdownload' && check_vis_server_installed(node['TC_ROOT'])) || feature == 'translator_rights_checker'
                    need_skydrm_config = true
                    break
                end
            end

            if need_skydrm_config && RUBY_PLATFORM =~ /mswin|mingw|windows/
                FileUtils.cp_r(File.join(temp_dir, 'config.properties'), general_config) if File.exist?(File.join(temp_dir, 'config.properties'))
            end

            # If feature have TRC, dispatcher_module or nxl_template -> need PDP information in properties file and preferences
            if node['feature_list'].include?('translator_rights_checker') || node['feature_list'].include?('dispatcher_module') || node['feature_list'].include?('nxl_template')
                log_info('Prepare Policy Controller configuration file')
                policy_config = File.join(ENV['RMX_ROOT'], 'config', 'policy_config.properties')
                file = File.new(policy_config, 'w+')
                file.puts 'nextlabs.pdp.engine.name=com.nextlabs.openaz.pdp.RestPDPEngine'
                file.puts 'nextlabs.cloudaz.payload_content_type=application/json'
                file.puts "nextlabs.cloudaz.host=#{node['jpc_remote_ip']}"
                file.puts "nextlabs.cloudaz.port=#{node['jpc_eva_port']}"
                file.puts "nextlabs.cloudaz.https=#{node['jpc_https']}"
                file.puts 'nextlabs.cloudaz.auth_type=OAUTH2'
                file.puts 'nextlabs.cloudaz.oauth2.grant_type=client_credentials'
                file.puts "nextlabs.cloudaz.oauth2.server=#{node['cc_host']}"
                file.puts "nextlabs.cloudaz.oauth2.port=#{node['cc_port']}"
                file.puts "nextlabs.cloudaz.oauth2.https=#{node['cc_oauth2_https']}"
                file.puts "nextlabs.cloudaz.oauth2.client_id=#{node['cc_apiuser']}"
                file.puts "nextlabs.cloudaz.oauth2.token_endpoint_path=/cas/token"
                file.puts "nextlabs.cloudaz.ignore_https_certificate=#{node['cc_ignore_https_cert']}"
                file.puts "xacml.pdpEngineFactory=com.nextlabs.openaz.pdp.PDPEngineFactoryImpl"
                file.puts "pep.mapper.classes=com.nextlabs.openaz.pepapi.RecipientMapper,com.nextlabs.openaz.pepapi.DiscretionaryPoliciesMapper,com.nextlabs.openaz.pepapi.HostMapper,com.nextlabs.openaz.pepapi.ApplicationMapper"
                file.close

                secret_config = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'secret.properties')
                secret_file = File.new(secret_config, 'w+')
                secret_file.puts "SECRET=#{node['cc_apisecret']}"
                secret_file.close

                if RUBY_PLATFORM =~ /mswin|mingw|windows/
                    encrypt_secret_password = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'encryptSecret.bat')
                    encrypt_secret_password_cmd = "\"#{encrypt_secret_password}\""
                else
                    encrypt_secret_password = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin', 'encryptSecret.sh')
                    grant_full_rights(encrypt_secret_password)
                    encrypt_secret_password_cmd = "\"#{encrypt_secret_password}\""
                end

                output = Open3.popen3(encrypt_secret_password_cmd) { |_stdin, stdout, _stderr, _wait_thr| stdout.read }
                log_info(output)

                # Override cc_apisercret with encrypted version
                node.override['cc_apisecret'] = get_secret_from_properties
            end
        else
            ENV['RMX_ROOT'] = File.join(node['install_dir'], 'TcDRM') if ENV['RMX_ROOT'].nil?
            ENV['RMX_ROOT'] = ENV['RMX_ROOT'].tr('\\', '/')
        end
    end
end
