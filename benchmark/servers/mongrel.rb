require 'rubygems'
require 'rack'
require File.dirname(__FILE__) + '/../app'

Rack::Handler::Mongrel.run App.new, :Host => '0.0.0.0', :Port => 7000