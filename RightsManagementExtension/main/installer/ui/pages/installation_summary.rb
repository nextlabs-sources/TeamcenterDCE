#! /usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Installation Summary
# Description   :: Render Installation summary page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Installation summary page
class Installation < Shoes
    include Utility
    url('/installation_summary', :installation_summary)

    def installation_summary
        # calculate the total number of features in features.xml
        feature_num = []
        total_num = 0
        Item::DEPLOYMENT_COMPONENTS.each do |component|
            num = @item.get_deployment_type_count(component)
            feature_num.push(num.to_i)
            total_num += num.to_i
        end

        stack @installer_page_size_style do
            # The header area
            stack @installer_header_size_style do
                background @header_color
                banner
            end

            # The body
            stack @installer_content_size_style do
                background @content_color
                success_fail = ''
                success_fail = 'success' if @flag.installation_finished
                success_fail = 'fail' if @flag.installation_failed

                stack width: 1.0, height: 60 do
                    header_msg = MESSAGE['installation_summary_page'][@item.installation_mode][success_fail]['heading']
                    # headerMsg = MESSAGE["installation_summary_page"]["rollback"]["heading"] if ENV['rollback'] == "true"
                    para header_msg, @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 20, margin_bottom: 20, margin_right: 20)
                end

                stack width: 0.7, height: 40 do
                    help_note_msg = MESSAGE['installation_summary_page'][@item.installation_mode][success_fail]['help_note']
                    # help_note_msg = MESSAGE["installation_summary_page"]["rollback"]["help_note"] if ENV['rollback'] == "true"
                    para help_note_msg, @small_text_style.merge(align: 'center', stroke: '#000000', margin_left: 150)
                    para_warn = para MESSAGE['installation_summary_page']['delete_warn'], @small_text_style.merge(align: 'center', stroke: '#000000', margin_left: 160)
                    para_warn.hide if ENV['has_warning_msg'] != 'true' || @item.installation_mode == 'install'
                end

                # This is a horizontal spacer
                stack width: 1.0, height: 20 do
                    para ''
                end

                # feature and component stack
                main_flow = flow width: 920, height: 340 do
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

                        stack width: 280, height: 190 do
                            background @while_square_color

                            dep_index = Item::DEPLOYMENT_COMPONENTS.index(deployment_component)
                            end_num = start_num + feature_num[dep_index]

                            (start_num...end_num).each do |i|
                                component = Item::FEATURES[i]
                                if @item.feature_list.include?(component)
                                    flow width: 1.0 do
                                        # This is a horizontal spacer
                                        stack width: 15 do
                                            para ''
                                        end
                                        image(File.join(File.dirname(File.dirname(__FILE__)), 'images/tick.png'), margin_top: 4)
                                        para MESSAGE['inputs'][component], @small_text_style.merge(width: 225, margin_bottom: 3, margin_left: 10, size: 10.5)
                                        # flow block
                                    end
                                end
                            end
                            start_num = end_num
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

                main_flow.hide if success_fail == 'fail' || ENV['rollback'] == 'true'
            end

            # The footer area
            stack @installer_footer_size_style do
                background @footer_color

                # This is a vertical spacer
                stack width: 1.0, height: 15 do
                    para ' '
                end

                flow width: 1.0, height: 60 do
                    # This is a horizontal spacer
                    stack width: 30, height: 1.0 do
                        para ' '
                    end

                    rollback_stack = stack width: 180, height: 50 do
                        rb_btn = nl_button :rollback
                        rb_btn.click do
                            if File.exist?(File.join(@item.temp_dir, 'has_installed.txt')) && @item.installation_mode.eql?('install')
                                # puts "reg_solution == " + reg_solution
                                error_msg = 'Rollback is not allowed in covering installtion. RMX does not support version maintenance yet.'
                                alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                            elsif ENV['rollback'] != 'true'
                                @flag.install_cmd_exited = false
                                @flag.installation_failed = false
                                if @item.installation_mode.eql?('install')
                                    @item.installation_mode = 'remove'
                                elsif @item.installation_mode.eql?('remove')
                                    @item.installation_mode = 'install'
                                end
                                ENV['rollback'] = 'true'
                                visit(wizard(:back))
                            else
                                error_msg = 'You cannot rollback based on rollback process.'
                                alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                            end
                        end
                    end
                    rollback_stack.hide if @flag.installation_finished || @item.installation_mode == 'upgrade'

                    # This is a horizontal spacer
                    stack width: 180, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        if @flag.installation_finished
                            finish_btn = nl_button :finish
                            finish_btn.click do
                                ENV['rollback'] = 'false'
                                exit
                            end
                        elsif @flag.installation_failed
                            retry_button = nl_button :retry
                            retry_button.click do
                                ENV['rollback'] = 'false'
                                @flag.install_cmd_exited = false
                                @flag.installation_failed = false
                                visit(wizard(:back))
                            end
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 180, height: 1.0 do
                        para ' '
                    end

                    failure_finish_stack = stack width: 180, height: 50 do
                        @finish_btn3 = nl_button :finish
                        @finish_btn3.click do
                            # Finish GUI on installation failure
                            exit
                        end
                    end
                    failure_finish_stack.hide if @flag.installation_finished
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
#     @item.installation_mode = "upgrade"
#     @flag.installation_finished = true
#     visit('/finish')
#   end
# end
