# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_active_worspace_support
# Description   :: Re-deploy Nextlabs AWC module: NXLOverlay and CmdDRM
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'rexml/document'
require 'fileutils'
require 'json'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'deploy_nextlab_onestep_command_upgrade' do
    block do
        log_info(LOGMSG::UPGRADE_ACTIVE_WORKSPACE_SUPPORT)
        aws_version = get_aws_version(node['TC_ROOT'])

        if aws_version >= 3.3
            # copy javascript code to solution folder if the version is 3.3 / 3.4 / 4.0 / 4.1 / 4.2
            log_info('Copy CmdDRM folder to TC_ROOT/aws2/stage/src/solution')
            source_folder = File.join(ENV['RMX_ROOT'], 'AWC', 'OneStepCommands')
            dest_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution')

            if File.exist?(dest_folder)
                FileUtils.remove_dir(File.join(dest_folder, 'CmdDRM'), true) if File.exist?(File.join(dest_folder, 'CmdDRM'))
                case aws_version
                when 3.4, 4.0, 4.1, 4.2
                    drm_package = 'nextlabs.drm_' + aws_version.to_s
                    FileUtils.cp_r(File.join(source_folder, drm_package), dest_folder)
                    File.rename(File.join(dest_folder, drm_package), File.join(dest_folder, 'CmdDRM'))
                else # aws_version == 3.3
                    drm_package = 'nextlabs.drm_3.4'
                    FileUtils.cp_r(File.join(source_folder, drm_package), dest_folder)
                    File.rename(File.join(dest_folder, drm_package), File.join(dest_folder, 'CmdDRM'))

                    # Need to change CmdDRMService.js to require logService.js instead of logger.js (logger.js will compatiable with 3.4, 4.0, 4.1, ...)
                    cmd_drm_file = File.join(dest_folder, 'CmdDRM', 'src', 'js', 'CmdDRMService.js')
                    lines = IO.readlines(cmd_drm_file).map do |line|
                        if line.include?('js/logger')
                            line.gsub('js/logger', 'js/logService')
                        else
                            line
                        end
                    end

                    File.open(cmd_drm_file, 'w') do |f|
                        f.puts lines
                    end
                end

                # Update kit.json of dest_folder
                log_info('Update kit.json file to include CmdDRM.')
                kit_json = File.join(dest_folder, 'kit.json')
                awc_update_kit_json(kit_json, 'install', 'CmdDRM')

                # Move icons to image folder
                log_info('Add cmdDRM icons to image folder.')
                icon_folder = File.join(dest_folder, 'CmdDRM', 'src', 'icons')
                image_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'image')
                copy_files(icon_folder, image_folder)
                FileUtils.remove_dir(icon_folder, true)
            end
        end
    end
end

ruby_block 'rebuild_awc_war_upgrade' do
    block do
        log_info('[Progress] Building awc.war...')
        aws_version = get_aws_version(node['TC_ROOT'])
        gwtcompile = File.join(node['TC_ROOT'], 'aws2', 'stage', 'gwtcompile.cmd')
        gwtcompile = gwtcompile.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        gwtcompile_cmd = "\"#{gwtcompile}\""
        # In AWS 4.0 and AWS 4.1, gwtcompile.cmd sometime hang if run inside chef-client.
        # Decide to run in separate command prompt and UI will wait for this process to finish and proceed further
        if aws_version >= 4.0
            gwtcompile_cmd = "start /w \"Building AWC.war\" cmd.exe /C #{gwtcompile}"
        end
        call_system_command('Rebuild AWC.war', 'gwtcompile', gwtcompile_cmd)
    end
end