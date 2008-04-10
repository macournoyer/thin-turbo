require 'rubygems'
require File.dirname(__FILE__) + "/../../lib/thin-turbo"
require File.dirname(__FILE__) + '/../app'

b = Thin::Backend.new('0.0.0.0', 7000, App.new)
b.start