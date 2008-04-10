require 'rubygems'
require 'ebb'
require File.dirname(__FILE__) + '/../app'

Ebb.start_server(App.new, :threaded_processing => false, :port => 7000)