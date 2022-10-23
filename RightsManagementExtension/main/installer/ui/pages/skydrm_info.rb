#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: SkyDRM Information
# Description   :: Render SkyDRM information page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI SkyDRM Information page
class Installation < Shoes
    include Utility
    url('/skydrm_info', :skydrm_info)

    def skydrm_info
        style(Link, underline: false, stroke: '#FFF', weight: 'bold')
        style(LinkHover, underline: false, stroke: '#FFF', weight: 'bold')

        stack @installer_page_size_style do
            # The header area
            stack @installer_header_size_style do
                background @header_color
                banner
            end

            # The body area
            stack @installer_content_size_style do
                background @content_color

                stack width: 1.0, height: 60 do
                    para MESSAGE['skydrm_info_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 10, margin_bottom: 20, margin_right: 20)
                end

                # main input flow
                flow width: 1.0, height: 370 do
                    # This is flow left margin
                    stack width: 0.1, height: 100 do
                        para ''
                    end

                    # Main content of the input flow
                    stack width: 0.8, height: 370 do
                        para MESSAGE['skydrm_info_page']['help_note'], @small_text_style.merge(stroke: '#000000', align: 'center', margin_right: 20)

                        stack width: 1.0, height: 10 do
                            para ''
                        end

                        flow width: 600, height: 300, margin_left: 80 do
                            para MESSAGE['inputs']['skydrm_router_url'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @skydrm_router_url_edit = nl_text_edit(:large, @item.skydrm_router_url, width: 600, height: 40)
                            end

                            # This is a vertical spacer
                            # stack width: 1.0, height: 10 do
                            #     para ''
                            # end
                            #
                            # para MESSAGE['inputs']['skydrm_tenant_name'], @small_text_style
                            # stack width: 600, height: 50, margin_top: 10 do
                            #     @skydrm_tenant_name_edit = nl_text_edit(:large, @item.skydrm_tenant_name, width: 600, height: 40)
                            # end

                            # This is a vertical spacer
                            stack width: 1.0, height: 10 do
                                para ''
                            end

                            para MESSAGE['inputs']['skydrm_app_id'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @skydrm_app_id_edit = nl_text_edit(:large, @item.skydrm_app_id, width: 600, height: 40)
                            end

                            # This is a vertical spacer
                            stack width: 1.0, height: 10 do
                                para ''
                            end

                            para MESSAGE['inputs']['skydrm_app_key'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @skydrm_app_key_edit = nl_text_edit(:large, @item.skydrm_app_key, width: 600, height: 40, secret: true)
                            end
                        end
                    end
                end

                # This is a vertical spacer
                stack width: 1.0, height: 25 do
                    para ''
                end

                # installer progress
                flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 420, height: 1.0 do
                        background @install_progress_front_color
                    end
                end
            end

            # The footer area
            stack @installer_footer_size_style do
                background @footer_color

                # This is a vertical spacer
                stack width: 1.0, height: 15 do
                    para ''
                end

                flow width: 1.0, height: 60 do
                    stack width: 180, height: 50 do
                        skydrm_info_back_btn = nl_button :back
                        skydrm_info_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        skydrm_info_next_btn = nl_button :next
                        skydrm_info_next_btn.click do
                            @item.skydrm_router_url = @skydrm_router_url_edit.text.strip
                            @item.skydrm_app_id = @skydrm_app_id_edit.text.strip
                            @item.skydrm_app_key = @skydrm_app_key_edit.text.strip

                            validated_url, error_msg_url = @item.validate_input(:skydrm_router_url, @item.skydrm_router_url)
                            validated_app_id, error_msg_app_id = @item.validate_input(:skydrm_app_id, @item.skydrm_app_id)
                            validated_app_key, error_msg_app_key = @item.validate_input(:skydrm_app_key, @item.skydrm_app_key)
                            
                            if !validated_url
                                alert_ontop_parent(app.win, error_msg_url, title: app.instance_variable_get('@title'))
                                @skydrm_router_url_edit.focus
                            elsif !validated_app_id
                                alert_ontop_parent(app.win, error_msg_app_id, title: app.instance_variable_get('@title'))
                                @skydrm_app_id_edit.focus
                            elsif !validated_app_key
                                alert_ontop_parent(app.win, error_msg_app_key, title: app.instance_variable_get('@title'))
                                @skydrm_app_key_edit.focus
                            else
                                ENV['skydrm_app_key'] = @item.skydrm_app_key
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
#  Shoes.app :title => MESSAGE["title"] , width: 960, height: 670 do
#    win.set_size_request(960, 650)
#    win.set_resizable(false)
#    win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#    visit('/skydrm_info')
#  end
# end
