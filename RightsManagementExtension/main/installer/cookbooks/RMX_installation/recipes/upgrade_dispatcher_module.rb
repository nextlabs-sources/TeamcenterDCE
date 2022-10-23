# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_dispatcher_module
# Description   :: Adding tonxlfile translator module to Dispatcher Module
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_dispatcher_module' do
    block do
        log_info(LOGMSG::UPGRADE_DISPATCHER_MODULE)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        disp_module_dir = File.join(node['DISP_ROOT'], 'Module')
        if File.exist?(disp_module_dir)
            log_info('Replace NextLabs Translator module.')
            tonxlfile_dir = File.join(disp_module_dir, 'Translators', 'tonxlfile')
            tonxlfile_bin_dir = File.join(tonxlfile_dir, 'bin')
            FileUtils.rm_rf(tonxlfile_bin_dir)      # Cleanup tonxlfile folder
            FileUtils.mkdir_p(tonxlfile_bin_dir)
            src_dir = File.join(ENV['RMX_ROOT'], 'Translator', 'tonxlfile', 'bin')
            copy_files(src_dir, tonxlfile_bin_dir)

            FileUtils.cp_r(File.join(node['temp_dir'], 'config.properties'), src_dir) if File.exist?(File.join(node['temp_dir'], 'config.properties'))
        end
    end
end

