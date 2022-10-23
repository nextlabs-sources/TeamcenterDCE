#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Installation
# Description   :: Render installation page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'
# include Utility
require 'tmpdir'
require 'fileutils'

# to stop the installation half way, we need to send SIGINT to the subprocess,
# but the chef-client spawns other processes, so we must send SIGINT to the process group
# which means the father process will also receive the SIGINT
# so we need to ignore here
trap('INT') {}

class Installation < Shoes
    include Utility
    url('/install', :install)

    def install
        style(Link, underline: false, stroke: '#FFF', weight: 'bold')
        style(LinkHover, underline: false, stroke: '#FFF', weight: 'bold')

        stack @installer_page_size_style do
            # The header area
            stack @installer_header_size_style do
                background @header_color
                banner
            end

            # The progress area
            stack width: 1.0 do
                background @content_color

                # a vertical spacer
                stack width: 1.0, height: 60 do
                    para ' '
                end

                flow width: 1.0, height: 50 do

                    # a horizontal spacer
                    stack width: 80, height: 1.0 do
                        para ' '
                    end
                    # the progress message
                    stack width: 800, height: 1.0 do
                        @progress_msg = inscription ' '
                        @warning_msg = inscription ' '
                    end
                end

                @progress = progress width: 800, left: 80, top: 130
            end

            # The log area
            flow width: 1.0, height: 315 do
                stack width: 80, height: 1.0 do
                    background @content_color
                    para ' '
                end

                stack width: 800, height: 1.0 do
                    @log_area = edit_box width: 1.0, height: 1.0, state: 'disabled'
                end

                stack width: 80, height: 1.0 do
                    background @content_color
                    para ' '
                end
            end

            # A vertical spacer
            stack width: 1.0, height: 35 do
                background @content_color
                para ' '
            end


            # The footer area
            stack @installer_footer_size_style do
                background @footer_color

                # This is a vertical spacer
                stack width: 1.0, height: 15 do
                    para ' '
                end

                flow width: 1.0, height: 50 do
                    # This is a horizontal spacer
                    stack width: 10, height: 1.0 do
                        para ' '
                    end

                    # The installation page has no back btn
                    stack width: 100, height: 1.0 do
                        para ' '
                    end

                    @install_cancel_btn_slot = stack width: 100, height: 1.0 do
                        nl_button :cancel
                    end
                    # Hide the Cancel functionality now
                    @install_cancel_btn_slot.hide

                    # This is a horizontal spacer
                    stack width: 180, height: 1.0 do
                        para ''
                    end

                    next_btn_click_proc = proc {
                        # we need to check the installation process
                        if @flag.install_cmd_exited
                            # First we need move the progress bar
                            @progress.move(0, -200)
                            visit(wizard(:next))
                        end
                    }

                    stack width: 180, height: 1.0 do
                        @install_next_btn = nl_button :next, state: 'disabled'
                        @install_next_btn.click { next_btn_click_proc.call }
                    end
                end
            end
        end

        # Make the (Un)installation call
        # ***PROGRESS***: Save Config file
        @progress.fraction = 0.05
        @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_save_config'], @progress_msg_color))

        # Generate rmx_properties.json file to TEMP dir
        config_path = @item.save_config_to_temp_dir

        rmx_properties_ui_location = File.join(START_DIR, 'rmx_properties_ui.json').tr('\\', '/')

        # also try to save the properties config file to the predefined location

        begin
            # Save rmx_properties_ui.json file in START_DIR
            File.open(rmx_properties_ui_location, 'w') do |file|
                file.write(@item.to_json)
            end

            # Make a copy of rmx_properties_ui.json file to Nextlabs Temp dir (backup)
            FileUtils.cp(rmx_properties_ui_location, @item.temp_dir)
        rescue StandardError => ex
            puts 'Failed to save json properties file for later reference'
            puts "Rescued: #{ex.inspect}"
        end

        # Then We create and save the chef-client config file
        chef_client_config = <<"END_OF_CONFIG"
cookbook_path       "#{File.join(START_DIR, 'cookbooks')}"
listen              false
local_mode          true
chef_zero.enabled   true
json_attribs        "#{config_path}"
log_location        STDOUT
log_level           :info
verbose_logging     true
cache_path          "#{START_DIR}/debug"
END_OF_CONFIG

        chef_client_config_path = File.join(Dir.tmpdir, 'client.rb')
        # puts chef_client_config_path
        # save the chef-client config file to temp dir
        File.open(chef_client_config_path, 'w') do |file|
            file.write(chef_client_config)
        end

        # ***PROGRESS***: Initialize Chef installer
        @progress.fraction = 0.05
        if ENV['rollback'] == 'true'
            @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_rollback'], @progress_msg_color))
        elsif @item.installation_mode.start_with?('install')
            @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_start_install_chef'], @progress_msg_color))
        elsif @item.installation_mode.start_with?('remove')
            @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_start_uninstall_chef'], @progress_msg_color))
        elsif @item.installation_mode.start_with?('upgrade')
            @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_start_upgrade_chef'], @progress_msg_color))
        end

        @install_util = {}
        # This is the variable to store the original logs for storing to log file
        @install_util['original_log'] = ''
        # This is a variable to store error message from chef-client process
        @install_util['error_msg'] = ''

        # Update Logging progress base on Chef Log
        update_progress = lambda do |log_msg|
            

            if match = log_msg.match(/INFO: \[Progress\]\s(.*)/i)
                progress_msg = match.captures[0].strip

                @progress_msg.text = strong(fg(progress_msg, @progress_msg_color))

                # Progress Log. Include for Install, Uninstall, Upgrade process
                # ***PROGRESS***: Bootstrapping
                if log_msg.include?(MESSAGE['progress_log_messages']['bootstrap'])
                    @progress.fraction = 0.1
                    # ***PROGRESS***: NXL Template
                elsif log_msg.include?(MESSAGE['progress_log_messages']['nxl_template'])
                    @progress.fraction = 0.15
                    # ***PROGRESS***: Foundation Customization
                elsif log_msg.include?(MESSAGE['progress_log_messages']['foundation_customization'])
                    @progress.fraction = 0.2
                    # ***PROGRESS***: Workflow Initial Protection
                elsif log_msg.include?(MESSAGE['progress_log_messages']['workflow_initial_protection'])
                    @progress.fraction = 0.25
                    # ***PROGRESS***: Batch Protection
                elsif log_msg.include?(MESSAGE['progress_log_messages']['batch_protection'])
                    @progress.fraction = 0.3
                    # ***PROGRESS***: Active Workspace Support
                elsif log_msg.include?(MESSAGE['progress_log_messages']['active_workspace_support'])
                    @progress.fraction = 0.35
                    # ***PROGRESS***: Supplier Collaboration Integration
                elsif log_msg.include?(MESSAGE['progress_log_messages']['scf_integration'])
                    @progress.fraction = 0.4
                    # ***PROGRESS***: Dispatcher Module
                elsif log_msg.include?(MESSAGE['progress_log_messages']['dispatcher_module'])
                    @progress.fraction = 0.45
                    # ***PROGRESS***: Dispatcher Client Extension
                elsif log_msg.include?(MESSAGE['progress_log_messages']['dispatcher_client_extension'])
                    @progress.fraction = 0.5
                    # ***PROGRESS***: Translator Rights Checker
                elsif log_msg.include?(MESSAGE['progress_log_messages']['translator_rights_checker'])
                    @progress.fraction = 0.55
                    # ***PROGRESS***: User Initial Protection
                elsif log_msg.include?(MESSAGE['progress_log_messages']['user_initial_protection'])
                    @progress.fraction = 0.6
                    # ***PROGRESS***: RAC RMX
                elsif log_msg.include?(MESSAGE['progress_log_messages']['rac_rmx'])
                    @progress.fraction = 0.65
                    # ***PROGRESS***: Proxy Runner
                elsif log_msg.include?(MESSAGE['progress_log_messages']['proxy_runner'])
                    @progress.fraction = 0.7
                    # ***PROGRESS***: FMS Partial Download
                elsif log_msg.include?(MESSAGE['progress_log_messages']['partialdownload'])
                    @progress.fraction = 0.75
                    # ***PROGRESS***: NX Interation
                elsif log_msg.include?(MESSAGE['progress_log_messages']['nx_integration'])
                    @progress.fraction = 0.8
                    # ***PROGRESS***: Creo Integration
                elsif log_msg.include?(MESSAGE['progress_log_messages']['creo_integration'])
                    @progress.fraction = 0.85
                    # ***PROGRESS***: SolidWorks Integration
                elsif log_msg.include?(MESSAGE['progress_log_messages']['solidworks_integration'])
                    @progress.fraction = 0.9
                    # ***PROGRESS***: AutoCAD Integration
                elsif log_msg.include?(MESSAGE['progress_log_messages']['autocad_integration'])
                    @progress.fraction = 0.95
                    # ***PROGRESS***: Briefcase Browser Integration
                elsif log_msg.include?(MESSAGE['progress_log_messages']['bb_integration'])
                    @progress.fraction = 0.95
                    # ***PROGRESS***: Finished
                elsif log_msg.include?(MESSAGE['progress_log_messages']['install_finished'])
                    @progress.fraction = 1.0
                    tmp_prgmsg = MESSAGE['install_page']['progress_finish']
                    tmp_prgmsg = MESSAGE['install_page']['progress_rollback_done'] if ENV['rollback'] == 'true'
                    @progress_msg.text = strong(fg(tmp_prgmsg, @progress_msg_color))
                    @flag.installation_finished = true
                elsif log_msg.include?(MESSAGE['progress_log_messages']['uninstall_finished'])
                    @progress.fraction = 1.0
                    tmp_prgmsg = MESSAGE['install_page']['progress_remove_finish']
                    tmp_prgmsg = MESSAGE['install_page']['progress_rollback_done'] if ENV['rollback'] == 'true'
                    @progress_msg.text = strong(fg(tmp_prgmsg, @progress_msg_color))
                    @flag.installation_finished = true
                elsif log_msg.include?(MESSAGE['progress_log_messages']['upgrade_finished'])
                    @progress.fraction = 1.0
                    @progress_msg.text = strong(fg('Upgrade completed', @progress_msg_color))
                    @flag.installation_finished = true
                end
            end

            if log_msg.match(/.*ERROR:(.*)/i)
                if log_msg.include?('check_nxl_file') && log_msg.include?('returns 14135')
                    @flag.installation_failed = false
                elsif log_msg.include?(MESSAGE['start_service_error'])
                    # FSC uninstallation
                    @flag.installation_failed = true
                    @install_util['error_msg'] = 'FSC service cannot start. Please restart your machine and uninstall RMX again.'
                else
                    match_error = log_msg.match(/.*ERROR:(.*)/i)
                    error_msg = match_error.captures[0].strip
                    @flag.installation_failed = true
                    @install_util['error_msg'] = "#{MESSAGE['install_page']['progress_failed'][@item.installation_mode]}: #{error_msg}"
                end
            end
        end

        # Clean the Log Timestamp
        clean_log = lambda do |log_msg|
            # Remove the timestamp of log message
            log_msg.gsub!(/\[[\d\-T:+]*\]/, '')

            case log_msg
            when /Terminate batch job/i then
                return ''
            when /INFO: Storing updated/i then
                return ''
            else
                return log_msg
            end
        end

        # Write whole chef log to text file after installation process. Also update if there are any failure or cancel.
        post_install_script_call = lambda do
            if @flag.installation_failed
                @progress.fraction = 0.0
                @progress_msg.text = strong(fg(@install_util['error_msg'], @progress_error_msg_color))
            end

            if @flag.installation_canceled
                @progress.fraction = 0.0
                @progress_msg.text = strong(fg(MESSAGE['install_page']['progress_canceled'], @progress_msg_color))
            end

            # update the flag
            @flag.install_cmd_exited = true

            # write the log to log file. Make a copy in TC_ROOT\logs folder
            puts 'Write Installation Log'
            log_location = File.join(ENV['START_DIR'], 'installer.log')
            log_file = File.open(log_location, 'a+')
            log_file.write(@install_util['original_log'])

            if @item.tc_root != '' && @flag.installation_finished
                mode = @item.installation_mode == 'install' ? 'install' : 'uninstall'
                mode = 'upgrade' if @item.installation_mode == 'upgrade'
                tc_drm_log_name = "TcDRM_#{mode}_#{Time.now.strftime('%Y%m%d_%H%M')}.log"
                tc_drm_log = File.join(@item.tc_root, 'install', tc_drm_log_name)
                File.open(tc_drm_log, 'w').write(@install_util['original_log'])
            end

            # delete rmx_properties.json file in %RMX_ROOT%/tmp dir
            FileUtils.rm_rf(config_path)
        end

        # Make the Installation Call here
        case RUBY_PLATFORM
        when /mswin|mingw|windows/ then # we are on windows
            trap('INT') {}

            install_script = "\"#{START_DIR}/engine/chef/bin/chef-client\" --config \"#{chef_client_config_path}\" -o RMX_installation::main"
            @install_util['install_script'] = install_script

            roll_back_script = "\"#{START_DIR}/engine/chef/bin/chef-client\" --config \"\""
            @install_util['rollback_script'] = roll_back_script

            Thread.new do
                trap('INT') {}

                @install_util['pipe'] = IO.popen(@install_util['install_script'])
                @install_util['pipe'].each do |line|
                    @install_util['original_log'] += line
                    update_progress.call line
                    @log_area.text += clean_log.call(line.to_s)
                end
                # end
                post_install_script_call.call
            end
        when /linux/ then # we are on linux
            trap('INT') {}

            @install_util['install_script'] = <<"END_OF_SCRIPT"
/opt/chef/bin/chef-client --config #{chef_client_config_path} \
-o RMX_installation::main
END_OF_SCRIPT

            Thread.new do
                trap('INT') {}

                @install_util['pipe'] = IO.popen(@install_util['install_script'])
                @install_util['pipe'].each do |line|
                    @install_util['original_log'] += line
                    update_progress.call line
                    @log_area.text += clean_log.call(line.to_s)
                end
                # end
                post_install_script_call.call
            end
        else
            puts "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
        end
    end
end

# if __FILE__ == $0
#   Shoes.app :title => MESSAGE['title'] , width: 960, height: 670 do
#     # disable the window from resized by the user
#     # refer to http://goo.gl/H6m1rZ
#     win.set_size_request(960, 670)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/install')
#   end
# end
