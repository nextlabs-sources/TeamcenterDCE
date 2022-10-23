# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_translator_rights_checker
# Description   :: Move necessary translator rights checker .jar file into shared folder
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_translator_rights_checker' do
    block do
        log_info(LOGMSG::INSTALL_TRANSLATOR_RIGHTS_CHECKER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Overwritting policy evaluation configuration...')
        trans_rights_checker = File.join(ENV['RMX_ROOT'], 'TranslatorRightsChecker')
        # Copy policy_config properties to TranslatorRightsChecker folder
        FileUtils.cp_r(File.join(ENV['RMX_ROOT'], 'config', 'policy_config.properties'), trans_rights_checker)

        path = File.join(trans_rights_checker, 'policy_evaluation.properties')
        file = File.new(path, 'w+')
        file.puts 'DISP_USER_NAME=dcproxy'
        file.puts 'NXL_EVALUATION_ATTRIBUTES=ip_classification,gov_classification'
        file.puts 'NXL_PDP_DEFAULT_ACTION=deny'
        file.puts 'NXL_PDP_DEFAULT_MESSAGE=pdp connection timeout'
        file.close
    end
end
