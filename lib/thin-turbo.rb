require 'rubygems'
require 'fastthread'
require 'thin'

require File.dirname(__FILE__) + "/../ext/thin_backend/thin_backend"
require File.dirname(__FILE__) + "/thin/backends/turbo"
require File.dirname(__FILE__) + "/rack/adapter/threaded_rails"
