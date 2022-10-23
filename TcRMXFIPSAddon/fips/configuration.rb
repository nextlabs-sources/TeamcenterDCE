require 'singleton'
require 'json'

# Represent Configuration variable of TcRMX
class Configuration
    include Singleton
    attr_accessor :install_dir, :tc_root, :disp_root, :feature_list

    def initialize
        @temp_dir = RUBY_PLATFORM =~ /mswin|mingw|windows/ ? 'C:/PROGRA~1/Nextlabs/tmp' : '/usr/Nextlabs/tmp'

        existing_config_file = File.join(@temp_dir, 'rmx_properties_ui.json')
        if !File.exist?(existing_config_file)
            puts('Not found existing config file (rmx_properties_ui.json). Program closing.')
            exit(0)
        end

        puts('Retrieving TcRMX configuration.')
        config = JSON.parse(File.read(existing_config_file))

        @install_dir = config ? config['install_dir'] : ''
        @tc_root = config ? config['tc_root'] : ''
        @disp_root = config ? config['disp_root'] : ''
        @feature_list = config ? config['feature_list'] : []
    end
end