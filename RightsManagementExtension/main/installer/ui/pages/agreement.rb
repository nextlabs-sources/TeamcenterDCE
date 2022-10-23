#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Agreement
# Description   :: Render agreement page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Installation Agreement Page
class Installation < Shoes
    include Utility
    url('/agreement', :agreement)

    def agreement
        # changes the link style and link hover style
        # this method changes the link style and link hover style for the whole application
        style(Link, underline: false, stroke: '#FFF', weight: 'bold')
        style(LinkHover, underline: false, stroke: '#FFF', weight: 'bold')

        # The header
        stack @installer_header_size_style do
            background @header_color
            banner
        end

        # The agreement area
        stack @installer_content_size_style do
            agreement_file = File.join(File.dirname(File.dirname(__FILE__)), 'agreement.txt')
            agreement_area = edit_box(width: 0.965, height: 1.0, margin_left: 20, margin_bottom: 5, state: 'disabled')
            agreement_area.text = File.exist?(agreement_file) ? File.read(agreement_file) : 'Error load agreeemnt'
        end

        # The footer area
        stack @installer_footer_size_style do
            background @footer_color
            # This is a vertical spacer
            stack width: 1.0, height: 15 do
                para ''
            end

            flow width: 1.0, height: 60 do
                stack width: 415, height: 1.0 do
                    para ''
                end

                stack width: 300, height: 50 do
                    cancel_btn = nl_button :cancel
                    cancel_btn.click do
                        puts 'User disagrees with agreement'
                        exit
                    end
                end

                stack width: 240, height: 50 do
                    agreement_next_btn = nl_button :agree
                    agreement_next_btn.click do
                        SKIP_PAGES['/creo_dir'] = true if @item.creo_dir == '' || !@item.is_cad_rmx_include?("Creo")
                        SKIP_PAGES['/solidworks_dir'] = true if @item.solidworks_dir == '' || !@item.is_cad_rmx_include?("SolidWorks")
                        SKIP_PAGES['/autocad_dir'] = true if @item.autocad_dir == '' || !@item.is_cad_rmx_include?("AutoCAD")
                        SKIP_PAGES['/catia_dir'] = true if @item.catia_dir == '' || !@item.is_cad_rmx_include?("CATIA")
                        visit(wizard(:next))
                    end
                end
            end
        end
    end
end

# if __FILE__ == $0
#   Shoes.app :title => MESSAGE["title"] , :width => 960, :height => 650 do
#     win.set_size_request(960, 650)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/agreement')
#   end
# end
