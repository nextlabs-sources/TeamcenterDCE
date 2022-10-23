#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Installation Directory
# Description   :: Render installation directory page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Installation Directory page
class Installation < Shoes
    include Utility
    url('/install_dir', :install_dir)

    def install_dir
        style(Link, underline: false, stroke: '#FFF', weight: 'bold')
        style(LinkHover, underline: false, stroke: '#FFF', weight: 'bold')

        stack @installer_page_size_style do
            # The header area
            stack @installer_header_size_style do
                background @header_color
                banner
            end

            # The body
            stack @installer_content_size_style do
                background @content_color

                stack width: 1.0, height: 80 do
                    para MESSAGE['install_dir_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end

                stack width: 760, height: 360, margin_left: 200 do
                    flow width: 1.0, height: 120 do
                        para MESSAGE['install_dir_page']['install_dir'], @small_text_style

                        stack width: 430, height: 50, margin_top: 10 do
                            @install_dir_edit = nl_text_edit(:large, @item.install_dir, width: 430, height: 40, state: 'disabled')
                        end

                        stack width: 100, height: 50, margin_top: 10 do
                            button MESSAGE['browse_folder_btn'], width: 80, height: 40 do
                                # if user cancel the folder selection, ask_open_folder will reture nil
                                # to prevent the nil being assigned to the variables, we need to check
                                browse_result = ask_open_folder
                                @install_dir_edit.text = browse_result unless browse_result.nil?
                            end
                        end
                        para MESSAGE['install_dir_page']['install_dir_help_note'], @small_text_style.merge(size: 8)
                    end

                    flow width: 1.0, height: 120 do
                        para MESSAGE['install_dir_page']['tc_root'], @small_text_style

                        stack width: 430, height: 50, margin_top: 10 do
                            @tc_root_edit = nl_text_edit(:large, @item.tc_root, width: 430, height: 40)
                            @tc_root_edit.change do
                                # use '/' to be consistent
                                @item.tc_root = @tc_root_edit.text.tr('\\', '/')
                            end
                        end

                        stack width: 100, height: 50, margin_top: 10 do
                            button MESSAGE['browse_folder_btn'], width: 80, height: 40 do
                                # if user cancel the folder selection, ask_open_folder will reture nil
                                # to prevent the nil being assigned to the variables, we need to check
                                browse_result = ask_open_folder
                                @tc_root_edit.text = browse_result unless browse_result.nil?
                            end
                        end
                        para MESSAGE['install_dir_page']['tc_root_help_note'], @small_text_style.merge(size: 8)
                        para MESSAGE['install_dir_page']['tc_root_empty'], @small_text_style.merge(size: 8)
                    end

                    @tcdata_flow = flow width: 1.0, height: 120 do
                        para MESSAGE['install_dir_page']['tc_data'], @small_text_style

                        stack width: 430, height: 50, margin_top: 10 do
                            @tc_data_edit = nl_text_edit(:large, @item.tc_data, width: 430, height: 40)
                            @tc_data_edit.change do
                                # use '/' to be consistent
                                @item.tc_data = @tc_data_edit.text.tr('\\', '/')
                            end
                        end

                        stack width: 100, height: 50, margin_top: 10 do
                            button MESSAGE['browse_folder_btn'], width: 80, height: 40 do
                                # if user cancel the folder selection, ask_open_folder will reture nil
                                # to prevent the nil being assigned to the variables, we need to check
                                browse_result = ask_open_folder
                                @tc_data_edit.text = browse_result unless browse_result.nil?
                            end
                        end
                        para MESSAGE['install_dir_page']['tc_data_help_note'], @small_text_style.merge(size: 8)
                        para MESSAGE['install_dir_page']['tc_data_empty'], @small_text_style.merge(size: 8)
                    end
                end

                stack width: 1.0, height: 15 do
                    para ''
                end

                # installer progress
                flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 0, height: 1.0 do
                        background @install_progress_front_color
                    end
                end
            end

            # The footer area
            stack @installer_footer_size_style do
                background @footer_color

                # This is a vertical spacer
                stack width: 1.0, height: 15 do
                    para ' '
                end

                flow width: 1.0, height: 60 do
                    stack width: 180, height: 50 do
                        back_btn = nl_button :back
                        back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        next_btn = nl_button :next
                        next_btn.click do
                            @item.install_dir = @install_dir_edit.text.strip
                            @item.tc_root = @tc_root_edit.text.strip
                            @item.tc_data = @tc_data_edit.text.strip

                            # depending on tc_root, the installer chech configuration.xml file under tc_root/install folder
                            # to determine the envrionemnt of this machine
                            # if only 4-tier rac is installed on this machine, the installer will not validate tc_data path
                            # Verify Install Dir, tc_root, tc_data
                            validated_install_dir, error_msg_install_dir = @item.validate_input(:install_dir, @item.install_dir)
                            validated_disk_space, error_msg_disk_space = @item.validate_input(:disk_space, @item.install_dir)
                            validated_tc_root, error_msg_tc_root = @item.validate_input(:tc_root, @item.tc_root)
                            validated_tc_data, error_msg_tc_data = @item.validate_input(:tc_data, @item.tc_data)
                            validated_fms_home, error_msg_fms_home = @item.validate_environment(:fms_home)
                            if !validated_install_dir
                                alert_ontop_parent(app.win, error_msg_install_dir, title: app.instance_variable_get('@title'))
                                @install_dir_edit.focus
                            elsif !validated_disk_space
                                alert_ontop_parent(app.win, error_msg_disk_space, title: app.instance_variable_get('@title'))
                            elsif !validated_tc_root
                                alert_ontop_parent(app.win, error_msg_tc_root, title: app.instance_variable_get('@title'))
                                @tc_root_edit.focus
                            elsif !validated_tc_data
                                alert_ontop_parent(app.win, error_msg_tc_data, title: app.instance_variable_get('@title'))
                                @tc_data_edit.focus
                            elsif @item.can_install_partialdownload? && !validated_fms_home
                                alert_ontop_parent(app.win, error_msg_fms_home, title: app.instance_variable_get('@title'))
                            else
                                visit(wizard(:next))
                            end
                        end
                    end
                end
            end
        end
    end
end

# if __FILE__ == $0
#   Shoes.app :title => MESSAGE['title'] , width: 960, height: 670 do
#     win.set_size_request(960, 670)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/install_dir')
#   end
# end
