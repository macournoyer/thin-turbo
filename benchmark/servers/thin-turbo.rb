require 'rubygems'
require File.dirname(__FILE__) + "/../../lib/thin-turbo"
require File.dirname(__FILE__) + '/../app'

Thin::Logging.silent = true
Thin::Server.start '0.0.0.0', 7000, :backend => Thin::Backends::Turbo do
  run App.new
end
