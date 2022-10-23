#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Installation confirm
# Description   :: Render Installation confirm page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Installation confirm page
class Installation < Shoes
    include Utility
    url('/installation_confirmation', :installation_confirmation)

    def installation_confirmation
        # calculate the total number of features in features.xml
        feautre_num = []
        total_num = 0
        Item::DEPLOYMENT_COMPONENTS.each do |component|
            num = @item.get_deployment_type_count(component)
            feautre_num.push(num.to_i)
            total_num += num.to_i
        end

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
                    para MESSAGE['installation_confirmation_page'][@item.installation_mode]['heading'], \
                         @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end

                stack width: 1.0, height: 40 do
                    para ''
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
                            end_num = start_num + feautre_num[dep_index]

                            (start_num...end_num).each do |i|
                                component = Item::FEATURES[i]
                                if @item.feature_list.include?(component)
                                    flow width: 1.0 do
                                        # This is a horizontal spacer
                                        stack width: 15 do
                                            para ''
                                        end
                                        para MESSAGE['inputs'][component], @small_text_style.merge(width: 225, margin_bottom: 3, margin_left: 10, size: 10.5)
                                        # flow block
                                    end
                                end
                                # feature loop block
                            end
                            start_num = end_num

                            # stack block
                        end
                        # deployment type loop
                    end

                    # This is a horizontal spacer
                    stack width: 20, height: 40 do
                        para ''
                    end

                    Item::DEPLOYMENT_COMPONENTS.each do |_deployment_component|
                        # This is a horizontal spacer
                        stack width: 20, height: 1.0 do
                            para ''
                        end
                        stack width: 280, height: 30 do
                            background @while_square_color
                            para ''
                        end
                    end
                    # feature flow
                end

                # This is a vertical spacer
                flow width: 1.0, height: 20 do
                    para ''
                end

                # installer progress
                progress_flow = flow @installer_progress_size_style do
                    background @install_progress_back_color
                    stack width: 960, height: 1.0 do
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
                        back_btn = nl_button :back
                        back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 535, height: 1.0 do
                        para ' '
                    end

                    stack width: 240, height: 50 do
                        next_btn = nl_button :confirm_and_uninstall if @item.installation_mode.eql?('remove')
                        next_btn = nl_button :confirm_and_install if @item.installation_mode.eql?('install')
                        next_btn = nl_button :confirm_and_upgrade if @item.installation_mode.eql?('upgrade')

                        next_btn.click do
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
#     visit('/installation_confirmation')
#   end
# end
