#!/usr/bin/env ruby

# Teamcenter RMX 5.2 GUI Installation
# Component     :: Wizard
# Description   :: GUI Navigation Wizard
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative './bootstrap'
require_relative './utility'

# Extend Shoes application to navigate different page UI
class Installation < Shoes
    include Utility
    INSTALL_WIZARD_ARRAY = %w(
        /
        /agreement
        /install_dir
        /disp_root
        /creo_dir
        /solidworks_dir
        /autocad_dir
        /catia_dir
        /bb_dir
        /feature_select
        /infodba_psd
        /skydrm_info
        /cc_info
        /installation_confirmation
        /install
        /installation_summary
    ).freeze

    REMOVE_WIZARD_ARRAY = %w(
        /
        /feature_select
        /infodba_psd
        /installation_confirmation
        /install
        /installation_summary
    ).freeze

    UPGRADE_WIZARD_ARRAY = %w(
        /
        /feature_select
        /infodba_psd
        /cc_info
        /jpc_remote_setting
        /installation_confirmation
        /install
        /installation_summary
    ).freeze

    SKIP_PAGES = {}

    INSTALL_WIZARD_ARRAY.each do |page|
        SKIP_PAGES[page] = false
    end

    REMOVE_WIZARD_ARRAY.each do |page|
        SKIP_PAGES[page] = false
    end

    UPGRADE_WIZARD_ARRAY.each do |page|
        SKIP_PAGES[page] = false
    end

    def wizard(direction)
        # Get current_location to decide which page to move next. (Example: '/', '/agreement', '/install',...)
        current_location = location

        if @item.installation_mode == 'remove'
            wizard_array = REMOVE_WIZARD_ARRAY
        elsif @item.installation_mode == 'install'
            wizard_array = INSTALL_WIZARD_ARRAY
        elsif @item.installation_mode == 'upgrade'
            wizard_array = UPGRADE_WIZARD_ARRAY
        else
            msg = "Not supported setup mode: #{@item.installation_mode}"
            alert_ontop_parent(app.win, msg, title: app.instance_variable_get('@title'))
            return current_location
        end

        next_visiting_page(current_location, direction, wizard_array)
    end

    def next_visiting_page(current_location, direction, wizard_array)
        current_idx = wizard_array.find_index(current_location)
        case direction
        when :next
            cnt = find_next_page(wizard_array, current_idx)
            return wizard_array.fetch(current_idx + cnt)
        when :back
            cnt = find_back_page(wizard_array, current_idx)
            return wizard_array.fetch(current_idx - cnt)
        else
            return current_location
        end
    end

    def find_next_page(wizard_array, id)
        i = 1
        i += 1 while SKIP_PAGES[wizard_array.fetch(id + i)]
        i
    end

    def find_back_page(wizard_array, id)
        i = 1
        i += 1 while SKIP_PAGES[wizard_array.fetch(id - i)]
        i
    end
end
