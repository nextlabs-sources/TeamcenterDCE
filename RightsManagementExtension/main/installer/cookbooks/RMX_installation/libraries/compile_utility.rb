# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Library       :: Configuration Utility Library
# Description   :: Contains configuration method for silence installer and bootstrapping (CHEF COMPILE RUN TIME)
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

module Compile
    # Configuration for Bootstrap and Silence config
    module Config
        require 'fileutils'
        require 'rexml/document'
        require 'open3'

        def convert_path_to_unix_style(path)
            path.nil? ? '' : path.strip.tr('\\', '/').gsub('Program Files (x86)', 'PROGRA~2').gsub('Program Files', 'PROGRA~1')
        end

        def get_deployment_type_count(feature_path, dep)
            file = File.new(feature_path)
            doc = REXML::Document.new(file)
            dep_elements = doc.elements.to_a('features/deployment')
            dep_elements.each do |element|
                if element.attributes['name'].eql?(dep)
                    return element.attributes['count']
                end
            end
            raise("Cannot find this deployment type: #{dep}")
        end

        def check_teamcenter_feature(tc_root, id)
            path = File.join(tc_root, 'install', 'configuration.xml')
            file = File.new(path)
            doc = REXML::Document.new(file)
            doc.elements.each('*/config/features/installed') do |element|
                return true if id.include?(element.attributes['feature'])
            end
            false
        end

        def get_tc_dep_of_feature(feature_path, feature)
            file = File.new(feature_path)
            doc = REXML::Document.new(file)
            tc_dep = []

            feature_elements = doc.elements.to_a('features/deployment/feature')
            feature_elements.each do |element|
                next unless element.attributes['name'].eql?(feature)

                tc_dep_elements = element.elements.to_a('teamcenter_dependency/tc_feature')
                tc_dep_elements.each do |ele|
                    tc_dep.push(ele.attributes['id'])
                end
                return tc_dep
            end
            raise("Cannot find this feature: #{feature}")
        end

        def teamcenter_dependency_checked(tc_root, tc_dep_array)
            return false if tc_root == ''

            tc_dep_array.each do |id|
                return false unless check_teamcenter_feature(tc_root, id)
            end
            true
        end

        def get_feature_available_solution(feature_path, feature)
            file = File.new(feature_path)
            doc = REXML::Document.new(file)

            feature_elements = doc.elements.to_a('features/deployment/feature')
            feature_elements.each do |element|
                if element.attributes['name'].eql?(feature)
                    return element.elements.to_a('available_solution')[0].text
                end
            end
            raise("Cannot find this feature: #{feature}")
        end

        def get_aws_version(tc_root)
            return node['MAX_VERSION_NUM'] if tc_root == ''

            media_file = File.join(tc_root, 'media_teamcenter_activeworkspace.xml')
            xml_media_file = REXML::Document.new(File.new(media_file))
            awc_version_with_dot = xml_media_file.elements.to_a('media/version')[0].text

            version = awc_version_with_dot[0..2]
            version.to_f
        end

        def can_install_partialdownload?(tc_version)
            # supportedVersion = >'11500', >'11600', >'12100'
            tc_version.start_with?('115', '116', '12', '13')
        end

        def get_teamcenter_full_version(tc_root)
            return node['MAX_VERSION_NUM'] if tc_root == ''

            media_file = File.join(tc_root, 'media_teamcenter_foundation.xml')
            xml_media_file = REXML::Document.new(File.new(media_file))
            tc_version_with_dot = xml_media_file.elements.to_a('media/version')[0].text

            version = tc_version_with_dot.tr('.', '')
            version += '0' while version.length < 5
            version
        end

        def get_break_point(tmp_dir)
            break_point_json = File.join(tmp_dir, 'break_point.json')
            return 'none' unless File.exist?(break_point_json)

            break_point = JSON.parse(IO.read(break_point_json, encoding: 'utf-8'))
            break_point['break_point']
        end

        # Check during upgrade, template need to reinstall?
        def check_template_upgrade
            upgrade_config_file = File.join(ENV['START_DIR'], 'ui', 'upgrade_config.json')
            upgrade_config = JSON.parse(File.read(upgrade_config_file))
            features_xml_installed = File.join(ENV['RMX_ROOT'], 'features.xml')
            rmx_version_installed = get_tcrmx_version(features_xml_installed)
            rmx_version_installed = '5.0' if rmx_version_installed.start_with?('5.0')

            ENV['template_upgrade'] = upgrade_config['template_upgrade'][rmx_version_installed]
        end

        # Find old version of TcRMX
        def tcrmx_old_version
            return if ENV['RMX_ROOT'].nil?
            features_xml_installed = File.join(ENV['RMX_ROOT'], 'features.xml')
            rmx_version_installed = get_tcrmx_version(features_xml_installed)
            ENV['OLD_RMX_VERSION'] = rmx_version_installed.to_s
        end

        # Check if we allow to install TcRMX
        def can_install_rmx?
            !File.exist?(File.join(node['temp_dir'], 'has_installed.txt'))
        end

        # Check if we allow to uninstall TcRMX
        def can_uninstall_rmx?
            File.exist?(File.join(node['temp_dir'], 'has_installed.txt')) && compare_tcrmx_version.zero?
        end

        # Check if we allow to upgrade TcRMX
        def can_upgrade_rmx?
            File.exist?(File.join(node['temp_dir'], 'has_installed.txt')) && compare_tcrmx_version == 1
        end

        # Check if we are upgrading TcRMX?
        def compare_tcrmx_version
            return 0 if ENV['RMX_ROOT'].nil?
            # Find Tc RMX version in RMX_ROOT\features.xml
            features_xml_installed = File.join(ENV['RMX_ROOT'], 'features.xml')
            rmx_version_installed = get_tcrmx_version(features_xml_installed)

            features_xml_current = File.join(ENV['START_DIR'], 'TcDRM', 'features.xml')
            rmx_version_current = get_tcrmx_version(features_xml_current)

            return 1 if rmx_version_installed.to_s < rmx_version_current.to_s
            return 0 if rmx_version_installed.to_s == rmx_version_current.to_s
            -1
        end

        # Get Teamcenter RMX version base on feature file
        def get_tcrmx_version(features_file)
            xml_doc = REXML::Document.new(File.new(features_file))
            overall_feature = xml_doc.elements.to_a('features')
            overall_feature[0].attributes['fullVersion']
        end

        def can_proceed_silence_config
            if node['installation_mode'] == 'remove' && !can_uninstall_rmx?
                err_msg = 'Silence uninstallation abort. RMX is not installed before.'
                raise(err_msg)
            elsif node['installation_mode'] == 'upgrade' && !can_upgrade_rmx?
                err_msg = 'Silence upgrade abort. No old RMX version detected'
                raise(err_msg)
            end
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

        def can_install_rac_rmx?(tc_full_version)
            tc_full_version.start_with?('116', '12', '13')
        end

        def can_install_partialdownload_server?
            path = File.join(node['TC_ROOT'], 'install', 'configuration.xml')
            file = File.new(path)
            doc = REXML::Document.new(file)
            doc.elements.each('*/config/features/installed') do |element|
                return true if element.attributes['feature'] == 'VDR04GFQPHUG7SBBS91XYV1E1A41UJ6Y'
            end
            false
        end

        def is_cad_rmx_include?(cad_app_name)
            tcdrm_folder = File.join(ENV['START_DIR'], 'TcDRM').tr('\\', '/')
            # msi_installer = "NextLabs Rights Management For #{cad_app_name}.msi"
            msi_installers = File.join(tcdrm_folder, "*.msi")
            Dir[msi_installers].each do |file|
                name = File.basename(file)
                if name.downcase.include?(cad_app_name.downcase)
                    return true
                end
            end
            return false
        end

        def can_install_rmd?
            if can_install_partialdownload_server?
                return false
            end
            is_rmd_installed, existing_version = get_rmd_existing_version()

            if !is_rmd_installed
                return true
            elsif existing_version.to_i < node['certify_rmd'].to_i
                return true
            else
                return false
            end
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
end
