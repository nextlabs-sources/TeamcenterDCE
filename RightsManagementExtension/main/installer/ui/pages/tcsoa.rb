#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: TcSOA Setting
# Description   :: Render Teamcenter SOA page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI TcSOA page
class Installation < Shoes
    include Utility
    url('/tcsoa', :tcsoa)

    def tcsoa
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
                    para MESSAGE['tcsoa_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end
                stack width: 0.55, height: 40 do
                    para MESSAGE['tcsoa_page']['help_note'], @small_text_style.merge(align: 'center', stroke: '#000000', margin_left: 220)
                end

                stack width: 1.0, height: 40 do
                    para ''
                end

                flow width: 1.0, height: 100, margin_left: 220 do
                    para MESSAGE['tcsoa_page']['tcsoa_hostname'], @small_text_style
                    stack width: 530, height: 60, margin_top: 10 do
                        @tcsoa_hostname_edit = nl_text_edit(:short, @item.tcsoa_hostname, width: 530, height: 40)
                    end
                end

                flow width: 1.0, height: 150 do
                    stack width: 470, height: 1.0, margin_left: 220 do
                        para MESSAGE['inputs']['tc_user'], @small_text_style
                        stack width: 250, height: 60, margin_top: 10 do
                            @tc_user_edit = nl_text_edit(:short, @item.tc_user, width: 250, height: 40)
                        end
                    end

                    stack width: 280, height: 1.0, margin_left: 30 do
                        para MESSAGE['inputs']['tc_user_psd'], @small_text_style
                        stack width: 250, height: 60, margin_top: 10 do
                            @tc_user_psd_edit = nl_text_edit(:short, ENV['tc_user_psd'], width: 250, height: 40, secret: true)
                        end
                    end
                end
                # This is a vertical spacer
                flow width: 1.0, height: 65, margin_left: 220 do
                    para ''
                end

                # installer progress
                flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 864, height: 1.0 do
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
                        tcsoa_back_btn = nl_button :back
                        tcsoa_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        tcsoa_next_btn = nl_button :next
                        tcsoa_next_btn.click do
                            @item.tcsoa_hostname = @tcsoa_hostname_edit.text
                            @item.tc_user = @tc_user_edit.text
                            ENV['tc_user_psd'] = @tc_user_psd_edit.text
                            visit(wizard(:next))
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
#     visit('/tcsoa')
#   end
# end
