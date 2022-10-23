#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Feature Select
# Description   :: Render feature select page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Feature Select page
class Installation < Shoes
    include Utility
    url('/feature_select', :feature_select)

    def feature_select
        ##################################### initialize  #####################################
        # calculate the total number of features in features.xml
        # initialize checkboxes and flows
        feature_num = []
        total_num = 0
        Item::DEPLOYMENT_COMPONENTS.each do |component|
            num = @item.get_deployment_type_count(component)
            feature_num.push(num.to_i)
            total_num += num.to_i
        end

        fea_check = Array.new(total_num)
        fea_flow = Array.new(total_num)

        # Jira Issue: TC-580 - Exclude CAD App (not show at all) if installer not include in TcDRM
        exclude_cad_app_feature_index = []

        feature_will_process = @item.feature_list

        if @item.installation_mode == 'install'
            start_num = 0
            Item::DEPLOYMENT_COMPONENTS.each do |deployment_component|
                dep_index = Item::DEPLOYMENT_COMPONENTS.index(deployment_component)
                end_num = start_num + feature_num[dep_index]

                (start_num...end_num).each do |i|
                    fea_flow[i] = flow width: 1.0 do
                        component = Item::FEATURES[i]
                        tc_dep = @item.get_tc_dep_of_feature(component)
                        available_solution = @item.get_feature_available_solution(component)

                        if Item::CAD_RMX.include?(component)        # This feature is a CAD RMX
                            cad_name = component.split('_')[0]      # Extract CAD NAME from the component name
                            unless @item.is_cad_rmx_include?(cad_name)
                                exclude_cad_app_feature_index.push(i)
                                feature_will_process.delete(component)
                                next
                            end
                        end

                        unless feature_will_process.include?(component)
                            if select_feature(component, tc_dep, available_solution, dep_index)
                                feature_will_process.push(component)
                            else
                                feature_will_process.delete(component)
                            end
                        end
                    end
                end
                start_num = end_num
            end
        else
            # When process happening is UNINSTALL. The RMD is the only one not uninstall
            if feature_will_process.include?('rights_management_desktop')
                feature_will_process.delete('rights_management_desktop')
            end

            (0...Item::FEATURES.length).each do |i|
                component = Item::FEATURES[i]
                if Item::CAD_RMX.include?(component) && !feature_will_process.include?(component)
                    exclude_cad_app_feature_index.push(i)
                end
            end
        end

        # Changes the link style and link hover style
        style(Link, underline: false, stroke: '#FFF', weight: 'bold')
        style(LinkHover, underline: false, stroke: '#FFF', weight: 'bold')

        stack @installer_page_size_style do
            # The header area
            stack @installer_header_size_style do
                background @header_color
                banner
            end

            # The body
            flow @installer_content_size_style do
                background @content_color

                # heading stack
                stack width: 1.0, height: 60 do
                    para MESSAGE['feature_select_page'][@item.installation_mode]['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end
                stack width: 0.45, height: 40 do
                    para MESSAGE['feature_select_page'][@item.installation_mode]['module_selection'], @small_text_style.merge(align: 'center', stroke: '#000000', margin_left: 260)
                end

                # feature and component stack
                flow width: 920, height: 335 do
                    # This is a horizontal spacer
                    stack width: 20, height: 85 do
                        para ''
                    end

                    (0...Item::DEPLOYMENT_COMPONENTS.size).each do |ii|
                        # This is a horizontal spacer
                        stack width: 20, height: 1.0 do
                            para ''
                        end
                        stack width: 280, height: 50 do
                            background @while_square_color

                            flow width: 1.0, height: 50, margin_left: 25 do
                                # deployment type string stack
                                stack width: 1.0, height: 10 do
                                    para ''
                                end

                                # deployment_component = Item::DEPLOYMENT_COMPONENTS[ii]
                                # @dep_check[ii] = check
                                # set_feature_status(@dep_check[ii], deployment_component, $install_config[ii])
                                para MESSAGE['inputs'][Item::DEPLOYMENT_COMPONENTS[ii]], @small_text_style.merge(stroke: '#006400', width: 200, margin_bottom: 3, size: 10.5, weight: 'bold')
                            end
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 20, height: 190 do
                        para ''
                    end
                    start_num = 0
                    # dep_index = 0
                    Item::DEPLOYMENT_COMPONENTS.each do |deployment_component|
                        # This is a horizontal spacer
                        stack width: 20, height: 1.0 do
                            para ''
                        end

                        stack width: 280, height: 210 do
                            background @while_square_color

                            dep_index = Item::DEPLOYMENT_COMPONENTS.index(deployment_component)
                            end_num = start_num + feature_num[dep_index]

                            (start_num...end_num).each do |i|
                                if exclude_cad_app_feature_index.include?(i)
                                    next
                                end
                                fea_flow[i] = flow width: 1.0 do
                                    # This is a horizontal spacer
                                    stack width: 15 do
                                        para ''
                                    end

                                    component = Item::FEATURES[i]
                                    fea_check[i] = check
                                    if component == 'nx_integration' && @item.installation_mode == 'install'
                                        if can_install_nx_rmx?
                                            @nx_rmx_feature = fea_check[i]
                                        else
                                            @nx_rmx_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'creo_integration' && @item.installation_mode == 'install'
                                        if can_install_creo_rmx?
                                            @creo_rmx_feature = fea_check[i]
                                        else
                                            @creo_rmx_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'solidworks_integration' && @item.installation_mode == 'install'
                                        if can_install_solidworks_rmx?
                                            @solidworks_rmx_feature = fea_check[i]
                                        else
                                            @solidworks_rmx_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'autocad_integration' && @item.installation_mode == 'install'
                                        if can_install_autocad_rmx?
                                            @autocad_rmx_feature = fea_check[i]
                                        else
                                            @autocad_rmx_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'catia_integration' && @item.installation_mode == 'install'
                                        if can_install_catia_rmx?
                                            @catia_rmx_feature = fea_check[i]
                                        else
                                            @catia_rmx_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'partialdownload' && @item.installation_mode == 'install'
                                        if @item.can_install_partialdownload_server?
                                            @partialdownload_feature = fea_check[i]
                                        else
                                            @partialdownload_feature = fea_check[i]
                                            fea_check[i].state = 'disabled'
                                        end
                                    elsif component == 'rights_management_desktop' && @item.installation_mode == 'install' && @item.can_install_rmd_with_version?
                                        if true
                                            @rmd_feature = fea_check[i]
                                        else
                                            @rmd_feature = nil
                                            fea_check[i].state = 'disabled'
                                        end
                                    else
                                        fea_check[i].state = 'disabled'
                                    end
                                    fea_check[i].checked = true if feature_will_process.include?(component)
                                    para MESSAGE['inputs'][component], @small_text_style.merge(width: 225, margin_bottom: 3, margin_left: 10, size: 10.5)
                                end
                            end
                            start_num = end_num
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 20, height: 40 do
                        para ''
                    end
                    Item::DEPLOYMENT_COMPONENTS.each do |deployment_component|
                        # This is a horizontal spacer
                        stack width: 20, height: 1.0 do
                            para ''
                        end
                        stack width: 280, height: 60 do
                            background @while_square_color
                            para MESSAGE['hover_msg'][deployment_component], @small_text_style.merge(stroke: '#696969', margin_top: 15, emphasis: 'italic', align: 'center')
                        end
                    end
                end

                stack width: 1.0, height: 20 do
                    para ''
                end

                # installer progress
                @progress_flow = flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 384, height: 1.0 do
                        background @install_progress_front_color
                    end
                end
                @progress_flow.hide if @item.installation_mode.eql?('remove')
            end

            # The footer area
            stack @installer_footer_size_style do
                background @footer_color

                # This is a vertical spacer
                @feature_text_area = stack width: 1.0, height: 15 do
                    para ' ', @small_text_style
                end

                flow width: 1.0, height: 60 do
                    stack width: 180, height: 50 do
                        @feature_select_back_btn = nl_button :back
                        @feature_select_back_btn.click do
                            if @item.installation_mode.eql?('install')
                                @item.feature_list = []
                            end
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        @feature_select_next_btn = nl_button :next
                        @feature_select_next_btn.click do
                            update_feature_list(feature_will_process)
                            puts(feature_will_process)
                            set_skip_page if @item.installation_mode.eql?('install')
                            set_uninstall_skip_page if @item.installation_mode.eql?('remove')
                            set_upgrade_skip_page if @item.installation_mode.eql?('upgrade')
                            set_feature_to_off if @item.installation_mode.eql?('remove')
                            visit(wizard(:next))
                        end
                    end
                end
            end
        end
    end

    def select_feature(component, tc_dep, available_solution, dep_index)
        # return false if @item.instance_variable_get("@#{component}") == 'OFF' # Feature set to OFF should return false immediately
        return false if @item.installation_mode != 'install' && !@item.feature_list.include?(component) # When uninstall/upgrade installed feature_list will be check

        to_select = true
        to_select = false if (RUBY_PLATFORM =~ /linux/) && dep_index == 2 # do not install client features on linux server
        to_select = false unless available_solution.include?(@item.solution_type)
        # if component.eql?('nxl_template') || component.eql?('foundation_customization') || component.eql?('batch_protection')
        if %w(nxl_template foundation_customization batch_protection nxl_ipem_template nxl_swim_template).include?(component)
            to_select = false if !teamcenter_dependency_checked(tc_dep) || @item.tc_data.strip == ''
        elsif component.eql?('active_workspace_support')
            to_select = false if !teamcenter_dependency_checked(tc_dep) || @item.aws_version < 4.3
        elsif component.eql?('scf_integration')
            to_select = false if !teamcenter_dependency_checked(tc_dep)
        elsif component.eql?('dispatcher_module') || component.eql?('translator_rights_checker') || component.eql?('dispatcher_client_extension')
            to_select = false if !teamcenter_dependency_checked(tc_dep) || @item.disp_root.strip == '' || !FileTest.directory?(@item.disp_root)
        elsif component.eql?('nx_integration')
            ugii_root = ENV['UGII_ROOT_DIR']
            ugii_root = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root.nil? && !ENV['UGII_BASE_DIR'].nil?
            to_select = false if ugii_root.nil? || !File.exist?(ugii_root)
        elsif component.eql?('partialdownload')
            to_select = false if !(FileTest.directory? File.join(@item.tc_root, 'tccs')) || !teamcenter_dependency_checked(tc_dep) || !@item.can_install_partialdownload?
        elsif component.eql?('rac_rmx')
            to_select = false if !teamcenter_dependency_checked(tc_dep) || !@item.can_install_rac_rmx?
        elsif component.eql?("bb_integration")
            ugii_root = ENV['UGII_ROOT_DIR']
            ugii_root = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root.nil? && !ENV['UGII_BASE_DIR'].nil?
            to_select = false if ugii_root.nil? || !File.exist?(ugii_root) || @item.bb_dir == "" || !FileTest.directory?(@item.bb_dir)
        elsif component.eql?('creo_integration')
            creo_common_files = File.expand_path('../Common Files', @item.creo_dir)
            to_select = false if @item.creo_dir == '' || !FileTest.directory?(creo_common_files)
        elsif component.eql?('solidworks_integration')
            to_select = false if @item.solidworks_dir == ''
        elsif component.eql?('autocad_integration')
            to_select = false if @item.autocad_dir == ''
        elsif component.eql?('catia_integration')
            to_select = false if @item.catia_dir == ''
        elsif component.eql?('rights_management_desktop')
            to_select = false if !@item.can_install_rmd?
        else
            to_select = false unless teamcenter_dependency_checked(tc_dep)
        end
        to_select
    end

    def can_install_nx_rmx?
        can_install = true
        ugii_root = ENV['UGII_ROOT_DIR']
        ugii_root = File.join(ENV['UGII_BASE_DIR'], 'UGII') if ugii_root.nil? && !ENV['UGII_BASE_DIR'].nil?
        can_install = false if ugii_root.nil? || !File.exist?(ugii_root)
        can_install
    end

    def can_install_creo_rmx?
        can_install = true
        creo_common_files = File.expand_path('../Common Files', @item.creo_dir)
        can_install = false if @item.creo_dir == '' || !FileTest.directory?(creo_common_files)
        can_install
    end

    def can_install_solidworks_rmx?
        can_install = @item.solidworks_dir == '' ? false : true
        can_install
    end

    def can_install_autocad_rmx?
        can_install = @item.autocad_dir == '' ? false : true
        can_install
    end

    def can_install_catia_rmx?
        can_install = @item.catia_dir == '' ? false : true
        can_install
    end

    def set_skip_page
        SKIP_PAGES['/infodba_psd'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@foundation_customization') == 'OFF'
                                         true
                                     else
                                         false
                                     end

        SKIP_PAGES['/cc_info'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@dispatcher_module') == 'OFF'
                                                true
                                            else
                                                false
                                            end

        # Partial Download Solution for client doesn't need SkyDRM configuration file
        SKIP_PAGES['/skydrm_info'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@dispatcher_module') == 'OFF' \
                                           && (@item.instance_variable_get('@partialdownload') == 'OFF' || (@item.instance_variable_get('@partialdownload') == 'ON' && !@item.can_install_partialdownload_server?))
                                         true
                                     else
                                         false
                                     end
    end

    def set_uninstall_skip_page
        SKIP_PAGES['/infodba_psd'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@foundation_customization') == 'OFF'
                                         true
                                     else
                                         false
                                     end
    end

    def set_upgrade_skip_page
        SKIP_PAGES['/infodba_psd'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@foundation_customization') == 'OFF'
                                         true
                                     else
                                         false
                                     end
        SKIP_PAGES['/cc_info'] = if @item.instance_variable_get('@nxl_template') == 'OFF' && @item.instance_variable_get('@dispatcher_module') == 'OFF'
                                                true
                                            else
                                                false
                                            end
    end

    def update_feature_list(feature_will_process)
        Item::FEATURES.each do |feature|
            if feature_will_process.include?(feature)
                @item.feature_list.push(feature) unless @item.feature_list.include?(feature)
                @item.instance_variable_set('@' + feature, 'ON')
            else
                @item.feature_list.delete(feature)
                @item.instance_variable_set('@' + feature, 'OFF')
            end
        end

        if @item.installation_mode == 'install'
            if @nx_rmx_feature.nil? || !@nx_rmx_feature.checked?
                @item.feature_list.delete('nx_integration')
                @item.instance_variable_set('@nx_integration', 'OFF')
            else
                @item.feature_list.push('nx_integration') unless @item.feature_list.include?('nx_integration')
                @item.instance_variable_set('@nx_integration', 'ON')
            end

            if @creo_rmx_feature.nil? || !@creo_rmx_feature.checked?
                @item.feature_list.delete('creo_integration')
                @item.instance_variable_set('@creo_integration', 'OFF')
            else
                @item.feature_list.push('creo_integration') unless @item.feature_list.include?('creo_integration')
                @item.instance_variable_set('@creo_integration', 'ON')
            end

            if @solidworks_rmx_feature.nil? || !@solidworks_rmx_feature.checked?
                @item.feature_list.delete('solidworks_integration')
                @item.instance_variable_set('@solidworks_integration', 'OFF')
            else
                @item.feature_list.push('solidworks_integration') unless @item.feature_list.include?('solidworks_integration')
                @item.instance_variable_set('@solidworks_integration', 'ON')
            end

            if @autocad_rmx_feature.nil? || !@autocad_rmx_feature.checked?
                @item.feature_list.delete('autocad_integration')
                @item.instance_variable_set('@autocad_integration', 'OFF')
            else
                @item.feature_list.push('autocad_integration') unless @item.feature_list.include?('autocad_integration')
                @item.instance_variable_set('@autocad_integration', 'ON')
            end

            if @catia_rmx_feature.nil? || !@catia_rmx_feature.checked?
                @item.feature_list.delete('catia_integration')
                @item.instance_variable_set('@catia_integration', 'OFF')
            else
                @item.feature_list.push('catia_integration') unless @item.feature_list.include?('catia_integration')
                @item.instance_variable_set('@catia_integration', 'ON')
            end

            if @partialdownload_feature.nil? || !@partialdownload_feature.checked?
                @item.feature_list.delete('partialdownload')
                @item.instance_variable_set('@partialdownload', 'OFF')
            else
                @item.feature_list.push('partialdownload') unless @item.feature_list.include?('partialdownload')
                @item.instance_variable_set('@partialdownload', 'ON')
            end

            if @rmd_feature.nil? || !@rmd_feature.checked?
                @item.feature_list.delete('rights_management_desktop')
                @item.instance_variable_set('@rights_management_desktop', 'OFF')
            else
                @item.feature_list.push('rights_management_desktop') unless @item.feature_list.include?('rights_management_desktop')
                @item.instance_variable_set('@rights_management_desktop', 'ON')
            end
        end
    end

    def set_feature_to_off
        # when uninstalling, have to set features inside feature_list to be OFF
        @item.feature_list.each do |component|
            @item.instance_variable_set('@' + component, 'OFF')
        end
    end

    def teamcenter_dependency_checked(tc_dep_array)
        tc_dep_array.each do |id|
            return false unless @item.check_teamcenter_feature(id)
        end
        true
    end
end

# if __FILE__ == $0
#   Shoes.app :title => MESSAGE['title'] , width: 960, height: 670 do
#     win.set_size_request(960, 670)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/feature_select')
#   end
# end
