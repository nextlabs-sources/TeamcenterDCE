# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_batch_protection
# Description   :: Overwritting connection config
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_batch_protection' do
    block do
        log_info(LOGMSG::INSTALL_BATCH_PROTECTION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        log_info('Overwritte connection configuration')
        path = File.join(ENV['RMX_ROOT'], 'BatchProtect', 'connection.properties')
        file = File.new(path, 'w+')
        file.puts 'TC_SOA_HOST=<SOA_HOST>'
        file.puts 'TC_USER_NAME=<TC_USER>'
        file.puts 'TC_USER_PASSWORD=<PASSWORD>'
        file.puts 'FORCE_UNCHECKOUT=false'
        file.close
    end
end
