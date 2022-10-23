#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: JPC Setting
# Description   :: Render Java policy controller setting page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI JPC Setting page
class Installation < Shoes
    include Utility
    url('/jpc_remote_setting', :jpc_remote_setting)

    def jpc_remote_setting
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

                stack width: 1.0, height: 60 do
                    para MESSAGE['jpc_remote_setting_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end

                stack width: 1.0, height: 80 do
                    para ''
                end

                flow width: 1.0, height: 120, margin_left: 280 do
                    para MESSAGE['jpc_remote_setting_page']['remote_ip'], @small_text_style

                    stack width: 500, height: 60, margin_top: 10 do
                        @jpc_remote_ip_edit = nl_text_edit(:short, @item.jpc_remote_ip, width: 500, height: 40)
                    end

                    flow width: 500, height: 60, margin_top: 10 do
                        stack width: 300, height: 60, margin_top: 10 do
                            para MESSAGE['jpc_remote_setting_page']['jpc_eva_port'], @small_text_style
                            @jpc_eva_port_edit = nl_text_edit(:short, @item.jpc_eva_port, width: 150, height: 40)
                        end

                        stack width: 200, height: 60 do
                            # para "HTTPS:?" , @small_text_style
                            flow do
                                para "HTTPS Policy Controller", @small_text_style
                                @jpc_https_check = check
                                @jpc_https_check.checked = @item.jpc_https
                            end
                        end
                    end
                end

                flow width: 1.0, height: 120, margin_left: 280 do
                    
                end

                # This is a vertical spacer
                flow width: 1.0, height: 75 do
                    para ''
                end

                # installer progress
                flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 672, height: 1.0 do
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
                        jpc_remote_setting_back_btn = nl_button :back
                        jpc_remote_setting_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        jpc_remote_setting_next_btn = nl_button :next
                        jpc_remote_setting_next_btn.click do
                            @item.jpc_remote_ip = @jpc_remote_ip_edit.text
                            @item.jpc_eva_port = @jpc_eva_port_edit.text
                            @item.jpc_https = @jpc_https_check.checked?

                            validated_eva_port, error_msg_eva_port = @item.validate_input(:jpc_eva_port, @item.jpc_eva_port)
                            validated_remote_ip, error_msg_remote_ip = @item.validate_input(:jpc_remote_ip, @item.jpc_remote_ip)
                            if !validated_eva_port
                                alert_ontop_parent(app.win, error_msg_eva_port, title: app.instance_variable_get('@title'))
                                @jpc_eva_port_edit.focus
                            elsif !validated_remote_ip
                                alert_ontop_parent(app.win, error_msg_remote_ip, title: app.instance_variable_get('@title'))
                                @jpc_remote_ip_edit.focus
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
#   Shoes.app :title => MESSAGE["title"] , width: 960, height: 670 do
#     win.set_size_request(960, 670)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/jpc_remote_setting')
#   end
# end
