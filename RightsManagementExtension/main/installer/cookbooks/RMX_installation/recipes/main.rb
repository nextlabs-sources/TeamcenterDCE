# Teamcenter RMX 5.2
# Cookbook Name :: RMX_installation
# Recipes       :: Main
# Description   :: Entry point of the cookbook. Get feature list to (un)install and update recipes list will be executed
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

include_recipe 'RMX_installation::bootstrap'
include_recipe 'RMX_installation::silence_config' if node['silence_install'] == 'yes'
include_recipe 'RMX_installation::generate_feature_list'

Chef::Recipe.send(:include, Compile::Config)

mode = 'unknown'
mode = 'install' if node['installation_mode'].to_s.strip.eql?('install')
mode = 'uninstall' if node['installation_mode'].to_s.strip.eql?('remove')
mode = 'upgrade' if node['installation_mode'].to_s.strip.eql?('upgrade')

break_point = get_break_point(node['temp_dir'])
# This feature in UI is different but will execute only in 2 recipe: nxl_template, install_foundation_customization
IGNORE_FEATURES = %w(nxl_template nxl_ipem_template nxl_swim_template foundation_customization)

if ENV['rollback'] != 'true' && ((mode == 'install' && (can_install_rmx? || node['silence_install'] == 'yes')) || (mode == 'upgrade' && can_upgrade_rmx?) || (mode == 'uninstall' && can_uninstall_rmx?))
    have_template_feature = false
    find_breakpoint = false
    find_breakpoint = true if break_point.eql?('none')
    Installer::RMX_FEATURES.each do |rmx_feature|
        find_breakpoint = true if break_point.include?(rmx_feature)
        if node['feature_list'].include?(rmx_feature) && find_breakpoint
            if IGNORE_FEATURES.include?(rmx_feature) && !have_template_feature
                include_recipe 'RMX_installation::' + mode + '_nxl_template'
                include_recipe 'RMX_installation::' + mode + '_foundation_customization'
                have_template_feature = true
            else
                recipe = 'RMX_installation::' + mode + '_' + rmx_feature
                include_recipe recipe
            end
        end
    end
    include_recipe 'RMX_installation::' + mode + '_finish'
elsif ENV['rollback'] == 'true'
    unless break_point.eql?('none')
        # for rollback, the break_point feature is the last feature that need to be rolled back
        Installer::RMX_FEATURES.each do |rmx_feature|
            if node['feature_list'].include?(rmx_feature)
                recipe = 'RMX_installation::' + mode + '_' + rmx_feature
                include_recipe recipe
            end

            break if break_point.include?(rmx_feature)
        end
        include_recipe 'RMX_installation::' + mode + '_finish'
    end
else
    raise('You do not have Rights Management eXtension fully installed, setup abort!')
end
