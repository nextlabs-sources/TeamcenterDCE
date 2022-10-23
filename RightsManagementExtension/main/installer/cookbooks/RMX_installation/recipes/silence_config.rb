# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: Silence Config
# Description   :: Read json configuration file and validate feature_list to (un)install
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'json'
Chef::Recipe.send(:include, Compile::Config)
Chef::Recipe.send(:include, Utility)

can_proceed_silence_config

log_info('[Progress] Configuring silence installer setup...')

def feature_select?(component, tc_dep, available_solution, deployment_index)
    return false unless node[component] == 'ON'

    to_select = true
    to_select = false if (RUBY_PLATFORM =~ /linux/) && deployment_index == 2 # Do not install client features on linux server
    to_select = false unless available_solution.include?(node['solution_type'])

    if component.eql?('nxl_template') || component.eql?('foundation_customization') || component.eql?('batch_protection')
        to_select = false if !teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || node['TC_DATA'] == ''
    elsif component.eql?('active_workspace_support')
        to_select = false if !teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || get_aws_version(node['TC_ROOT']) < 4.3
    elsif component.eql?("scf_integration")
    	to_select = false if (!teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || !node['tc_full_version'].start_with?('11') || RUBY_PLATFORM =~ /linux/)      # not install SCF on Linux
    elsif component.eql?('dispatcher_module') || component.eql?('translator_rights_checker') || component.eql?('dispatcher_client_extension')
        to_select = false if !teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || node['DISP_ROOT'] == '' || !(FileTest.directory? node['DISP_ROOT'])
    elsif component.eql?('nx_integration')
        ugii_root = ENV['UGII_ROOT_DIR']
        ugii_root = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root.nil? && !ENV['UGII_BASE_DIR'].nil?
        puts("NX RMX include? : #{is_cad_rmx_include?("NX")}")
        to_select = false if ugii_root.nil? || !File.exist?(ugii_root) || !is_cad_rmx_include?("NX")
    elsif component.eql?('partialdownload')
        to_select = false if !(FileTest.directory? File.join(node['TC_ROOT'], 'tccs')) || !teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || !can_install_partialdownload?(node['tc_full_version'])
    elsif component.eql?('rac_rmx')
        to_select = false if !teamcenter_dependency_checked(node['TC_ROOT'], tc_dep) || !can_install_rac_rmx?(node['tc_full_version'])
    elsif component.eql?('creo_integration')
        creo_common_files = File.expand_path('../Common Files', node['creo_dir'])
        puts("Creo RMX include? : #{is_cad_rmx_include?("Creo")}")
        to_select = false if node['creo_dir'] == '' || !FileTest.directory?(creo_common_files) || !is_cad_rmx_include?("Creo")
    elsif component.eql?('solidworks_integration')
        solidworks_exe = File.join(node['solidworks_dir'], 'SOLIDWORKS', 'SLDWORKS.exe')
        puts("SolidWorks RMX include? : #{is_cad_rmx_include?("SolidWorks")}")
        to_select = false if node['solidworks_dir'] == '' || !File.exist?(solidworks_exe) || !is_cad_rmx_include?("SolidWorks")
    elsif component.eql?('autocad_integration')
        autocad_exe = File.join(node['autocad_dir'], 'acad.exe')
        puts("AutoCAD RMX include? : #{is_cad_rmx_include?("AutoCAD")}")
        to_select = false if node['autocad_dir'] == '' || !File.exist?(autocad_exe) || !is_cad_rmx_include?("AutoCAD")
    elsif component.eql?('catia_integration')
        puts("CATIA RMX include? : #{is_cad_rmx_include?("CATIA")}")
        to_select = false if node['catia_dir'] == ''
    elsif component.eql?("bb_integration")
        ugii_root = ENV['UGII_ROOT_DIR']
        if ugii_root == nil && ENV['UGII_BASE_DIR'] != nil
            ugii_root = File.join(ENV['UGII_BASE_DIR'], "UGII")
        end
        to_select = false if ugii_root == nil || !File.exist?(ugii_root) || node['bb_dir'] == "" || !(FileTest.directory? node['bb_dir'])
    elsif component.eql?("rights_management_desktop")
        to_select = false if !can_install_rmd? || node['installation_mode'].to_s.strip.eql?('remove')
    else
        to_select = false unless teamcenter_dependency_checked(node['TC_ROOT'], tc_dep)
    end
    to_select
end

# parameters definition
feature_path = File.join(ENV['START_DIR'], 'TcDRM', 'features.xml').tr('\\', '/')
dep_components = %w(server_component dispatcher rac_extension)

# ReadableNames = JSON.parse(File.read(File.join(ENV['START_DIR'], 'ui', 'message_properties.json'), :encoding => "utf-8"))

feature_num = []

# update feature_num
dep_components.each do |component|
    num = get_deployment_type_count(feature_path, component)
    feature_num.push(num.to_i)
end

start_num = 0
(0...dep_components.size).each do |deployment_index|
    end_num = start_num + feature_num[deployment_index]
    (start_num...end_num).each do |feature_index|
        component = Installer::RMX_FEATURES[feature_index]
        tc_dep = get_tc_dep_of_feature(feature_path, component)
        available_solution = get_feature_available_solution(feature_path, component)
        node['feature_list'].push(component) if feature_select?(component, tc_dep, available_solution, deployment_index)
    end
    start_num = end_num
end

log_info('(**) List of Feature will be processed: ')
log_info(node['feature_list'])
# log_info('NextLabs Rights Management Desktop will NOT be removed together with Rights Management Extension.')