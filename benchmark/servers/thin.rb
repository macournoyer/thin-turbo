require 'rubygems'
require 'thin'
require File.dirname(__FILE__) + '/../app'

Thin::Logging.silent = true
Thin::Server.start '0.0.0.0', 7000 do
  run App.new
end
