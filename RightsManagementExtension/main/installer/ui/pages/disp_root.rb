#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Dispatcher Root
# Description   :: Render dispatcher root page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Dispatcher Root page
class Installation < Shoes
    include Utility
    url('/disp_root', :disp_root)

    def disp_root
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
                    para MESSAGE['disp_root_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end

                stack width: 1.0, height: 25 do
                    para ''
                end

                stack width: 760, height: 350, margin_left: 200 do
                    flow width: 1.0, height: 120 do
                        para MESSAGE['disp_root_page']['disp_root_dir'], @small_text_style

                        stack width: 430, height: 50, margin_top: 10 do
                            @disp_root_edit = nl_text_edit(:large, @item.disp_root, width: 430, height: 40)
                            @disp_root_edit.change do
                                # use '/' to be consistent
                                @item.disp_root = @disp_root_edit.text.tr('\\', '/')
                            end
                        end

                        stack width: 100, height: 50, margin_top: 10 do
                            button MESSAGE['browse_folder_btn'], width: 80, height: 40 do
                                # if user cancel the folder selection, ask_open_folder will reture nil
                                # to prevent the nil being assigned to the variables, we need to check
                                browse_result = ask_open_folder
                                @disp_root_edit.text = browse_result unless browse_result.nil?
                            end
                        end
                        para MESSAGE['disp_root_page']['help_note'], @small_text_style.merge(size: 8)
                    end
                end

                # installer progress
                flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 96, height: 1.0 do
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
                        @disp_root_back_btn = nl_button :back
                        @disp_root_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        @disp_root_next_btn = nl_button :next
                        @disp_root_next_btn.click do
                            @item.disp_root = @disp_root_edit.text.strip

                            validated_disp_root, error_msg_disp_root = @item.validate_input(:disp_root, @item.disp_root)
                            
                            if !validated_disp_root
                                alert_ontop_parent(app.win, error_msg_disp_root, title: app.instance_variable_get('@title'))
                                @disp_root_edit.focus
                            else
                                # puts @item.disp_root.to_s
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
#     visit('/disp_root')
#   end
# end
