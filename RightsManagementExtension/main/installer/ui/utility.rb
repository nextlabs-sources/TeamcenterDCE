#!/usr/bin/env ruby

# Teamcenter RMX 5.2 GUI Installation
# Component     :: Utility
# Description   :: GUI Utility function
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'json'
require 'resolv'
require 'tmpdir'
require 'fileutils'
require 'rexml/document'
require 'singleton'
# require 'win32/service' if RUBY_PLATFORM =~ /mswin|mingw|windows/
# require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/
# require 'win32ole' if RUBY_PLATFORM =~ /mswin|mingw|windows/

require_relative './registry_finder'
require_relative './validator'

# Helper for finding SolidWorks SWIM dir (swim_dir_in_registry)
# Reference: https://stackoverflow.com/questions/50713867/ruby-how-to-read-registry-keys-for-32-bit-and-64-bit-applications-from-32-bit
# if RUBY_PLATFORM =~ /mswin|mingw|windows/
#     module Win32::Registry::Constants
#         KEY_WOW64_64KEY = 0x0100
#         KEY_WOW64_32KEY = 0x0200
#     end
# end

module Utility
    MESSAGE = JSON.parse(IO.read(File.join(File.dirname(__FILE__), 'message_properties.json'), encoding: 'utf-8'))

    START_DIR = File.expand_path('..', __dir__)

    # FEATURE_PATH is the xml file for RMX feature description
    FEATURE_PATH = File.join(START_DIR, 'TcDRM', 'features.xml').tr('\\', '/')

    # define max version number
    MAX_VERSION_NUM = 99_999

    # Represent Configuration item in this (un)installation attempt
    class Item
        include Singleton
        include Validator
        include RegistryFinder

        attr_accessor :install_dir, :installation_mode, :solution_type,
                    :tc_root, :tc_data, :disp_root, :infodba_psd,
                    :creo_dir, :ipem_dir, :solidworks_dir, :swim_dir, :bb_dir, :autocad_dir, :catia_dir,
                    :jpc_remote_ip, :jpc_eva_port, :jpc_https,
                    :cc_host, :cc_port, :cc_oauth2_https, :cc_apiuser, :cc_apisecret, :cc_ignore_https_cert,
                    :skydrm_router_url, :skydrm_app_id, :skydrm_app_key,
                    :temp_dir, :certify_rmd, :feature_list

        DEPLOYMENT_COMPONENTS = %w(server_component dispatcher rac_extension).freeze

        # order by: server_component, dispatcher, rac_extension
        # same order as in feature.xml
        FEATURES = %w(
            nxl_template
            foundation_customization
            batch_protection
            workflow_initial_protection
            active_workspace_support
            scf_integration
            nxl_ipem_template
            nxl_swim_template
            dispatcher_module
            dispatcher_client_extension
            translator_rights_checker
            user_initial_protection
            rac_rmx
            proxy_runner
            partialdownload
            bb_integration
            nx_integration
            creo_integration
            solidworks_integration
            autocad_integration
            catia_integration
            rights_management_desktop
        ).freeze

        CAD_RMX = %w(
            nx_integration
            creo_integration
            solidworks_integration
            autocad_integration
            catia_integration
        )

        def initialize
            @temp_dir = 'C:/PROGRA~1/Nextlabs/tmp' if RUBY_PLATFORM =~ /mswin|mingw|windows/
            @temp_dir = '/usr/Nextlabs/tmp' if RUBY_PLATFORM =~ /linux/
            FileUtils.mkdir_p(@temp_dir) unless File.exist?(@temp_dir)

            config_file_path = if File.exist?(File.join(@temp_dir, 'rmx_properties_ui.json'))
                                   File.join(@temp_dir, 'rmx_properties_ui.json')
                               else
                                   File.join(File.dirname(__FILE__), 'default_config.json')
                               end

            begin
                external_config = JSON.parse(File.read(config_file_path))
            rescue Errno::ENOENT, JSON::ParserError
                external_config = nil
            end

            # default install_dir is platform dependent
            @install_dir = if !external_config.nil?
                               if config_file_path.end_with?('default_config.json')
                                   if RUBY_PLATFORM =~ /mswin|mingw|windows/
                                       external_config['install_dir']['windows']
                                   else
                                       external_config['install_dir']['linux']
                                   end
                               else
                                   external_config['install_dir']
                               end
                           else
                               ''
                           end
            # If using rmx_properties_ui.json as config, no need to check TC_ROOT and TC_DATA (this file usually represent a previous config)
            @tc_root = external_config ? external_config['tc_root'] : ''
            @tc_root = ENV['TC_ROOT'].nil? ? '' : ENV['TC_ROOT'] if @tc_root == '' && !config_file_path.end_with?('rmx_properties_ui.json')
            @tc_data = external_config ? external_config['tc_data'] : ''
            @tc_data = ENV['TC_DATA'].nil? ? '' : ENV['TC_DATA'] if @tc_data == '' && !config_file_path.end_with?('rmx_properties_ui.json')
            @creo_dir = external_config ? external_config['creo_dir'] : ''
            @creo_dir = creo_dir_in_registry if @creo_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @ipem_dir = external_config ? external_config['ipem_dir'] : ''
            @ipem_dir = ipem_dir_in_registry if @ipem_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @solidworks_dir = external_config ? external_config['solidworks_dir'] : ''
            @solidworks_dir = solidworks_dir_in_registry if @solidworks_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @swim_dir = external_config ? external_config['swim_dir'] : ''
            @swim_dir = swim_dir_in_registry if @swim_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @autocad_dir = external_config ? external_config['autocad_dir'] : ''
            @autocad_dir = autocad_dir_in_registry if @autocad_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @catia_dir = external_config ? external_config['catia_dir'] : ''
            @catia_dir = catia_dir_in_registry if @catia_dir == '' && !config_file_path.end_with?('rmx_properties_ui.json') && RUBY_PLATFORM =~ /mswin|mingw|windows/
            @bb_dir                     = external_config ? external_config['bb_dir']               : ''
            @disp_root                  = external_config ? external_config['disp_root']            : ''
            @infodba_psd                = ''
            @solution_type              = external_config ? external_config['solution_type']        : ''
            @jpc_remote_ip              = external_config ? external_config['jpc_remote_ip']        : ''
            @jpc_eva_port               = external_config ? external_config['jpc_eva_port']         : '58080'
            @jpc_https                  = external_config ? external_config['jpc_https']            : false
            @installation_mode          = external_config ? external_config['installation_mode']    : 'install'
            @skydrm_router_url          = external_config ? external_config['skydrm_router_url']    : ''
            @skydrm_app_id              = external_config ? external_config['skydrm_app_id']        : ''
            @skydrm_app_key             = ''
            @cc_host                    = external_config ? external_config['cc_host']              : ''
            @cc_port                    = external_config ? external_config['cc_port']              : ''
            @cc_oauth2_https            = external_config ? external_config['cc_oauth2_https']      : true
            @cc_apiuser                 = external_config ? external_config['cc_apiuser']           : ''
            @cc_apisecret               = ''
            @cc_ignore_https_cert       = external_config ? external_config['cc_ignore_https_cert'] : true
            @certify_rmd                = external_config ? external_config['certify_rmd']          : '9999'
            @feature_list               = external_config ? external_config['feature_list'] : []
        end

        # return number of features in certain deployment type (server, dispatcher, client)
        def get_deployment_type_count(deployment_type)
            file = File.new(FEATURE_PATH)
            doc = REXML::Document.new(file)

            dep_elements = doc.elements.to_a('features/deployment')

            dep_elements.each do |element|
                return element.attributes['count'] if element.attributes['name'].eql?(deployment_type)
            end
            raise("#{__method__}: Unable to find this deployment type: #{deployment_type}")
        end

        # get the teamcenter dependency feature of a certain RMX feature
        def get_tc_dep_of_feature(feature)
            file = File.new(FEATURE_PATH)
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
            raise("#{__method__}: Unable to find this feature: #{feature}")
        end

        # check if the feature is available on volume or download
        def get_feature_available_solution(feature)
            file = File.new(FEATURE_PATH)
            doc = REXML::Document.new(file)

            feature_elements = doc.elements.to_a('features/deployment/feature')
            feature_elements.each do |element|
                return element.elements.to_a('available_solution')[0].text if element.attributes['name'].eql?(feature)
            end
            raise("#{__method__}: Unable to find this feature: #{feature}")
        end

        # check teamcenter feature exist? base on ID
        def check_teamcenter_feature(id)
            return false if @tc_root == ''
            return true if id.nil? || id.eql?('')

            path = File.join(@tc_root, 'install', 'configuration.xml')
            file = File.new(path)
            doc = REXML::Document.new(file)
            doc.elements.each('*/config/features/installed') do |element|
                return true if id.include?(element.attributes['feature'])
            end
            false
        end

        # Check Active Workspace Server version. Return first decimal: 2.4, 3.2, 3.3 ,...
        def aws_version
            return MAX_VERSION_NUM if @tc_root == ''

            media_file = File.join(@tc_root, 'media_teamcenter_activeworkspace.xml')
            xml_media_file = REXML::Document.new(File.new(media_file))
            awc_version_with_dot = xml_media_file.elements.to_a('media/version')[0].text

            version = awc_version_with_dot[0..2]
            version.to_f
        end

        # Check Teamcenter Full Version number. 10.1.4.2 will return 10142, 11.5 will return 11500,...
        def teamcenter_full_version
            return MAX_VERSION_NUM.to_s if @tc_root == ''

            media_file = File.join(@tc_root, 'media_teamcenter_foundation.xml')
            xml_media_file = REXML::Document.new(File.new(media_file))
            tc_version_with_dot = xml_media_file.elements.to_a('media/version')[0].text

            version = tc_version_with_dot.tr('.', '')
            version += '0' while version.length < 5
            version
        end

        # Check during upgrade, template need to reinstall?
        def check_template_upgrade
            upgrade_config_file = File.join(START_DIR, 'ui', 'upgrade_config.json')
            upgrade_config = JSON.parse(File.read(upgrade_config_file))
            features_xml_installed = File.join(ENV['RMX_ROOT'], 'features.xml')
            rmx_version_installed = get_tcrmx_version(features_xml_installed)
            rmx_version_installed = '5.0' if rmx_version_installed.start_with?('5.0')

            ENV['template_upgrade'] = upgrade_config['template_upgrade'][rmx_version_installed]
        end

        # Check Teamcenter version that support Partialdownload feature: >11500 (11.5.0.0 and above), >11600, all 12..
        def can_install_partialdownload?
            tc_version = teamcenter_full_version
            tc_version.start_with?('115', '116', '12', '13')
        end
        
        def can_install_partialdownload_server?
            return false unless can_install_partialdownload?
            
            path = File.join(@tc_root, 'install', 'configuration.xml')
            return false unless File.exist?(path)
            file = File.new(path)
            doc = REXML::Document.new(file)
            doc.elements.each('*/config/features/installed') do |element|
                return true if element.attributes['feature'] == 'VDR04GFQPHUG7SBBS91XYV1E1A41UJ6Y'
            end
            false
        end

        # Check Teamcenter version that support RAC RMX feature: >11600 (11.6.0.0 and above), >12200
        def can_install_rac_rmx?
            tc_version = teamcenter_full_version
            tc_version.start_with?('116', '122', '123', '124', '13')
        end

        # Check if we are upgrading TcRMX?
        def compare_tcrmx_version
            return 0 if ENV['RMX_ROOT'].nil?
            # Find Tc RMX version in RMX_ROOT\features.xml
            features_xml_installed = File.join(ENV['RMX_ROOT'], 'features.xml')
            rmx_version_installed = get_tcrmx_version(features_xml_installed)
            ENV['OLD_RMX_VERSION'] = rmx_version_installed.to_s

            features_xml_current = FEATURE_PATH
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

        def is_cad_rmx_include?(cad_app_name)
            tcdrm_folder = File.join(START_DIR, 'TcDRM')
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
            unless RUBY_PLATFORM =~ /mswin|mingw|windows/
                return false
            end
            if can_install_partialdownload_server?
                return false
            end
            
            return can_install_rmd_with_version?
        end

        def can_install_rmd_with_version?
            unless RUBY_PLATFORM =~ /mswin|mingw|windows/
                return false
            end
            is_rmd_installed, existing_version = get_rmd_existing_version()

            if !is_rmd_installed
                return true
            elsif existing_version.to_i < @certify_rmd.to_i
                return true
            else
                return false
            end
        end

        # Generate pretty_json for the feature list
        def to_json(*fields_to_nil)
            hash = {}

            ignore_parameters = %w(@infodba_psd @skydrm_app_key @cc_apisecret)

            instance_variables.each do |var|
                hash[var.to_s.delete('@')] = instance_variable_get(var) unless ignore_parameters.include?(var.to_s)
            end

            fields_to_nil.each do |field|
                hash[field] = nil if hash.key?(field)
            end

            JSON.pretty_generate(hash)
        end

        # Save the config properties file to temp dir
        def save_config_to_temp_dir
            temp_config_path = File.join(@temp_dir, 'rmx_properties.json')
            File.open(temp_config_path, 'w') do |file|
                file.write(to_json)
            end
            temp_config_path
        end

        # Validate user input
        def validate_input(type, *input)
            case type
            when :tc_data
                validate_tc_data(*input)
            when :tc_root
                validate_tc_root(*input)
            when :disp_root
                validate_disp_root(*input)
            when :infodba_psd
                valid, error_msg = validate_non_empty(*input[2])
                return validate_infodba_password(*input) if valid

                [valid, error_msg]
            when :jpc_eva_port
                validate_port(*input)
            when :install_dir
                validate_install_dir(*input)
            when :creo_dir
                validate_creo_dir(*input)
            when :ipem
                validate_ipem_dir(*input)
            when :solidworks_dir
                validate_solidworks_dir(*input)
            when :swim
                validate_swim_dir(*input)
            when :autocad_dir
                validate_autocad_dir(*input)
            when :catia_dir
                validate_catia_dir(*input)
            when :bb_dir
                validate_bb_dir(*input)
            when :disk_space
                validate_disk_space(*input)
            when :skydrm_connection
                validate_skydrm_connection(*input)
            when :cc_connection
                validate_cc_connection(*input)
            when :cc_api
                validate_cc_api(*input)
            when :jpc_connection
                validate_jpc_connection(*input)
            else
                validate_non_empty(*input)
            end
        end

        # Validate environment requirement
        def validate_environment(type)
            case type
            when :java
                validate_java_env
            when :fms_home
                validate_fms_home
            else
                puts('Unknown type')
            end
        end
    end

    # Represent Installation state flag for installer
    class InstallationFlag
        include Singleton
        attr_reader :installation_canceled, :installation_finished, :installation_failed, :install_cmd_exited
        attr_writer :installation_canceled, :installation_finished, :installation_failed, :install_cmd_exited

        def initialize
            @installation_canceled = false
            # A flag for identify the installation finished
            @installation_finished = false
            # A flag for identify the installation failed
            @installation_failed = false

            @install_cmd_exited = false
        end
    end
end

# Dir[File.expand_path(File.dirname(__FILE__)) + '/libraries/*.rb'].each {|file|
#   require file
# }
