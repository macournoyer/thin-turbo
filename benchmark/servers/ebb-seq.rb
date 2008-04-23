require 'rubygems'
require 'ebb'
require File.dirname(__FILE__) + '/../app'

Ebb.start_server(App.new, :port => 7000, :threaded_processing => false)