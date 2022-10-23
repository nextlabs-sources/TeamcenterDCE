# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_translator_rights_checker
# Description   :: Update rightschecker jar
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_translator_rights_checker' do
    block do
        log_info(LOGMSG::UPGRADE_TRANSLATOR_RIGHTS_CHECKER)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        FileUtils.cp_r(File.join(node['temp_dir'], 'policy_evaluation.properties'), File.join(ENV['RMX_ROOT'], 'TranslatorRightsChecker'))
    end
end
