# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_dispatcher_module
# Description   :: Remove tonxlfile translator module
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_disp_module' do
    block do
        log_info(LOGMSG::UNINSTALL_DISPATCHER_MODULE)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        # Remove %DISP_ROOT%/Module/Translators/tonxlfile
        log_info('Remove tonxlfile folder...')
        tonxlfile_dir = File.join(node['DISP_ROOT'], 'Module', 'Translators', 'tonxlfile')
        FileUtils.remove_dir(tonxlfile_dir, true)

        log_info('Remove tonxlfile module from translator.xml...')
        filepath = File.join(node['DISP_ROOT'], 'Module', 'conf', 'translator.xml')
        delete_xml_element(filepath, 4, 'Translators/ToNxlFile')

        # remove tonxlfile service from Dispatcher AdminClientUI.xml (if this file is exist - with relavent feature install)
        filepath = File.join(node['DISP_ROOT'], 'AdminClient', 'ui', 'swing', 'AdminClientUI.xml')
        if File.exist?(filepath) && regexp?(filepath, /.*tonxlfile.*/)
            log_info('Remove tonxlfile service from Dispatcher AdminClientUI.xml...')
            delete_xml_element(filepath, 4, "AdminClient/ProviderMap/Provider[@name='NEXTLABS']")
        end
    end
end