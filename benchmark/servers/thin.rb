require 'rubygems'
require 'thin'
require File.dirname(__FILE__) + '/../app'

Thin::Logging.silent = true
Rack::Handler::Thin.run App.new, :Host => '0.0.0.0', :Port => 7000