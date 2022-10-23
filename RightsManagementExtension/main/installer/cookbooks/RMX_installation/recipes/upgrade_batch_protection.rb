# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_batch_protection
# Description   :: Rewritting connection config
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_batch_protection' do
    block do
        log_info(LOGMSG::UPGRADE_BATCH_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        FileUtils.cp_r(File.join(node['temp_dir'], 'connection.properties'), File.join(ENV['RMX_ROOT'], 'BatchProtect'))
    end
end
