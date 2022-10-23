#! /usr/bin/env ruby

# Teamcenter RMX 5.1 GUI Installation
# Pages         :: App
# Description   :: Entry Point for Shoes GUI Application
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'green_shoes'
require 'gtk2'

require_relative './bootstrap'
require_relative './utility'
require_relative './wizard'

Dir[File.join(__dir__, 'pages', '*.rb')].each(&method(:require))

Shoes.app(title: MESSAGE['title'], width: 960, height: 650) do
    win.set_size_request(960, 650)
    win.set_resizable(false)
    win.set_window_position(Gtk::Window::POS_CENTER_ALWAYS)
    visit('/')
end
