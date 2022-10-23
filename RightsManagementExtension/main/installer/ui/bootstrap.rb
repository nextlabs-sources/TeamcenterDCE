#!/usr/bin/env ruby

# Teamcenter RMX 5.1 GUI Installtion
# Component     :: Bootstrap
# Description   :: GUI component used by every pages
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'green_shoes'
require 'gtk2'
require_relative './utility'

# Added alert_ontop_parent method to Object
# to allow alert window be the center of the main window
class Object
    include Types
    include Utility
    def alert_ontop_parent(parent_win, msg, options = { block: true })
        $dde = true
        dialog = Gtk::MessageDialog.new(
            parent_win,
            Gtk::Dialog::MODAL,
            Gtk::MessageDialog::INFO,
            Gtk::MessageDialog::BUTTONS_OK,
            msg.to_s
        )
        dialog.title = options.key?(:title) ? options[:title] : 'Green Shoes says:'
        if options[:block]
            dialog.run
            dialog.destroy
        else
            dialog.signal_connect('response') { dialog.destroy }
            dialog.show
        end
    end

    def confirm_ontop_parent(parent_win, msg, options = {})
        # width = options.has_key?(:width) ? options[:width] : 500
        # height = options.has_key?(:height) ? options[:width] : 100
        $dde = true
        dialog = Gtk::Dialog.new(
            'Green Shoes asks:',
            parent_win,
            Gtk::Dialog::MODAL | Gtk::Dialog::DESTROY_WITH_PARENT,
            [Gtk::Stock::OK, Gtk::Dialog::RESPONSE_ACCEPT],
            [Gtk::Stock::CANCEL, Gtk::Dialog::RESPONSE_REJECT]
        )
        dialog.title = options.key?(:title) ? options[:title] : 'Green Shoes says:'
        dialog.vbox.add Gtk::Label.new msg
        # dialog.set_size_request width, height
        dialog.show_all
        ret = dialog.run == Gtk::Dialog::RESPONSE_ACCEPT
        dialog.destroy
        ret
    end

    def welcome_confirm_ontop_parent(parent_win, msg, options = {})
        # width = options.has_key?(:width) ? options[:width] : 300
        # height = options.has_key?(:height) ? options[:width] : 100
        $dde = true
        dialog = Gtk::Dialog.new(
            'Green Shoes asks:',
            parent_win,
            Gtk::Dialog::MODAL | Gtk::Dialog::DESTROY_WITH_PARENT,
            ['Acknowledge', Gtk::Dialog::RESPONSE_ACCEPT]
        )
        dialog.title = options.key?(:title) ? options[:title] : 'Green Shoes says:'
        dialog.vbox.add Gtk::Label.new msg
        # dialog.set_size_request width, height
        dialog.show_all
        ret = dialog.run == Gtk::Dialog::RESPONSE_ACCEPT
        dialog.destroy
        ret
    end
end

# Open Shoes::App class to redefine Progress Bar definition (Manage the size of the Progress Bar. Default Progress Bar is too short for the UI)
# Refer to https://www.rubydoc.info/gems/green_shoes/1.1.366/Shoes/App#progress-instance_method and find original 'def progress' source code
class Shoes
    class App
        def progress(args = {})
            args = basic_attributes args
            args[:width] = 150 if args[:width] < 150
            pb = Gtk::ProgressBar.new
            pb.set_size_request(args[:width], 20)
            @canvas.put pb, args[:left], args[:top]
            pb.show_now
            args[:real], args[:app], args[:noorder], args[:nocontrol] = pb, self, true, true
            Progress.new args
        end
    end
end

# Replace green shoes ICON with our's
# refer to http://goo.gl/jRVJo0
File.open(File.join(File.dirname(__FILE__), 'images/RMS-128-svg.png'), 'rb') do |new_icon_file|
    content = new_icon_file.read
    Pathname.new(Shoes::DIR).join('../static/gshoes-icon.png').open('wb') do |shoes_icon_file|
        shoes_icon_file.write content
    end
end

# Installation GUI
class Installation < Shoes
    include Utility
    def initialize
        # Application styles goes here (apply to all GUI pages)
        # This will be create when Shoe app run
        @installer_page_size_style = { width: 960, height: 650 }
        @installer_header_size_style = { width: 960, height: 95 }
        @installer_content_size_style = { width: 960, height: 460 }
        @installer_footer_size_style = { width: 960, height: 95 }
        @installer_progress_size_style = { width: 960, height: 5 }

        @welcome_background_color = [52 / 255.0, 153 / 255.0, 76 / 255.0, 1.0]
        @header_color = [255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 1.0]
        @content_color = [245 / 255.0, 245 / 255.0, 245 / 255.0, 1.0]
        @while_square_color = [255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 1.0]
        @footer_color = [52 / 255.0, 153 / 255.0, 76 / 255.0, 1.0]
        @progress_msg_color = [0 / 255.0, 0 / 255.0, 0 / 255.0, 1.0]
        @progress_error_msg_color = [255.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0, 1.0]
        @progress_success_msg_color = [0.0 / 255.0, 255.0 / 255.0, 0.0 / 255.0, 1.0]

        @link_color = [68 / 255.0, 138 / 255.0, 255.0, 1.0]
        @install_progress_back_color = [0 / 255.0, 100 / 255.0, 0, 1.0]
        @install_progress_front_color = [255.0, 255.0, 255.0, 1.0]

        @small_text_style = { font: 'Segoe UI', stroke: '#000000', size: 10 }
        @middle_text_style = { font: 'Segoe UI', stroke: '#000000', size: 14 }
        @large_text_style = { font: 'Segoe UI', stroke: '#000000', size: 20 }
        @banner_style = { font: 'Segoe UI', stroke: '#008000', margin_left: 10, size: 14 }
        @welcome_page_text = { font: 'Segoe UI', stroke: '#FFFFFF', size: 12 }

        @text_label_style_hover = { font: 'Segoe UI', stroke: '#FFFFFF', size: 8, align: 'center' }
        @copyright_label_style = { font: 'Segoe UI', stroke: '#FFFFFF', align: 'center', size: 10, margin_top: 70 }

        @item = Item.instance
        @flag = InstallationFlag.instance
    end

    # Here we define some common helper methods for UI dev
    def banner
        flow width: 1.0, height: 1.0 do
            # this is a vertical spacer
            stack width: 1.0, height: 10 do
                para ' '
            end
            stack width: 0.4, height: 1.0 do
                flow width: 1.0, height: 60, margin_top: 10 do
                    # this is a horizontal spacer
                    stack width: 20, height: 1.0 do
                        para ' '
                    end

                    stack width: 100, height: 30 do
                        image(File.join(File.dirname(__FILE__), 'images/logonextlabs_black.jpg'), width: 150, height: 25)
                    end
                end
            end
            stack width: 0.6, height: 1.0 do
                flow width: 1.0, height: 1.0 do
                    stack width: 0.6, height: 1.0, margin_top: 20 do
                        para MESSAGE['common']['product_name'], @banner_style.merge(align: 'right')
                    end
                    stack width: 0.1, height: 1.0, margin_top: 20 do
                        para MESSAGE['common']['for'], @banner_style.merge(stroke: '#C0C0C0', margin_left: 20)
                    end
                    stack width: 0.3, height: 1.0 do
                        image(File.join(File.dirname(__FILE__), 'images/teamcenter logo.png'), width: 140, height: 45)
                    end
                end
            end
        end
    end

    ##
    # This method creates a text edit
    # *type* :: <tt>:short</tt> or <tt>:long</tt>
    # *text* :: the text pre-filled for the text edit
    # Default long text edit width is 300
    # Default short text edit width is 100
    #
    def nl_text_edit(type, text = '', args = {})
        width =
            case type
            when :short
                100
            when :normal
                250
            when :long
                300
            else
                200
            end
        edit_line(text, { width: width }.merge(args))
    end

    ##
    # This method creates several kinds of buttons (back, next, back_text, next_text, finish, cancel, skip)
    # when passing click callbacks, for Link based buttons should pass the callback proc as parameter :click
    # *type* :: <tt>:back</tt>, <tt>:next</tt>, <tt>:back_text</tt>, <tt>:next_text</tt>, <tt>finish</tt>, <tt>cancel</tt>, <tt>skip</tt>
    def nl_button(type, args = {})
        btn = nil
        case type
        when :back
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 10 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 22, height: 1.0 do
                        para ' '
                    end

                    stack width: 130, height: 20 do
                        btn = image(File.join(__dir__, 'images/back.png'))
                    end
                end
            end
            return btn
        when :next
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 130, height: 38 do
                        btn = image(File.join(__dir__, 'images/next.png'))
                    end
                end
            end
            return btn
        when :agree
            stack width: 240, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 200, height: 38 do
                        btn = image(File.join(__dir__, 'images/agree_proceed.png'))
                    end
                end
            end
            return btn
        when :confirm_and_install
            stack width: 240, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 200, height: 38 do
                        btn = image(File.join(__dir__, 'images/confirm_and_install.png'))
                    end
                end
            end
            return btn
        when :confirm_and_uninstall
            stack width: 240, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 200, height: 38 do
                        btn = image(File.join(__dir__, 'images/confirm_and_uninstall.png'))
                    end
                end
            end
            return btn
        when :confirm_and_upgrade
            stack width: 240, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 200, height: 38 do
                        btn = image(File.join(__dir__, 'images/confirm_and_upgrade.png'))
                    end
                end
            end
            return btn
        when :finish
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 2 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 2, height: 1.0 do
                        para ' '
                    end

                    stack width: 130, height: 46 do
                        btn = image(File.join(__dir__, 'images/finish.png'))
                    end
                end
            end
            return btn
        when :retry
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 2 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 2, height: 1.0 do
                        para ' '
                    end

                    stack width: 130, height: 46 do
                        btn = image(File.join(__dir__, 'images/retry.png'))
                    end
                end
            end
            return btn
        when :rollback
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 2 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 2, height: 1.0 do
                        para ' '
                    end

                    stack width: 130, height: 46 do
                        btn = image(File.join(__dir__, 'images/rollback.png'))
                    end
                end
            end
            return btn
        when :proceed_to_install
            stack width: 300, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 250, height: 38 do
                        btn = image(File.join(__dir__, 'images/Proceed to Install.png'))
                    end
                end
            end
            return btn
        when :uninstall
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 38 do
                        btn = image(File.join(__dir__, 'images/Uninstall RMX.png'))
                    end
                end
            end
            return btn
        when :upgrade
            stack width: 180, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 180, height: 38 do
                        btn = image(File.join(__dir__, 'images/Upgrade RMX.png'))
                    end
                end
            end
            return btn
        when :cancel
            stack width: 300, height: 50 do
                # a vertical spacer
                stack width: 1.0, height: 6 do
                    para ' '
                end

                flow width: 1.0, height: 1.0 do
                    # a horizontal spacer
                    stack width: 6, height: 1.0 do
                        para ' '
                    end

                    stack width: 250, height: 38 do
                        btn = image(File.join(__dir__, 'images/disagree_and_cancel.png'))
                    end
                end
            end
            return btn
        else
            puts 'Unknown Type of Button'
            raise NotImplementedError('Unknown Type of Button')
        end
    end
end
