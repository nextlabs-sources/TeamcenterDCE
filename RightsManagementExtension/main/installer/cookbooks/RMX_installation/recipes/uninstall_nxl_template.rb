# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_nxl_template
# Description   :: Remove nextlabs tc preferences
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'remove_tc_preferences' do
    block do
        log_info(LOGMSG::UNINSTALL_NXL_TEMPLATE)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Remove NextLabs Teamcenter preferences...')
        # get nextlabs tc preference names
        nxl_tc_preference = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'NXL_TcPreferences.xml')
        file = File.new(nxl_tc_preference)
        doc = REXML::Document.new(file)
        preference_elements = doc.elements.to_a('preferences/category/preference')
        preference_names = ''

        preference_elements.each do |element|
            preference_names += element.attributes['name'] + ','
        end

        nxl_tc_preference_merge = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'NXL_TcPreferences.merge.xml')
        file = File.new(nxl_tc_preference_merge)
        doc = REXML::Document.new(file)
        preference_elements = doc.elements.to_a('preferences/category/preference')
        # only one TC preference in this Document
        preference_names += preference_elements[0].attributes['name']

        # perferences_manager will update a log, have to use \dir\subdir\ style, cannot use /dir/subdir
        manager = File.join(node['TC_ROOT'], 'bin', TeamcenterProcess::PREFERENCES_MANAGER)
        manager = manager.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        step = 'Remove NextLabs Teamcenter preference'
        preference_manager_cmd = "#{node['tc_profilevar']} && \"#{manager}\" -mode=delete -scope=SITE -preferences=#{preference_names} -u=infodba -p=#{node['infodba_psd']} -g=dba"
        call_system_command(step, 'preference_manager', preference_manager_cmd, :no)
    end
end