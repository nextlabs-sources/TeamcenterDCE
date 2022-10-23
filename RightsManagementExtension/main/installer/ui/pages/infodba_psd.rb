#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Infodba Password
# Description   :: Render Infodba password page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Infodba Password page
class Installation < Shoes
    include Utility
    url('/infodba_psd', :infodba_psd)

    def infodba_psd
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
                    para MESSAGE['infodba_psd_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end
                stack width: 0.45, height: 40 do
                    para MESSAGE['infodba_psd_page']['help_note'], @small_text_style.merge(align: 'center', stroke: '#000000', margin_left: 240)
                end

                stack width: 1.0, height: 40 do
                    para ''
                end

                flow width: 1.0, height: 315, margin_left: 220 do
                    para MESSAGE['inputs']['infodba_psd'], @small_text_style

                    stack width: 500, height: 60, margin_top: 10 do
                        @infodba_psd_edit = nl_text_edit(:short, ENV['infodba_psd'], width: 500, height: 40, secret: true)
                    end

                    stack width: 0.7, height: 60 do
                        para MESSAGE['infodba_psd_page']['help_note_2'], @small_text_style.merge(size: 8)
                    end
                end

                # installer progress
                progress_flow = flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 480, height: 1.0 do
                        background @install_progress_front_color
                    end
                end
                progress_flow.hide if @item.installation_mode.eql?('remove')
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
                        infodba_psd_back_btn = nl_button :back
                        infodba_psd_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        infodba_psd_next_btn = nl_button :next
                        infodba_psd_next_btn.click do
                            @item.infodba_psd = @infodba_psd_edit.text
                            validated, error_msg = @item.validate_input(:infodba_psd, @item.tc_root, @item.tc_data, @item.infodba_psd)
                            if !validated
                                alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                                @infodba_psd_edit.focus
                            else
                                ENV['infodba_psd'] = @infodba_psd_edit.text
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
#     visit('/infodba_psd')
#   end
# end
