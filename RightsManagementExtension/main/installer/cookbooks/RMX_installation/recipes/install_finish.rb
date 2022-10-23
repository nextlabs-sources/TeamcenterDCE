# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_finish
# Description   :: Put .txt file indicate RMX is installed. Create RMX registry
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
require 'json'
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/

ruby_block 'install_finish' do
    block do
        FileUtils.mkdir_p(node['temp_dir']) unless File.exist?(node['temp_dir'])
        file_path = File.join(node['temp_dir'], InstallFile::HAS_INSTALLED_TXT)
        File.new(file_path, 'w+')
        FileUtils.remove_file(File.join(node['temp_dir'], 'break_point.json'), true)

        # Bug 56544: Update rmx_properties_ui.json during silence install (if this file exist due to previous UI installation)
        rmx_properties_ui = File.join(node['temp_dir'], 'rmx_properties_ui.json')
        if File.exist?(rmx_properties_ui) && node['silence_install'] == 'yes'
            log_info('Updating rmx_properties_ui.json to include new feature.')
            config = JSON.parse(File.read(rmx_properties_ui))
            node['feature_list'].each do |feature|
                unless config['feature_list'].include?(feature)
                    config['feature_list'].push(feature)
                    config[feature] = 'ON'
                end
            end

            File.open(rmx_properties_ui, 'w') do |file|
                file.write(JSON.pretty_generate(config))
            end
        # Bug 66230: Addition rmx_properties_ui.json file when silence install finish to support FIPS SDK deployment if needed
        elsif !File.exist?(rmx_properties_ui) && node['silence_install'] == 'yes'
            log_info('Make a copy of rmx_properties_ui.json to NextLabs tmp dir.')
            rmx_package_folder = ENV['START_DIR']
            silence_config = File.join(rmx_package_folder, 'silence', 'rmx_properties_ui.json')
            config = JSON.parse(File.read(silence_config))

            ignore_parameters = %w(infodba_psd skydrm_app_key cc_apisecret)
            ignore_parameters.each do |field|
                config[field] = ''
            end
            config['feature_list'] = node['feature_list']

            Installer::RMX_FEATURES.each do |rmx_feature|
                config.delete(rmx_feature)
            end

            File.new(rmx_properties_ui, 'w+')
            File.open(rmx_properties_ui, 'w') do |file|
                file.write(JSON.pretty_generate(config))
            end
        end

        log_info(LOGMSG::INSTALL_FINISHED)
    end
end
