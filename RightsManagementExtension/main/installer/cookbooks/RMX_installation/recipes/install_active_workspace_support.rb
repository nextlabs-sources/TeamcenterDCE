# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_active_worspace_support
# Description   :: Deploy Nextlabs AWC module: NXLOverlay and CmdDRM
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'rexml/document'
require 'fileutils'
require 'json'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'deploy_nextlab_icon_indicator' do
    block do
        log_info(LOGMSG::INSTALL_ACTIVE_WORKSPACE_SUPPORT)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        aws_version = get_aws_version(node['TC_ROOT'])
        log_info("Detected Active Workspace #{aws_version} installed.")
        
        # TcRMX 13.3 support any version up from 4.3
        if aws_version >= 4.3
            log_info('Deploy NextLabs Icon Indicator.')
            log_info('Copy NXLOverlay folder to TC_ROOT/aws2/stage/src/solution')
            source_folder = File.join(ENV['RMX_ROOT'], 'NXLOverlay', "v#{aws_version}", 'NXLOverlay')
            dest_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution')
            if File.exist?(dest_folder)
                FileUtils.cp_r(source_folder, dest_folder)
            end

            # Update kit.json of dest_folder
            log_info('Update kit.json file to include NXLOverlay.')
            kit_json = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution', 'kit.json')
            awc_update_kit_json(kit_json, 'install', 'NXLOverlay')

            # Move indicator icon to image folder
            log_info('Add indicatorNXLOverlay icon to image folder.')
            image_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'image')
            icon_file = File.join(dest_folder, 'NXLOverlay', 'indicatorNXLOverlay16.svg')
            FileUtils.cp_r(icon_file, image_folder)
            FileUtils.remove(icon_file)
        end
    end
end

ruby_block 'deploy_nextlab_onestep_command' do
    block do
        aws_version = get_aws_version(node['TC_ROOT'])
        log_info('[Progress] Deploy NextLabs OneStepCommand module')

        if aws_version >= 4.3
            log_info('Copy CmdDRM folder to TC_ROOT/aws2/stage/src/solution')
            source_folder = File.join(ENV['RMX_ROOT'], 'AWC', 'OneStepCommands')
            dest_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution')
            if File.exist?(dest_folder)
                # Remove existing CmdDRM if any
                FileUtils.remove_dir(File.join(dest_folder, 'CmdDRM'), true) if File.exist?(File.join(dest_folder, 'CmdDRM'))

                drm_package = 'nextlabs.drm_' + aws_version.to_s
                FileUtils.cp_r(File.join(source_folder, drm_package), dest_folder)
                File.rename(File.join(dest_folder, drm_package), File.join(dest_folder, 'CmdDRM'))

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

ruby_block 'rebuild_awc_war' do
    block do
        log_info('[Progress] Redeploy AWC application...')
        awbuild_script = File.join(node['TC_ROOT'], 'aws2', 'stage', 'awbuild.cmd')
        awbuild_script = awbuild_script.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        awbuild_cmd = "\"#{awbuild_script}\""
        awbuild_cmd = "start /w \"Redeploy AWC web application\" cmd.exe /C #{awbuild_cmd}"
        call_system_command('Redeploy AWC web application', 'awbuild', awbuild_cmd)
    end
end