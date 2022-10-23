#!/usr/bin/env ruby

# Teamcenter RMX 5.0 GUI Installtion
# Pages         :: Welcome
# Description   :: Render welcome page and register into Shoes.url()
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require_relative '../bootstrap'
require_relative '../utility'

# GUI Installation Welcome Page
class Installation < Shoes
    include Utility
    url('/', :welcome)
    def welcome
        stack @installer_page_size_style do
            background(File.join(File.dirname(File.dirname(__FILE__)), 'images/Green BG.png'))
            # First is the company name

            stack width: 1.0, height: 60 do
                para ''
            end

            stack width: 0.9, height: 500, margin_left: 120 do
                stack width: 100, height: 22 do
                    image(File.join(File.dirname(File.dirname(__FILE__)), 'images/Logo-NextLabs-White.png'))
                end

                # Then welcome paragrah
                flow width: 1.0, height: 65 do
                    stack width: 0.06, height: 1.0 do
                        image(File.join(File.dirname(File.dirname(__FILE__)), 'images/Line.png'), margin_top: 55)
                    end
                    stack width: 0.7, height: 1.0 do
                        para MESSAGE['welcome_page']['welcome_para'], @welcome_page_text.merge(margin_left: 0, margin_top: 45, margin_bottom: 0, size: 12, weight: 'bold')
                    end
                end

                # Then product name
                stack width: 1.0, height: 90 do
                    para MESSAGE['welcome_page']['product_name'], @welcome_page_text.merge(margin_left: 0, margin_top: 25, margin_bottom: 0, size: 20, weight: 'bold')
                end

                stack width: 1.0, height: 50 do
                    para MESSAGE['welcome_page']['help_note'], @welcome_page_text.merge(margin_top: 20)
                end

                stack width: 1.0, height: 30 do
                    para MESSAGE['welcome_page']['server'], @welcome_page_text.merge(margin_left: 30)
                end

                stack width: 1.0, height: 30 do
                    para MESSAGE['welcome_page']['dispatcher'], @welcome_page_text.merge(margin_left: 30)
                end

                stack width: 1.0, height: 30 do
                    para MESSAGE['welcome_page']['tcsoa'], @welcome_page_text.merge(margin_left: 30)
                end

                stack width: 1.0, height: 30 do
                    para MESSAGE['welcome_page']['jpc'], @welcome_page_text.merge(margin_left: 30)
                end

                stack width: 1.0, height: 30 do
                    para MESSAGE['welcome_page']['skydrm'], @welcome_page_text.merge(margin_left: 30)
                end

                stack width: 1.0, height: 20 do
                    para ''
                end

                flow width: 1.0, height: 60 do
                    # Welcome page will only show 1 button for any scenario
                    if File.exist?(File.join(@item.temp_dir, 'has_installed.txt')) && @item.compare_tcrmx_version == 1
                        # If RMX is installed and it is a upgrade scenario then show Upgrade button only
                        stack width: 190, height: 50 do
                            welcome_upgrade_btn = nl_button :upgrade
                            welcome_upgrade_btn.click do
                                @item.installation_mode = 'upgrade'
                                @item.check_template_upgrade
                                validated, error_msg = @item.validate_environment(:java)
                                if !validated
                                    alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                                else
                                    visit(wizard(:next))
                                end
                            end
                        end
                    elsif File.exist?(File.join(@item.temp_dir, 'has_installed.txt')) && @item.compare_tcrmx_version.zero?
                        # If RMX is installed and not in a upgrade scenario then show Uninstall button only
                        stack width: 190, height: 50 do
                            welcome_uninstall_btn = nl_button :uninstall
                            welcome_uninstall_btn.click do
                                @item.installation_mode = 'remove'
                                validated, error_msg = @item.validate_environment(:java)
                                if !validated
                                    alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                                else
                                    visit(wizard(:next))
                                end
                            end
                        end
                    else
                        # When RMX is not installed at all or same version installed, show Install button only
                        stack width: 255, height: 50 do
                            welcome_install_btn = nl_button :proceed_to_install
                            welcome_install_btn.click do
                                @item.installation_mode = 'install'
                                validated, error_msg = @item.validate_environment(:java)
                                if !validated
                                    alert_ontop_parent(app.win, error_msg, title: app.instance_variable_get('@title'))
                                else
                                    visit(wizard(:next))
                                end
                            end
                        end
                    end
                end
            end

            # Then the copyright info
            stack @installer_footer_size_style do
                para MESSAGE['welcome_page']['copyright_message'], @copyright_label_style
            end
        end
    end
end

# if __FILE__ == $0
#   Shoes.app :title => MESSAGE['title'] , width: 960, height: 680 do
#     win.set_size_request(960, 680)
#     win.set_resizable(false)
#     win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
#     visit('/')
#   end
# end
