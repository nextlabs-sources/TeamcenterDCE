# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_active_worspace_support
# Description   :: Remove Nextlabs AWC module: NXLOverlay and CmdDRM
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'uninstall_active_workspace_support' do
    block do
        log_info(LOGMSG::UNINSTALL_ACTIVE_WORKSPACE_SUPPORT)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)
        aws_version = get_aws_version(node['TC_ROOT'])

        if aws_version >= 4.3
            log_info('Remove NXLOverlay folder')
            nxloverlay = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution', 'NXLOverlay')
            FileUtils.remove_dir(nxloverlay, true)

            # Delete NXLOverlay entry in kit.json
            log_info('Remove NXLOverlay entry from kit.json in src/solution folder...')
            kit_json = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution', 'kit.json')
            awc_update_kit_json(kit_json, 'uninstall', 'NXLOverlay')

            # Uninstall CmdDRM OneStepCommand
            log_info('Remove CmdDRM folder...')
            cmd_drm = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution', 'CmdDRM')
            FileUtils.remove_dir(cmd_drm, true)

            log_info('Remove CmdDRM entry from kit.json in src/solution folder...')
            kit_json = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'solution', 'kit.json')
            awc_update_kit_json(kit_json, 'uninstall', 'CmdDRM')

            log_info('Remove svg icons in images folder...')
            image_folder = File.join(node['TC_ROOT'], 'aws2', 'stage', 'src', 'image')
            filenames = %w(cmdDRMProtect24.svg cmdDRMUnprotect24.svg)
            delete_files(image_folder, filenames)
        end
    end
end
