#! /usr/bin/env ruby

# Teamcenter RMX 5.2 GUI Installtion
# Pages         :: Control Center Information
# Description   :: Render Control Center information page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'
require 'green_shoes'

# GUI Control Center Information page
class Installation < Shoes
    include Utility
    url('/cc_info', :cc_info)

    def cc_info
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
                    para MESSAGE['cc_info_page']['heading'], @large_text_style.merge(stroke: '#000000', align: 'center', margin_top: 10, margin_bottom: 20, margin_right: 20)
                end

                # main input flow
                flow width: 1.0, height: 370 do
                    # This is flow left margin
                    stack width: 0.1, height: 100 do
                        para ''
                    end

                    # Main content of the input flow
                    stack width: 0.8, height: 370 do
                        para MESSAGE['cc_info_page']['help_note'], @small_text_style.merge(stroke: '#000000', align: 'center', margin_right: 20)

                        stack width: 1.0, height: 10 do
                            para ''
                        end

                        flow width: 600, height: 300, margin_left: 80 do
                            para MESSAGE['inputs']['cc_host'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @cc_host_edit = nl_text_edit(:large, @item.cc_host, width: 400, height: 40)
                            end

                            # This is a vertical spacer
                            stack width: 1.0, height: 10 do
                                para ''
                            end
                            
                            para MESSAGE['inputs']['cc_apiuser'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @cc_apiuser_edit = nl_text_edit(:large, @item.cc_apiuser, width: 400, height: 40)
                            end

                            # This is a vertical spacer
                            stack width: 1.0, height: 10 do
                                para ''
                            end

                            para MESSAGE['inputs']['cc_apisecret'], @small_text_style
                            stack width: 600, height: 50, margin_top: 10 do
                                @cc_apisecret_edit = nl_text_edit(:large, @item.cc_apisecret, width: 400, height: 40, secret: true)
                            end

                            # This is a vertical spacer
                            stack width: 1.0, height: 10 do
                                para ''
                            end

                            stack width: 600, height: 50 do
                                flow do
                                    stack width: 400 do
                                        para MESSAGE['jpc_remote_setting_page']['remote_ip'], @small_text_style
                                        @jpc_remote_ip_edit = nl_text_edit(:large, @item.jpc_remote_ip, width: 400, height: 40, margin_top: 10)
                                    end

                                    stack width: 200, margin_top: 30 do
                                        @cc_advanced = button 'Advanced...', width: 80, height: 40, margin_left: 40
                                        @cc_advanced.click do
                                            cc_advanced_parameters
                                        end
                                    end
                                end
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
                        cc_info_back_btn = nl_button :back
                        cc_info_back_btn.click do
                            visit(wizard(:back))
                        end
                    end

                    # This is a horizontal spacer
                    stack width: 595, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 50 do
                        cc_info_next_btn = nl_button :next
                        cc_info_next_btn.click do
                            @item.cc_host = @cc_host_edit.text.strip
                            @item.cc_apiuser = @cc_apiuser_edit.text.strip
                            @item.cc_apisecret = @cc_apisecret_edit.text.strip
                            @item.jpc_remote_ip = @jpc_remote_ip_edit.text.strip

                            validated_host, error_msg_host = @item.validate_input(:cc_host, @item.cc_host)
                            validated_port, error_msg_port = @item.validate_input(:cc_port, @item.cc_port)
                            validated_user, error_msg_user = @item.validate_input(:cc_apiuser, @item.cc_apiuser)
                            validated_secret, error_msg_secret = @item.validate_input(:cc_apisecret, @item.cc_apisecret)

                            validated_remote_ip, error_msg_remote_ip = @item.validate_input(:jpc_remote_ip, @item.jpc_remote_ip)
                            validated_eva_port, error_msg_eva_port = @item.validate_input(:jpc_eva_port, @item.jpc_eva_port)

                            # Massive validation. Make sure all core input is not empty
                            if !validated_host
                                alert_ontop_parent(app.win, error_msg_host, title: app.instance_variable_get('@title'))
                                @cc_host_edit.focus
                            elsif !validated_port
                                alert_ontop_parent(app.win, error_msg_port, title: app.instance_variable_get('@title'))
                            elsif !validated_user
                                alert_ontop_parent(app.win, error_msg_user, title: app.instance_variable_get('@title'))
                                @cc_apiuser_edit.focus
                            elsif !validated_secret
                                alert_ontop_parent(app.win, error_msg_secret, title: app.instance_variable_get('@title'))
                                @cc_apisecret_edit.focus
                            elsif !validated_remote_ip
                                alert_ontop_parent(app.win, error_msg_remote_ip, title: app.instance_variable_get('@title'))
                                @jpc_remote_ip_edit.focus
                            elsif !validated_eva_port
                                alert_ontop_parent(app.win, error_msg_eva_port, title: app.instance_variable_get('@title'))
                            else
                                # Validate connection to CC, API account auth and connection to JPC
                                validated_cc_connection, error_cc_connection = @item.validate_input(:cc_connection, @item.cc_host, @item.cc_port)
                                validated_cc_api, error_cc_api = @item.validate_input(:cc_api, @item.cc_host, @item.cc_port, @item.cc_apiuser, @item.cc_apisecret)
                                validated_jpc_connection, error_jpc_connection = @item.validate_input(:jpc_connection, @item.jpc_remote_ip, @item.jpc_eva_port, @item.jpc_https)

                                prompt_msg = ""
                                if !validated_cc_connection
                                    prompt_msg = " #{error_cc_connection}\n You can choose OK to manually fix later\n or choose Cancel to fix this inputs now.\n\n"
                                elsif !validated_cc_api
                                    prompt_msg = " #{error_cc_api}\n You can choose OK to manually fix later\n or choose Cancel to fix this inputs now.\n\n"
                                elsif !validated_jpc_connection
                                    prompt_msg = " #{error_jpc_connection}\n You can choose OK to manually fix later\n or choose Cancel to fix this inputs now.\n\n"
                                end

                                # Either no test fail, or one test fail and will fix MANUALLY later
                                if prompt_msg == "" || (prompt_msg != "" && confirm_ontop_parent(app.win, prompt_msg, title: app.instance_variable_get('@title')))
                                    ENV['cc_apisecret'] = @item.cc_apisecret
                                    visit(wizard(:next))
                                end
                            end
                        end
                    end
                end
            end
        end
    end

    def cc_advanced_parameters
        Shoes.app title: MESSAGE["title"], width: 450, height: 450 do
            @config = Item.instance

            @small_text_style = { font: 'Segoe UI', stroke: '#000000', size: 10 }
            @middle_text_style = { font: 'Segoe UI', stroke: '#000000', size: 14 }
            @large_text_style = { font: 'Segoe UI', stroke: '#000000', size: 20 }

            para "Control Center Advanced Parameters", @middle_text_style.merge(align: 'center', margin_top: 20)

            # This is a horizontal spacer
            stack width: 1.0, height: 15 do
                para ' '
            end

            stack height: 150 do
                flow height: 50 do
                    stack width: 0.55 do
                        para "Control Center Port", @small_text_style.merge(align: 'center')
                    end
                    stack width: 0.35 do
                        @cc_port_edit = edit_line(@config.cc_port, { width: 100 }.merge({}))
                    end
                end

                flow height: 50 do
                    stack width: 0.55 do
                        para "Using OAuth HTTPS", @small_text_style.merge(align: 'center')
                    end
                    @cc_oauth = stack width: 0.35 do
                        @cc_oauth2_https_check = check
                        @cc_oauth2_https_check.checked = @config.cc_oauth2_https
                    end
                end

                flow height: 50 do
                    stack width: 0.55 do
                        para "Ignore CC HTTPS Certificate", @small_text_style.merge(align: 'center')
                    end
                    stack width: 0.35 do
                        @cc_ignore_https_cert_check = check
                        @cc_ignore_https_cert_check.checked = @config.cc_ignore_https_cert
                    end
                end
            end

            para "Policy Controller Advanced Parameters", @middle_text_style.merge(align: 'center', margin_top: 20)

            # This is a horizontal spacer
            stack width: 1.0, height: 15 do
                para ' '
            end

            stack height: 100 do
                flow height: 50 do
                    stack width: 0.55 do
                        para "Policy Controller Port", @small_text_style.merge(align: 'center')
                    end
                    stack width: 0.35 do
                        @jpc_eva_port_edit = edit_line(@config.jpc_eva_port, {width: 100})
                    end
                end

                flow height: 50 do
                    stack width: 0.55 do
                        para "HTTPS Policy Controller", @small_text_style.merge(align: 'center')
                    end
                    stack width: 0.35 do
                        @jpc_https_check = check
                        @jpc_https_check.checked = @config.jpc_https
                    end
                end
            end

            stack width: 1.0, height: 15 do
                para ' '
            end

            flow height: 40 do
                stack width: 0.5 do
                    @update_param = button 'OK', width: 80, height: 30, margin_left: 60
                    @update_param.click do
                        @config.cc_port = @cc_port_edit.text.strip
                        @config.cc_oauth2_https = @cc_oauth2_https_check.checked?
                        @config.cc_ignore_https_cert = @cc_ignore_https_cert_check.checked?
                        @config.jpc_eva_port = @jpc_eva_port_edit.text.strip
                        @config.jpc_https = @jpc_https_check.checked?

                        if @config.cc_port == ""
                            alert_ontop_parent(app.win, "Control Center port should not be empty", title: app.instance_variable_get('@title'))
                        elsif @config.jpc_eva_port == ""
                            alert_ontop_parent(app.win, "Policy Controller port should not be empty", title: app.instance_variable_get('@title'))
                        else
                            win.hide_all
                        end
                        # win.hide_all
                    end
                end
                

                stack width: 0.5 do
                    @cancel_param = button "Cancel", width: 80, height: 30, margin_left: 60
                    @cancel_param.click do
                        # TODO: Temporary solution is to hide this pop-up. Ideally, it should be CLOSED. Will work on next sprint
                        win.hide_all
                    end
                end
            end
        end
    end
end

if __FILE__ == $0
 Shoes.app :title => MESSAGE["title"] , width: 960, height: 670 do
   win.set_size_request(960, 650)
   win.set_resizable(false)
   win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
   visit('/cc_info')
 end
end
